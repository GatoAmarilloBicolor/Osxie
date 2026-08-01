/* cl_driver.h — Vulkan/CL abstract driver API */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct VkDeviceCreateInfo VkInitProps;

void clDriver_init(VkInitProps* props);
#endif /* cl_driver.h */