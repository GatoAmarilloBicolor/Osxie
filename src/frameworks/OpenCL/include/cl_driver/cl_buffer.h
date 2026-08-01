/* cl_buffer.h — Vulkan buffer type definitions */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct CLBufferHandle { void* _ptr; size_t _size; } cl_buffer_handle_t, cl_buf_state_t;
#endif /* cl_buffer.h */