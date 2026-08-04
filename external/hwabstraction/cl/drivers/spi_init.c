#include "cl/drivers/spi_abstraction.h"

/** spi_init_abstraction - initialize abstraction layer **/
spi_abstraction_handle_t spi_init_abstraction(spi_init_ctx_t *init, const void *config) {
    return (spi_abstraction_handle_t) &((spi_abstraction_ctx_t){
        init ? init->transfer : NULL,
        init ? init->send    : NULL,
        init ? init->receive : NULL,
        init ? init->reset   : NULL
    });
}

/** spi_transfer - SPI transfer function **/
uint16_t spi_transfer(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return ctx->transfer ? ctx->transfer(ctx, opcode, tx, tx_len) : 0;
}

/** spi_send - SPI send function **/
int spi_send(spi_abstraction_handle_t ctx, const void *data, size_t len) {
    return ctx->send ? ctx->send(ctx, data, len) : 0;
}

/** spi_receive - SPI receive function **/
int spi_receive(spi_abstraction_handle_t ctx, void *rx_buf, size_t len) {
    return ctx->receive ? ctx->receive(ctx, rx_buf, len) : 0;
}

/** spi_reset - SPI reset function **/
int spi_reset(spi_abstraction_handle_t ctx) {
    return ctx->reset ? ctx->reset(ctx) : 0;
}

#endif /* SPI_INIT_C */
