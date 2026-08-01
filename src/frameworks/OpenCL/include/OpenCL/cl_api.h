/* cl_api.h — OpenCL-style C interface */

#include "CLBuffer.h"
#include "CLCommandQueue.h"

typedef struct cl_command_buffer {
    void* _cmd;
} cl_command_t;

void CL_set_float(void* buf, size_t offset, float val);
void CL_get_float(const void* buf, size_t offset, float* out);

#endif /* cl_api.h */