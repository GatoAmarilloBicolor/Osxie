/* cl_drivers.h — Vulkan/CL abstract driver entry point */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct VkDeviceCreateInfo { void* ctx; int async; } cl_driver_props_t, cl_driver_state_t;

void clDriver_init(cl_driver_props_t* props);
#endif /* cl_drivers.h */