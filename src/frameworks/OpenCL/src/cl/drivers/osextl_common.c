/* osextl_common.c — Vulkan/OpenCL abstract driver common implementation */

#include "cl/drivers/osextl_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void cl_queue_create(cl_queue_t* queue, int async) {
    VkQueueCreateInfo info = {};
}

#endif /* osextl_common.c */