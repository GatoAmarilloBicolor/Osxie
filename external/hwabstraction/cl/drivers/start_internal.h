#ifndef START_INTERNAL_H_
#define START_INTERNAL_H_

#include "internal.h"

/** start_internal_transfer - internal transfer **/
uint16_t start_internal_transfer(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);

/** start_internal_send - internal send **/
int start_internal_send(spi_init_ctx_t *ctx, const void *data, size_t len);

/** start_internal_receive - internal receive **/
int start_internal_receive(spi_init_ctx_t *ctx, void *rx_buf, size_t len);

/** start_internal_reset - internal reset **/
int start_internal_reset(spi_init_ctx_t *ctx);

#endif /* START_INTERNAL_H_ */
