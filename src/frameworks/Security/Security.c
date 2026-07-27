/**
 * Osxie Security Framework Implementation
 */

#include "Security.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

// Internal structures
typedef struct OpaqueSecKeychainRef {
    char *path;
    bool is_locked;
    CFMutableArrayRef items;
    pthread_mutex_t lock;
} *SecKeychainRef_t;

typedef struct OpaqueSecKeychainItemRef {
    SecKeychainRef keychain;
    SecItemClass itemClass;
    CFMutableDictionaryRef attributes;
    CFDataRef data;
} *SecKeychainItemRef_t;

typedef struct AuthorizationOpaqueRef {
    uint32_t flags;
    AuthorizationRights *rights;
    uid_t uid;
    pid_t pid;
} *AuthorizationRef_t;

// Global default keychain
static SecKeychainRef g_default_keychain = NULL;
static CFMutableArrayRef g_keychain_list = NULL;
static pthread_mutex_t g_keychain_lock = PTHREAD_MUTEX_INITIALIZER;

// Helper functions
static char* get_default_keychain_path() {
    static char path[1024];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(path, sizeof(path), "%s/Library/Keychains/login.keychain", home);
    return path;
}

static void ensure_keychain_dir() {
    char dir[1024];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    
    snprintf(dir, sizeof(dir), "%s/Library/Keychains", home);
    mkdir(dir, 0700);
}

// Keychain functions
OSStatus SecKeychainOpen(const char *pathName, SecKeychainRef *keychain) {
    if (!pathName || !keychain) {
        return errSecParam;
    }
    
    SecKeychainRef_t kc = calloc(1, sizeof(struct OpaqueSecKeychainRef));
    if (!kc) {
        return errSecAllocate;
    }
    
    kc->path = strdup(pathName);
    kc->is_locked = false;
    kc->items = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    pthread_mutex_init(&kc->lock, NULL);
    
    *keychain = kc;
    
    printf("[OSXIE Security] Opened keychain: %s\n", pathName);
    
    return errSecSuccess;
}

OSStatus SecKeychainCreate(const char *pathName, uint32_t passwordLength, const void *password,
                           Boolean promptUser, SecAccessRef initialAccess, SecKeychainRef *keychain) {
    if (!pathName) {
        return errSecParam;
    }
    
    ensure_keychain_dir();
    
    // Check if already exists
    struct stat st;
    if (stat(pathName, &st) == 0) {
        return errSecDuplicateKeychain;
    }
    
    // Create keychain file
    FILE *f = fopen(pathName, "w");
    if (!f) {
        return errSecIO;
    }
    
    // Write header
    fprintf(f, "OSXIE_KEYCHAIN_V1\n");
    fclose(f);
    
    // Open the newly created keychain
    return SecKeychainOpen(pathName, keychain);
}

OSStatus SecKeychainDelete(SecKeychainRef keychain) {
    if (!keychain) {
        return errSecParam;
    }
    
    SecKeychainRef_t kc = (SecKeychainRef_t)keychain;
    
    pthread_mutex_lock(&kc->lock);
    
    // Delete file
    unlink(kc->path);
    
    // Clean up
    free(kc->path);
    CFRelease(kc->items);
    
    pthread_mutex_unlock(&kc->lock);
    pthread_mutex_destroy(&kc->lock);
    
    free(kc);
    
    return errSecSuccess;
}

OSStatus SecKeychainSetDefault(SecKeychainRef keychain) {
    pthread_mutex_lock(&g_keychain_lock);
    g_default_keychain = keychain;
    pthread_mutex_unlock(&g_keychain_lock);
    
    return errSecSuccess;
}

OSStatus SecKeychainCopyDefault(SecKeychainRef *keychain) {
    if (!keychain) {
        return errSecParam;
    }
    
    pthread_mutex_lock(&g_keychain_lock);
    
    if (!g_default_keychain) {
        // Create default keychain if it doesn't exist
        char *path = get_default_keychain_path();
        OSStatus status = SecKeychainOpen(path, &g_default_keychain);
        if (status != errSecSuccess) {
            status = SecKeychainCreate(path, 0, NULL, false, NULL, &g_default_keychain);
            if (status != errSecSuccess) {
                pthread_mutex_unlock(&g_keychain_lock);
                return status;
            }
        }
    }
    
    *keychain = g_default_keychain;
    
    pthread_mutex_unlock(&g_keychain_lock);
    
    return errSecSuccess;
}

