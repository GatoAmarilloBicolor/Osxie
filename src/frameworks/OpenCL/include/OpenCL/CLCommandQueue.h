/* CLCommandQueue.h — Vulkan async queue abstraction */
#include <stdio.h>
#include <stdlib.h>

typedef struct { void* cmd_pool; int is_async; } CLCommandQueue_t;
#endif /* CLCommandQueue.h */