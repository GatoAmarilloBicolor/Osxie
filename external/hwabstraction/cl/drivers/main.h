#ifndef MAIN_H_
#define MAIN_H_

#include "spi_init_ctx.h"
#include <stdint.h>
#include <stddef.h>

/** main_transfer - transfer function **/
uint16_t main_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);

/** main_send - send function **/
int main_send(spi_init_ctx_t *ctx, const void *data, size_t len);

/** main_receive - receive function **/
int main_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len);

/** main_reset - reset function **/
int main_reset(spi_init_ctx_t *ctx);

#endif /* MAIN_H_ */
