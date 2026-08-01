#ifndef SPI_ODX_ABSTRACTION_H
#define SPI_ODX_ABSTRACTION_H

#include "cl/drivers/spi_odx_driver.h"
#include <stdint.h>

/* OxD abstraction layer */
typedef struct {
    odx_spi_bus_t *bus;            /* OxD context pointer */
} spi_odx_abstraction_t;

/** odx_abstraction_execute - abstract OxD operations **/
int odx_abstraction_execute(spi_odx_abstraction_t *ctx, uint16_t opcode);

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_odx_abstraction_t *ctx, uint8_t opcode);

#endif /* SPI_ODX_ABSTRACTION_H */
