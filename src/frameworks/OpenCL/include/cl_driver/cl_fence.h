/* cl_fence.h — Vulkan fence synchronization */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct CLFenceHandle { void* _ptr; int is_synced; } cl_fence_t, cl_fence_state_t;

void cl_fence_create(cl_fence_t* fence);
#endif /* cl_fence.h */