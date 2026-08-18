#ifndef CONFIG_H
#define CONFIG_H

#define LIB_PATH "/usr/local//osxie"
#define LIBEXEC_PATH "/usr/local/libexec/osxie"
#define LIB_DIR_NAME "lib"
#define INSTALL_PREFIX "/usr/local"
#define SHARE_PATH "/usr/local/share/osxie"
#define DYLD_PATH "/usr/local/bin/dyld"

#define ETC_OSXIE_PATH "/etc/osxie"
// Path where the system root gets "mounted" inside the prefix
#define SYSTEM_ROOT "/Volumes/SystemRoot"

#define GIT_BRANCH "master"
#define GIT_COMMIT_HASH "901250ea3"

/* #undef MULTILIB */

#ifndef __APPLE__

/*
 * Credit:
 *
 *    David Chisnall
 *       http://comments.gmane.org/gmane.comp.desktop.etoile.devel/1556
 *
 *       */

#ifdef __block
#       undef __block
#       include <unistd.h>
#       define __block __attribute__((__blocks__(byref)))
#else
#       include <unistd.h>
#endif

#include <stdint.h>

#define __uint64_t uint64_t
/* #define __darwin_natural_t long */

#endif

#endif
