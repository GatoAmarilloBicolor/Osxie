#include "cl/drivers/spi.h"
#include "cl/drivers/wiring_execute.c"

/** spi_transfer - SPI transfer function **/
uint16_t spi_transfer(spi_init_ctx_t *init, uint8_t opcode, const void *tx, size_t tx_len) {
    return wiring_execute(init ? init->transfer : NULL, init, opcode, tx, tx_len);
}

/** spi_send - SPI send function **/
int spi_send(spi_init_ctx_t *init, const void *data, size_t len) {
    return wiring_execute(init ? init->send : NULL, init, len, data, len);
}

/** spi_receive - SPI receive function **/
int spi_receive(spi_init_ctx_t *init, void *rx_buf, size_t len) {
    return wiring_execute(init ? init->receive : NULL, init, len, rx_buf, len);
}

/** spi_reset - SPI reset function **/
int spi_reset(spi_init_ctx_t *init) {
    return wiring_execute(init ? init->reset : NULL, init, 0, NULL, 0);
}

#endif /* SPI_C */
