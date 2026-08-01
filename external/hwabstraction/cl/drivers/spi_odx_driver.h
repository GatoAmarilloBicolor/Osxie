#ifndef SPI_ODX_DRIVER_H
#define SPI_ODX_DRIVER_H

#include "cl/drivers/spi_bus/include/spi_bus_driver.h"
#include <stdint.h>

/* OxD Command structure */
typedef struct {
    uint16_t opcode;
    union {
        void *data;
        uint8_t *bytes;
    } payload;
    uint32_t len;
} odx_cmd_t;

/** odx_execute - OxD specific command routing **/
int odx_execute(odx_spi_bus_t *ctx, uint16_t opcode);

#endif /* SPI_ODX_DRIVER_H */
