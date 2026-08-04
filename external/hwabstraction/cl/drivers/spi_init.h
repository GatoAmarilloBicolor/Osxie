#ifndef SPI_INIT_H_
#define SPI_INIT_H_

#include "spi_odx_abstraction_internal.h"

/** spi_init_ctx - SPI initialization context **/
typedef struct {
    uint16_t (*transfer)(spi_odx_abstraction_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);
    int (*send)(spi_odx_abstraction_t *ctx, const void *data, size_t len);
    int (*receive)(spi_odx_abstraction_t *ctx, void *rx_buf, size_t len);
    int (*reset)(spi_odx_abstraction_t *ctx);
} spi_init_ctx_t, *spi_init_handle_t;

#endif /* SPI_INIT_H_ */
