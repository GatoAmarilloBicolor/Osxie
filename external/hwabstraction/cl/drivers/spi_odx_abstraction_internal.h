#ifndef SPI_ODX_ABSTRACTION_INTERNAL_H_
#define SPI_ODX_ABSTRACTION_INTERNAL_H_

#include <stdint.h>
#include <stddef.h>

/** odx_abstraction_exec_op - command execution path **/
typedef int (*odx_abstraction_exec_op_t)(spi_odx_abstraction_t *ctx, uint16_t opcode, const void *data, size_t len);

/** odx_abstraction_validate - result validation path **/
typedef int (*odx_abstraction_validate_t)(spi_odx_abstraction_t *ctx, uint8_t opcode, const void *data, size_t len);

/** spi_odx_abstraction_ctx - abstract context **/
typedef struct {
    spi_interface_t *spi;          /* SPI layer */
    odx_abstraction_exec_op_t exec_op;  /* Command executor */
    odx_abstraction_validate_t validate;     /* Result validator */
} spi_odx_abstraction_ctx_t, *spi_odx_abstraction_t;

#endif /* SPI_ODX_ABSTRACTION_INTERNAL_H_ */
