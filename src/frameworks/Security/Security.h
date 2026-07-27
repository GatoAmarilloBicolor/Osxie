/**
 * Osxie Security Framework
 * Core security services for macOS compatibility
 */

#ifndef OSXIE_SECURITY_H
#define OSXIE_SECURITY_H

#include <stdint.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

// Security result codes
typedef int32_t OSStatus;
enum {
    errSecSuccess = 0,
    errSecUnimplemented = -4,
    errSecIO = -36,
    errSecParam = -50,
    errSecAllocate = -108,
    errSecUserCanceled = -128,
    errSecBadReq = -909,
    errSecInternalComponent = -2070,
    errSecCoreFoundationUnknown = -4960,
    
    // Keychain errors
    errSecNoSuchKeychain = -25294,
    errSecInvalidKeychain = -25295,
    errSecDuplicateKeychain = -25296,
    errSecDuplicateItem = -25299,
    errSecItemNotFound = -25300,
    errSecBufferTooSmall = -25301,
    errSecDataTooLarge = -25302,
    errSecNoSuchAttr = -25303,
    errSecInvalidItemRef = -25304,
    errSecInvalidSearchRef = -25305,
    errSecNoSuchClass = -25306,
    errSecNoDefaultKeychain = -25307,
    errSecInteractionNotAllowed = -25308,
    errSecReadOnlyAttr = -25309,
    errSecWrongSecVersion = -25310,
    errSecKeySizeNotAllowed = -25311,
    errSecNoStorageModule = -25312,
    errSecNoCertificateModule = -25313,
    errSecNoPolicyModule = -25314,
    errSecInteractionRequired = -25315,
    errSecDataNotAvailable = -25316,
    errSecDataNotModifiable = -25317,
    errSecCreateChainFailed = -25318,
    errSecInvalidPrefsDomain = -25319,
    errSecInDarkWake = -25320,
    
    // Authorization errors
    errAuthorizationInvalidSet = -60001,
    errAuthorizationInvalidRef = -60002,
    errAuthorizationInvalidTag = -60003,
    errAuthorizationInvalidPointer = -60004,
    errAuthorizationDenied = -60005,
    errAuthorizationCanceled = -60006,
    errAuthorizationInteractionNotAllowed = -60007,
    errAuthorizationInternal = -60008,
    errAuthorizationExternalizeNotAllowed = -60009,
    errAuthorizationInternalizeNotAllowed = -60010,
    errAuthorizationInvalidFlags = -60011,
    errAuthorizationToolExecuteFailure = -60031,
    errAuthorizationToolEnvironmentError = -60032,
    errAuthorizationBadAddress = -60033,
};

// Keychain types
typedef struct OpaqueSecKeychainRef *SecKeychainRef;
typedef struct OpaqueSecKeychainItemRef *SecKeychainItemRef;
typedef struct OpaqueSecKeychainSearchRef *SecKeychainSearchRef;
typedef struct OpaqueSecKeychainAttributeList *SecKeychainAttributeList;
typedef struct OpaqueSecAccessRef *SecAccessRef;
typedef struct OpaqueSecTrustedApplicationRef *SecTrustedApplicationRef;
typedef struct OpaqueSecACLRef *SecACLRef;
typedef struct OpaqueSecPasswordRef *SecPasswordRef;
typedef struct OpaqueSecCertificateRef *SecCertificateRef;
typedef struct OpaqueSecKeyRef *SecKeyRef;
typedef struct OpaqueSecIdentityRef *SecIdentityRef;
typedef struct OpaqueSecIdentitySearchRef *SecIdentitySearchRef;
typedef struct OpaqueSecPolicyRef *SecPolicyRef;
typedef struct OpaqueSecPolicySearchRef *SecPolicySearchRef;
typedef struct OpaqueSecTrustRef *SecTrustRef;

// Authorization types
typedef struct AuthorizationOpaqueRef *AuthorizationRef;
typedef struct AuthorizationOpaqueRef *AuthorizationExternalForm;

typedef struct {
    char name[1024];
    size_t valueLength;
    void *value;
    uint32_t flags;
} AuthorizationItem;

typedef struct {
    uint32_t count;
    AuthorizationItem *items;
} AuthorizationItemSet;

typedef struct {
    uint32_t count;
    AuthorizationItem *items;
} AuthorizationRights;

typedef struct {
    uint32_t count;
    AuthorizationItem *items;
} AuthorizationEnvironment;

// Authorization flags
typedef enum {
    kAuthorizationFlagDefaults = 0,
    kAuthorizationFlagInteractionAllowed = (1 << 0),
    kAuthorizationFlagExtendRights = (1 << 1),
    kAuthorizationFlagPartialRights = (1 << 2),
    kAuthorizationFlagDestroyRights = (1 << 3),
    kAuthorizationFlagPreAuthorize = (1 << 4),
    kAuthorizationFlagNoData = (1 << 20),
} AuthorizationFlags;

