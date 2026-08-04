#include "cl/drivers/main.h"

/** main_transfer - transfer function **/
uint16_t main_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(ctx->transfer ? ctx->transfer : NULL, ctx, opcode, tx, tx_len);
}

/** main_send - send function **/
int main_send(spi_init_ctx_t *ctx, const void *data, size_t len) {
    return wiring_execute(ctx->send ? ctx->send : NULL, ctx, len, data, len);
}

/** main_receive - receive function **/
int main_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len) {
    return wiring_execute(ctx->receive ? ctx->receive : NULL, ctx, len, rx_buf, len);
}

/** main_reset - reset function **/
int main_reset(spi_init_ctx_t *ctx) {
    return wiring_execute(ctx->reset ? ctx->reset : NULL, ctx, 0, NULL, 0);
}

#ifndef ODAbstractValidateExecute
#define ODAbstractValidateExecute(val, ctx, opcode, data, len) ((val && ctx) ? val(ctx, opcode, data, len) : -1)
#endif

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_init_ctx_t *ctx, uint8_t opcode) {
    return wiring_validate(odx_abstraction_validate ? odx_abstraction_validate : NULL, ctx, opcode, NULL, 0);
}

#endif /* MAIN_C */
