#include "cl/drivers/spi_odx_abstraction_internal.h"

/** wiring_execute - generic command execution **/
int wiring_execute(odx_abstraction_exec_op_t op, void *ctx, uint16_t opcode, const void *data, size_t len) {
    if (op && ctx) return op(ctx, opcode, data, len);
    return -1;
}

#endif /* WIRING_EXECUTE_C */
