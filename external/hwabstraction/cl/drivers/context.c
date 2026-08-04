#include "cl/drivers/context.h"

/** odx_abstraction_validate - abstract result checking **/
static int (*odx_abstraction_validate)(void*, uint8_t) = NULL;

/** context_transfer - combined transfer **/
uint16_t context_transfer(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(ctx->transfer ? ctx->transfer : NULL, ctx, opcode, tx, tx_len);
}

/** context_send - combined send **/
int context_send(spi_abstraction_handle_t ctx, const void *data, size_t len) {
    return wiring_execute(ctx->send ? ctx->send : NULL, ctx, len, data, len);
}

/** context_receive - combined receive **/
int context_receive(spi_abstraction_handle_t ctx, void *rx_buf, size_t len) {
    return wiring_execute(ctx->receive ? ctx->receive : NULL, ctx, len, rx_buf, len);
}

/** context_reset - combined reset **/
int context_reset(spi_abstraction_handle_t ctx) {
    return wiring_execute(ctx->reset ? ctx->reset : NULL, ctx, 0, NULL, 0);
}

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_abstraction_handle_t ctx, uint8_t opcode) {
    return wiring_validate(odx_abstraction_validate ? odx_abstraction_validate : NULL, ctx, opcode, NULL, 0);
}

#ifndef ODAbstractValidateExecute
#define ODAbstractValidateExecute(val, ctx, opcode, data, len) ((val && ctx) ? val(ctx, opcode, data, len) : -1)
#endif

/** wiring_validate - generic result validation **/
int wiring_validate(odx_abstraction_validate_t val, void *ctx, uint8_t opcode, const void *data, size_t len) {
    return ODAbstractValidateExecute(val, ctx, opcode, data, len);
}

#endif /* CONTEXT_C */
