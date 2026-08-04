#ifndef SPI_INTERFACE_H_
#define SPI_INTERFACE_H_

#include <stdint.h>
#include "cl/drivers/spi_odx_abstraction_internal.h"

/** spi_interface_t - SPI programming interface **/
typedef struct {
    uint16_t (*transfer)(spi_odx_abstraction_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);
    int (*send)(spi_odx_abstraction_t *ctx, const void *data, size_t len);
    int (*receive)(spi_odx_abstraction_t *ctx, void *rx_buf, size_t len);
    int (*reset)(spi_odx_abstraction_t *ctx);
} spi_interface_t, *spi_interface_handle_t;

#endif /* SPI_INTERFACE_H_ */
