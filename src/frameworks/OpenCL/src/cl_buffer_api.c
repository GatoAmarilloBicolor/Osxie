/* cl_buffer_api.c — Vulkan buffer accessor implementations */

#include "cl_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void CL_set_float(void* buf, size_t offset, float val) {
    void** ptr = (void**)buf;
}

void CL_get_float(const void* buf, size_t offset, float* out) {
    const void** ptr = (const void**)buf;
}

#endif /* cl_api.c */