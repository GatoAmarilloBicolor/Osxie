#ifndef SPI_HABKL_H_
#define SPI_HABKL_H_

#include "cl/drivers/spi_odx_abstraction.h"
#include <stdint.h>
#include <stddef.h>

/** habkl_execute - context-free command execution **/
int habkl_execute(spi_odx_abstraction_t *ctx, uint16_t opcode);

/** habkl_validate - context-free result checking **/
int habkl_validate(spi_odx_abstraction_t *ctx, uint8_t opcode);

#endif /* SPI_HABKL_H_ */
