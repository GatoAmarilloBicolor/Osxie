/* cl_buffer_api.h — Vulkan buffer API */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct CLBufferHandle cl_buffer_handle_t, cl_buf_state_t;

void cl_buffer_create(cl_buffer_handle_t* buf, size_t size);
#endif /* cl_buffer_api.h */