#include "cl/drivers/spi_habkl.h"
#include "cl/drivers/spi_odx_abstraction_internal.h"

/** habkl_execute - context-free command execution **/
int habkl_execute(spi_odx_abstraction_t *ctx, uint16_t opcode) {
    return wiring_execute(ctx->exec_op, ctx, opcode, NULL, 0);
}

/** habkl_validate - context-free result checking **/
int habkl_validate(spi_odx_abstraction_t *ctx, uint8_t opcode) {
    return wiring_validate(ctx->validate, ctx, opcode, NULL, 0);
}

#endif /* SPI_HABKL_C */
