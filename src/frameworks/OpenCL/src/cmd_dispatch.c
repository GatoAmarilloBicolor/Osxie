/* cmd_dispatch.c — Vulkan command submission */

#include "cl_driver/cmd_dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void cl_submit_command(cl_cmd_t* cmd, cl_fence_t fence) {
    VkCommandBufferBeginInfo info = {};
}

#endif /* cmd_dispatch.c */