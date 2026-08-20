/*
 * osxie_stub_log.h — Shared stub logging infrastructure for Osxie.
 *
 * Always logs to ~/.osxie/logs/stubs.log (persistent, always-on).
 * Deduplicates repeated calls per compilation unit.
 * Respects STUB_VERBOSE env var for stderr output.
 *
 * Usage in any .c/.m file:
 *
 *   #include <osxie_stub_log.h>
 *   OSXIE_STUB_INIT
 *
 *   void SomeFunction(void) {
 *       OSXIE_STUB_CALLED;
 *       return NULL;
 *   }
 */

#ifndef OSXIE_STUB_LOG_H
#define OSXIE_STUB_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

/* ------------------------------------------------------------------ */
/* Persistent log + per-TU dedup hash table                            */
/* ------------------------------------------------------------------ */

#define OSXIE_STUB_LOG_MAX_ENTRIES 512
#define OSXIE_STUB_LOG_SUMMARY_THRESHOLD 20

typedef struct {
    const char *name;
    unsigned long count;
} _osxie_stub_entry_t;

typedef struct {
    _osxie_stub_entry_t entries[OSXIE_STUB_LOG_MAX_ENTRIES];
    unsigned int count;
    int verbose;
    FILE *logfp;
    pid_t pid;
    pthread_mutex_t lock;
} _osxie_stub_state_t;

static _osxie_stub_state_t _osxie_stub_state;
static int _osxie_stub_state_initialized = 0;

static inline unsigned long _osxie_stub_hash(const char *s) {
    unsigned long h = 5381;
    while (*s) h = h * 33 + (unsigned char)*s++;
    return h;
}

static inline void _osxie_stub_state_init(void) {
    if (__sync_bool_compare_and_swap(&_osxie_stub_state_initialized, 0, 1)) {
        memset(&_osxie_stub_state, 0, sizeof(_osxie_stub_state));
        pthread_mutex_init(&_osxie_stub_state.lock, NULL);
        _osxie_stub_state.pid = getpid();
        _osxie_stub_state.verbose = getenv("STUB_VERBOSE") != NULL;

        const char *home = getenv("HOME");
        if (home) {
            char path[512];
            snprintf(path, sizeof(path), "%s/.osxie/logs/stubs.log", home);
            /* Best-effort mkdir for logs dir */
            char dir[512];
            snprintf(dir, sizeof(dir), "%s/.osxie/logs", home);
            mkdir(dir, 0755);
            _osxie_stub_state.logfp = fopen(path, "a");
        }
    }
}

static inline void _osxie_stub_log_impl(const char *funcname) {
    _osxie_stub_state_init();

    pthread_mutex_lock(&_osxie_stub_state.lock);

    unsigned long h = _osxie_stub_hash(funcname) % OSXIE_STUB_LOG_MAX_ENTRIES;
    unsigned int start = (unsigned int)h;

    /* Linear probing to find existing entry or first empty slot */
    while (_osxie_stub_state.entries[h].name != NULL &&
           _osxie_stub_state.entries[h].count > 0 &&
           strcmp(_osxie_stub_state.entries[h].name, funcname) != 0) {
        h = (h + 1) % OSXIE_STUB_LOG_MAX_ENTRIES;
        if (h == start) {
            /* Table full — drop this entry silently */
            pthread_mutex_unlock(&_osxie_stub_state.lock);
            return;
        }
    }

    _osxie_stub_entry_t *e = &_osxie_stub_state.entries[h];

    if (e->name != NULL && strcmp(e->name, funcname) == 0) {
        /* Existing entry — increment count, log summary at threshold */
        e->count++;
        if (e->count == OSXIE_STUB_LOG_SUMMARY_THRESHOLD && _osxie_stub_state.logfp) {
            fprintf(_osxie_stub_state.logfp,
                    "[stub] pid=%d [%s] STUB '%s' called %lu times (suppressing further repeats)\n",
                    _osxie_stub_state.pid, "summary", funcname, e->count);
            fflush(_osxie_stub_state.logfp);
        }
    } else {
        /* New entry */
        e->name = funcname;
        e->count = 1;

        if (_osxie_stub_state.logfp) {
            time_t now = time(NULL);
            struct tm tm_buf;
            struct tm *tm = localtime_r(&now, &tm_buf);
            char ts[32];
            strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
            fprintf(_osxie_stub_state.logfp,
                    "[stub] pid=%d t=%s STUB '%s'\n",
                    _osxie_stub_state.pid, ts, funcname);
            fflush(_osxie_stub_state.logfp);
        }

        if (_osxie_stub_state.verbose) {
            fprintf(stderr, "STUB: %s called\n", funcname);
        }
    }

    pthread_mutex_unlock(&_osxie_stub_state.lock);
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/* Place once per compilation unit, at file scope (replaces the old
 * static int verbose + __attribute__((constructor)) initme boilerplate). */
#define OSXIE_STUB_INIT \
    __attribute__((constructor)) \
    static void _osxie_stub_init_once(void) { \
        _osxie_stub_state_init(); \
    }

/* Place inside every stub function body (replaces puts/printf). */
#define OSXIE_STUB_CALLED \
    _osxie_stub_log_impl(__PRETTY_FUNCTION__)

/* Convenience macro: init + stub body in one line, for trivial files */
#define OSXIE_STUB_FUNCTION(rettype, name) \
    rettype name(void) { \
        OSXIE_STUB_CALLED; \
        return (rettype)0; \
    }

#endif /* OSXIE_STUB_LOG_H */
