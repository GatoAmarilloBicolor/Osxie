#ifndef WIRING_ABSTRACTION_H_
#define WIRING_ABSTRACTION_H_

#include <stdint.h>
#include "cl/drivers/spi_odx_abstraction_internal.h"

extern int wiring_execute(odx_abstraction_exec_op_t op, void *ctx, uint16_t opcode, const void *data, size_t len);
extern int wiring_validate(odx_abstraction_validate_t val, void *ctx, uint8_t opcode, const void *data, size_t len);

#endif /* WIRING_ABSTRACTION_H_ */
