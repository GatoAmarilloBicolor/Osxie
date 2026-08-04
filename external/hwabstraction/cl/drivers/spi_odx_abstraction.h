#ifndef SPI_ODX_ABSTRACTION_H_
#define SPI_ODX_ABSTRACTION_H_

#include <stdint.h>
#include "cl/drivers/spi_interface.h"
#include "spi_odx_abstraction_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** spi_odx_abstraction_init - initialize abstraction layer **/
spi_odx_abstraction_t *odx_abstraction_init(spi_interface_t *spi, const void *config);

/** odx_abstraction_execute - abstract OxD operations **/
int odx_abstraction_execute(spi_odx_abstraction_t *ctx, uint16_t opcode);

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_odx_abstraction_t *ctx, uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif /* SPI_ODX_ABSTRACTION_H_ */
