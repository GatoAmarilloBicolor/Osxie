#include "cl/drivers/spi_abstraction.h"

/** spi_transfer - SPI transfer function **/
uint16_t spi_transfer(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(ctx->transfer ? ctx->transfer : NULL, ctx, opcode, tx, tx_len);
}

/** spi_send - SPI send function **/
int spi_send(spi_abstraction_handle_t ctx, const void *data, size_t len) {
    return wiring_execute(ctx->send ? ctx->send : NULL, ctx, len, data, len);
}

/** spi_receive - SPI receive function **/
int spi_receive(spi_abstraction_handle_t ctx, void *rx_buf, size_t len) {
    return wiring_execute(ctx->receive ? ctx->receive : NULL, ctx, len, rx_buf, len);
}

/** spi_reset - SPI reset function **/
int spi_reset(spi_abstraction_handle_t ctx) {
    return wiring_execute(ctx->reset ? ctx->reset : NULL, ctx, 0, NULL, 0);
}

/** spi_init_abstraction - initialize abstraction layer **/
spi_abstraction_handle_t spi_init_abstraction(spi_init_ctx_t *init, const void *config) {
    return (spi_abstraction_handle_t) &((spi_abstraction_ctx_t){
        init->transfer,
        init->send,
        init->receive,
        init->reset
    });
}

#ifndef ODAbstractValidateExecute
#define ODAbstractValidateExecute(val, ctx, opcode, data, len) ((val && ctx) ? val(ctx, opcode, data, len) : -1)
#endif

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_abstraction_handle_t ctx, uint8_t opcode) {
    return wiring_validate(odx_abstraction_validate ? odx_abstraction_validate : NULL, ctx, opcode, NULL, 0);
}

#endif /* SPI_ABSTRACTION_C */