// Keychain attribute types
typedef enum {
    kSecCreationDateItemAttr = 'cdat',
    kSecModDateItemAttr = 'mdat',
    kSecDescriptionItemAttr = 'desc',
    kSecCommentItemAttr = 'icmt',
    kSecCreatorItemAttr = 'crtr',
    kSecTypeItemAttr = 'type',
    kSecScriptCodeItemAttr = 'scrp',
    kSecLabelItemAttr = 'labl',
    kSecInvisibleItemAttr = 'invi',
    kSecNegativeItemAttr = 'nega',
    kSecCustomIconItemAttr = 'cusi',
    kSecAccountItemAttr = 'acct',
    kSecServiceItemAttr = 'svce',
    kSecGenericItemAttr = 'gena',
    kSecSecurityDomainItemAttr = 'sdmn',
    kSecServerItemAttr = 'srvr',
    kSecAuthenticationTypeItemAttr = 'atyp',
    kSecPortItemAttr = 'port',
    kSecPathItemAttr = 'path',
    kSecVolumeItemAttr = 'vlme',
    kSecAddressItemAttr = 'addr',
    kSecSignatureItemAttr = 'ssig',
    kSecProtocolItemAttr = 'ptcl',
    kSecCertificateType = 'ctyp',
    kSecCertificateEncoding = 'cenc',
    kSecCrlType = 'crtp',
    kSecCrlEncoding = 'crnc',
    kSecAlias = 'alis',
} SecItemAttr;

// Keychain functions
OSStatus SecKeychainOpen(const char *pathName, SecKeychainRef *keychain);
OSStatus SecKeychainCreate(const char *pathName, uint32_t passwordLength, const void *password,
                           Boolean promptUser, SecAccessRef initialAccess, SecKeychainRef *keychain);
OSStatus SecKeychainDelete(SecKeychainRef keychain);
OSStatus SecKeychainSetDefault(SecKeychainRef keychain);
OSStatus SecKeychainCopyDefault(SecKeychainRef *keychain);
OSStatus SecKeychainUnlock(SecKeychainRef keychain, uint32_t passwordLength, const void *password, Boolean usePassword);
OSStatus SecKeychainLock(SecKeychainRef keychain);
OSStatus SecKeychainLockAll(void);
OSStatus SecKeychainCopySearchList(CFArrayRef *searchList);
OSStatus SecKeychainSetSearchList(CFArrayRef searchList);

// Keychain item functions
OSStatus SecKeychainItemCreateFromContent(SecItemClass itemClass, SecKeychainAttributeList *attrList,
                                          uint32_t length, const void *data, SecKeychainRef keychainRef,
                                          SecAccessRef initialAccess, SecKeychainItemRef *itemRef);
OSStatus SecKeychainItemModifyContent(SecKeychainItemRef itemRef, const SecKeychainAttributeList *attrList,
                                      uint32_t length, const void *data);
OSStatus SecKeychainItemCopyContent(SecKeychainItemRef itemRef, SecItemClass *itemClass,
                                    SecKeychainAttributeList *attrList, uint32_t *length, void **outData);
OSStatus SecKeychainItemDelete(SecKeychainItemRef itemRef);
OSStatus SecKeychainItemCopyKeychain(SecKeychainItemRef itemRef, SecKeychainRef *keychainRef);

// Authorization functions
OSStatus AuthorizationCreate(const AuthorizationRights *rights,
                            const AuthorizationEnvironment *environment,
                            AuthorizationFlags flags,
                            AuthorizationRef *authorization);
OSStatus AuthorizationFree(AuthorizationRef authorization, AuthorizationFlags flags);
OSStatus AuthorizationCopyRights(AuthorizationRef authorization,
                                 const AuthorizationRights *rights,
                                 const AuthorizationEnvironment *environment,
                                 AuthorizationFlags flags,
                                 AuthorizationRights **authorizedRights);
OSStatus AuthorizationCopyInfo(AuthorizationRef authorization,
                               const char *tag,
                               AuthorizationItemSet **info);
OSStatus AuthorizationMakeExternalForm(AuthorizationRef authorization,
                                       AuthorizationExternalForm *extForm);
OSStatus AuthorizationCreateFromExternalForm(const AuthorizationExternalForm *extForm,
                                             AuthorizationRef *authorization);
OSStatus AuthorizationFreeItemSet(AuthorizationItemSet *set);

// Certificate functions
OSStatus SecCertificateCreateFromData(const void *data, CSSM_CERT_TYPE type, CSSM_CERT_ENCODING encoding,
                                      SecCertificateRef *certificate);
