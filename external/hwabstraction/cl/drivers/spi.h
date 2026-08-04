#ifndef SPI_H_
#define SPI_H_

#include "spi_init_ctx.h"
#include <stdint.h>
#include <stddef.h>

/** spi_transfer - SPI transfer function **/
uint16_t spi_transfer(spi_init_ctx_t *init, uint8_t opcode, const void *tx, size_t tx_len);

/** spi_send - SPI send function **/
int spi_send(spi_init_ctx_t *init, const void *data, size_t len);

/** spi_receive - SPI receive function **/
int spi_receive(spi_init_ctx_t *init, void *rx_buf, size_t len);

/** spi_reset - SPI reset function **/
int spi_reset(spi_init_ctx_t *init);

#endif /* SPI_H_ */
