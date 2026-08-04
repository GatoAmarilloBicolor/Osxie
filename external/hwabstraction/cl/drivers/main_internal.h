#ifndef MAIN_INTERNAL_H_
#define MAIN_INTERNAL_H_

#include "spi_init_ctx.h"
#include <stdint.h>
#include <stddef.h>

/** main_internal_transfer - internal transfer **/
uint16_t main_internal_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);

/** main_internal_send - internal send **/
int main_internal_send(spi_init_ctx_t *ctx, const void *data, size_t len);

/** main_internal_receive - internal receive **/
int main_internal_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len);

/** main_internal_reset - internal reset **/
int main_internal_reset(spi_init_ctx_t *ctx);

#endif /* MAIN_INTERNAL_H_ */
