#ifndef _OSXIESERVER_DUCT_TAPE_STUBS_H_
#define _OSXIESERVER_DUCT_TAPE_STUBS_H_

#include <stdbool.h>

void dtape_stub_log(const char* function_name, int safety, const char* subsection);

// for general functions where it's unknown whether they can be safely stubbed or not
#define dtape_stub(...) (dtape_stub_log(__FUNCTION__, 0, "" __VA_ARGS__))

// for functions that have been confirmed to be okay being stubbed
#define dtape_stub_safe(...) (dtape_stub_log(__FUNCTION__, 1, "" __VA_ARGS__))

// for functions that have been confirmed to require an actual implementation (rather than a simple stub)
//
// NOTE: this must NOT kill the whole server.  A single unimplemented mach call
// from one guest process used to abort() the shared osxieserver, which then
// broke every other guest process's dserver RPC connection (ECONNREFUSED), and
// they died with SIGILL.  We log loudly and return KERN_FAILURE instead, so the
// server survives and the caller gets a clean error.  Set DTAPE_FATAL_STUBS=1
// to get the old abort-on-unsafe behavior back while debugging.
#define dtape_stub_unsafe(...) ({ \
		dtape_stub_log(__FUNCTION__, -1, "" __VA_ARGS__); \
		KERN_FAILURE; \
	}) \

#endif // _OSXIESERVER_DUCT_TAPE_STUBS_H_