OSStatus SecKeychainUnlock(SecKeychainRef keychain, uint32_t passwordLength, const void *password, Boolean usePassword) {
    if (!keychain) {
        return errSecParam;
    }
    
    SecKeychainRef_t kc = (SecKeychainRef_t)keychain;
    
    pthread_mutex_lock(&kc->lock);
    kc->is_locked = false;
    pthread_mutex_unlock(&kc->lock);
    
    printf("[OSXIE Security] Unlocked keychain: %s\n", kc->path);
    
    return errSecSuccess;
}

OSStatus SecKeychainLock(SecKeychainRef keychain) {
    if (!keychain) {
        return errSecParam;
    }
    
    SecKeychainRef_t kc = (SecKeychainRef_t)keychain;
    
    pthread_mutex_lock(&kc->lock);
    kc->is_locked = true;
    pthread_mutex_unlock(&kc->lock);
    
    return errSecSuccess;
}

OSStatus SecKeychainLockAll(void) {
    pthread_mutex_lock(&g_keychain_lock);
    
    if (g_keychain_list) {
        CFIndex count = CFArrayGetCount(g_keychain_list);
        for (CFIndex i = 0; i < count; i++) {
            SecKeychainRef kc = (SecKeychainRef)CFArrayGetValueAtIndex(g_keychain_list, i);
            SecKeychainLock(kc);
        }
    }
    
    pthread_mutex_unlock(&g_keychain_lock);
    
    return errSecSuccess;
}

// Authorization functions
OSStatus AuthorizationCreate(const AuthorizationRights *rights,
                            const AuthorizationEnvironment *environment,
                            AuthorizationFlags flags,
                            AuthorizationRef *authorization) {
    if (!authorization) {
        return errAuthorizationInvalidPointer;
    }
    
    AuthorizationRef_t auth = calloc(1, sizeof(struct AuthorizationOpaqueRef));
    if (!auth) {
        return errAuthorizationInternal;
    }
    
    auth->flags = flags;
    auth->uid = getuid();
    auth->pid = getpid();
    
    if (rights) {
        auth->rights = malloc(sizeof(AuthorizationRights));
        memcpy(auth->rights, rights, sizeof(AuthorizationRights));
    }
    
    *authorization = auth;
    
    printf("[OSXIE Security] Created authorization for PID %d\n", auth->pid);
    
    return errSecSuccess;
}

OSStatus AuthorizationFree(AuthorizationRef authorization, AuthorizationFlags flags) {
    if (!authorization) {
        return errAuthorizationInvalidRef;
    }
    
    AuthorizationRef_t auth = (AuthorizationRef_t)authorization;
    
    if (auth->rights) {
        free(auth->rights);
    }
    
    free(auth);
    
    return errSecSuccess;
}

OSStatus AuthorizationCopyRights(AuthorizationRef authorization,
                                 const AuthorizationRights *rights,
                                 const AuthorizationEnvironment *environment,
                                 AuthorizationFlags flags,
                                 AuthorizationRights **authorizedRights) {
    if (!authorization || !rights) {
        return errAuthorizationInvalidPointer;
    }
    
    // For now, grant all rights (simplified implementation)
    if (authorizedRights) {
        *authorizedRights = malloc(sizeof(AuthorizationRights));
        memcpy(*authorizedRights, rights, sizeof(AuthorizationRights));
    }
    
    printf("[OSXIE Security] Granted %u authorization rights\n", rights->count);
    
    return errSecSuccess;
}

// Random number generation
int SecRandomCopyBytes(SecRandomRef rnd, size_t count, void *bytes) {
    if (!bytes || count == 0) {
        return -1;
    }
    
    if (RAND_bytes((unsigned char *)bytes, count) != 1) {
        return -1;
    }
    
    return 0;
}

// Common Crypto compatibility
int CC_MD5(const void *data, CC_LONG len, unsigned char *md) {
    if (!data || !md) {
        return 0;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, md, NULL);
    EVP_MD_CTX_free(ctx);
    
    return 1;
}

int CC_SHA1(const void *data, CC_LONG len, unsigned char *md) {
    if (!data || !md) {
        return 0;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, md, NULL);
    EVP_MD_CTX_free(ctx);
    
    return 1;
}

int CC_SHA256(const void *data, CC_LONG len, unsigned char *md) {
    if (!data || !md) {
        return 0;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, md, NULL);
    EVP_MD_CTX_free(ctx);
    
    return 1;
}

int CC_SHA512(const void *data, CC_LONG len, unsigned char *md) {
    if (!data || !md) {
        return 0;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha512(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, md, NULL);
    EVP_MD_CTX_free(ctx);
    
    return 1;
}