OSStatus SecCertificateCopyData(SecCertificateRef certificate, CFDataRef *data);
OSStatus SecCertificateCopySubjectSummary(SecCertificateRef certificate, CFStringRef *summary);
OSStatus SecCertificateCopyCommonName(SecCertificateRef certificate, CFStringRef *commonName);
OSStatus SecCertificateCopyEmailAddresses(SecCertificateRef certificate, CFArrayRef *emailAddresses);

// Trust functions
OSStatus SecTrustCreateWithCertificates(CFTypeRef certificates, CFTypeRef policies, SecTrustRef *trust);
OSStatus SecTrustSetPolicies(SecTrustRef trust, CFTypeRef policies);
OSStatus SecTrustSetVerifyDate(SecTrustRef trust, CFDateRef verifyDate);
OSStatus SecTrustEvaluate(SecTrustRef trust, SecTrustResultType *result);
OSStatus SecTrustGetCertificateCount(SecTrustRef trust, CFIndex *count);
OSStatus SecTrustGetCertificateAtIndex(SecTrustRef trust, CFIndex ix, SecCertificateRef *certificate);

// Key functions
OSStatus SecKeyGeneratePair(SecKeychainRef keychainRef, CSSM_ALGORITHMS algorithm,
                            uint32_t keySizeInBits, CSSM_CC_HANDLE contextHandle,
                            CSSM_KEYUSE publicKeyUsage, uint32_t publicKeyAttr,
                            CSSM_KEYUSE privateKeyUsage, uint32_t privateKeyAttr,
                            SecAccessRef initialAccess, SecKeyRef *publicKey, SecKeyRef *privateKey);
OSStatus SecKeyCreateSignature(SecKeyRef key, SecKeyAlgorithm algorithm,
                               CFDataRef dataToSign, CFErrorRef *error, CFDataRef *signature);
OSStatus SecKeyVerifySignature(SecKeyRef key, SecKeyAlgorithm algorithm,
                               CFDataRef signedData, CFDataRef signature, CFErrorRef *error);
OSStatus SecKeyEncrypt(SecKeyRef key, SecPadding padding,
                       const uint8_t *plainText, size_t plainTextLen,
                       uint8_t *cipherText, size_t *cipherTextLen);
OSStatus SecKeyDecrypt(SecKeyRef key, SecPadding padding,
                       const uint8_t *cipherText, size_t cipherTextLen,
                       uint8_t *plainText, size_t *plainTextLen);

// Random number generation
int SecRandomCopyBytes(SecRandomRef rnd, size_t count, void *bytes);

// Password functions
OSStatus SecPasswordGenerate(CFDictionaryRef parameters, CFErrorRef *error, CFStringRef *password);

// Access control
OSStatus SecAccessCreate(CFStringRef descriptor, CFArrayRef trustedlist, SecAccessRef *accessRef);
OSStatus SecAccessCopyACLList(SecAccessRef accessRef, CFArrayRef *aclList);

// Identity functions
OSStatus SecIdentityCreateWithCertificate(CFTypeRef keychainOrArray, SecCertificateRef certificate,
                                          SecIdentityRef *identityRef);
OSStatus SecIdentityCopyCertificate(SecIdentityRef identityRef, SecCertificateRef *certificateRef);
OSStatus SecIdentityCopyPrivateKey(SecIdentityRef identityRef, SecKeyRef *privateKeyRef);

// Common Crypto compatibility
typedef uint32_t CC_LONG;
typedef uint64_t CC_LONG64;

int CC_MD5(const void *data, CC_LONG len, unsigned char *md);
int CC_SHA1(const void *data, CC_LONG len, unsigned char *md);
int CC_SHA256(const void *data, CC_LONG len, unsigned char *md);
int CC_SHA512(const void *data, CC_LONG len, unsigned char *md);

// Secure transport
typedef struct SSLContext *SSLContextRef;
OSStatus SSLCreateContext(CFAllocatorRef alloc, SSLProtocolSide protocolSide,
                         SSLConnectionType connectionType, SSLContextRef *contextRef);
OSStatus SSLSetIOFuncs(SSLContextRef context, SSLReadFunc readFunc, SSLWriteFunc writeFunc);
OSStatus SSLSetConnection(SSLContextRef context, SSLConnectionRef connection);
OSStatus SSLHandshake(SSLContextRef context);
OSStatus SSLRead(SSLContextRef context, void *data, size_t dataLength, size_t *processed);
OSStatus SSLWrite(SSLContextRef context, const void *data, size_t dataLength, size_t *processed);
OSStatus SSLClose(SSLContextRef context);

#ifdef __cplusplus
}
#endif

#endif // OSXIE_SECURITY_H