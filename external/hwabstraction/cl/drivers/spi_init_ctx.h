#ifndef SPI_INIT_CTX_H_
#define SPI_INIT_CTX_H_

#include "spi_abstraction.h"
#include <stdint.h>
#include <stddef.h>

/** spi_init_ctx_t - SPI initialization context **/
typedef struct {
    uint16_t (*transfer)(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len);
    int (*send)(spi_abstraction_handle_t ctx, const void *data, size_t len);
    int (*receive)(spi_abstraction_handle_t ctx, void *rx_buf, size_t len);
    int (*reset)(spi_abstraction_handle_t ctx);
} spi_init_ctx_t, *spi_init_handle_t;

#endif /* SPI_INIT_CTX_H_ */
