#include "cl/drivers/spi_init_ctx.h"
#include "cl/drivers/internal.h"

/** start_internal_transfer - internal transfer **/
uint16_t start_internal_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(ctx->transfer, ctx, opcode, tx, tx_len);
}

/** start_internal_send - internal send **/
int start_internal_send(spi_init_ctx_t *ctx, const void *data, size_t len) {
    return wiring_execute(ctx->send, ctx, len, data, len);
}

/** start_internal_receive - internal receive **/
int start_internal_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len) {
    return wiring_execute(ctx->receive, ctx, len, rx_buf, len);
}

/** start_internal_reset - internal reset **/
int start_internal_reset(spi_init_ctx_t *ctx) {
    return wiring_execute(ctx->reset, ctx, 0, NULL, 0);
}

#endif /* START_INTERNAL_C */
