/* osextl.h — Vulkan/OpenCL abstract driver entry point */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct OSEXTLInitProperties {
    void* vulkan_ctx;
    int async_mode;
} osextl_init_props_t, osextl_state_t;

void osextl_init(osextl_init_props_t* props);
#endif /* osextl.h */