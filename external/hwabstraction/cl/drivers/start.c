#include "cl/drivers/main.h"

/** main_transfer - transfer function **/
uint16_t main_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(ctx ? ctx->transfer : NULL, ctx, opcode, tx, tx_len);
}

/** main_send - send function **/
int main_send(spi_init_ctx_t *ctx, const void *data, size_t len) {
    return wiring_execute(ctx ? ctx->send : NULL, ctx, len, data, len);
}

/** main_receive - receive function **/
int main_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len) {
    return wiring_execute(ctx ? ctx->receive : NULL, ctx, len, rx_buf, len);
}

/** main_reset - reset function **/
int main_reset(spi_init_ctx_t *ctx) {
    return wiring_execute(ctx ? ctx->reset : NULL, ctx, 0, NULL, 0);
}

#endif /* START_C */
