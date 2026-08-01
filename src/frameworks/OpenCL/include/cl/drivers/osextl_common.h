/* osextl_common.h — Vulkan/OpenCL abstract driver common types */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct CLQueueHandle { void* ptr; int async; } cl_queue_t, cl_queue_state_t;
#endif /* osextl_common.h */