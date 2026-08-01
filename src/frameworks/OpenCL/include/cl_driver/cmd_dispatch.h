/* cmd_dispatch.h — Vulkan command submission */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct CLCommandHandle { void* _ptr; int is_async; } cl_cmd_t, cl_cmd_state_t;

void cl_submit_command(cl_cmd_t* cmd, cl_fence_t fence);
#endif /* cmd_dispatch.h */