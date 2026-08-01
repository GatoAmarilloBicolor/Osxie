/* cmd_pool.h — Vulkan command pool management */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct CLCommandPoolHandle { void* _ptr; int _async; } cl_cmd_pool_t, cl_pool_state_t;

void cl_cmd_pool_create(cl_cmd_pool_t* pool, int async);
#endif /* cmd_pool.h */