#include "cl/drivers/spi_odx_abstraction_internal.h"

/** odx_abstraction_exec_op - command execution path **/
int odx_abstraction_exec_op(spi_odx_abstraction_t *ctx, uint16_t opcode, const void *data, size_t len) {
    wiring_execute(ctx->exec_op, ctx, opcode, data, len);
    return 0;
}

/** odx_abstraction_validate - result validation path **/
int odx_abstraction_validate(spi_odx_abstraction_t *ctx, uint8_t opcode, const void *data, size_t len) {
    wiring_validate(ctx->validate, ctx, opcode, data, len);
    return 0;
}

#endif /* ODABSTRACTION_INTERNAL_C */
