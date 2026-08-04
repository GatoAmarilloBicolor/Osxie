#ifndef WIRING_ABSTRACTION_INTERNAL_H_
#define WIRING_ABSTRACTION_INTERNAL_H_

#include "spi_init_ctx.h"
#include <stdint.h>
#include <stddef.h>

/** wiring_abstraction_exec_op - execution callback **/
typedef int (*wiring_abstraction_exec_op_t)(spi_init_ctx_t *ctx, uint16_t opcode, const void *data, size_t len);

/** wiring_abstraction_validate - validation callback **/
typedef int (*wiring_abstraction_validate_t)(spi_init_ctx_t *ctx, uint8_t opcode, const void *data, size_t len);

/** wiring_abstraction_ctx - execution context **/
typedef struct {
    uint16_t (*transfer)(spi_init_ctx_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);
    int (*send)(spi_init_ctx_t *ctx, const void *data, size_t len);
    int (*receive)(spi_init_ctx_t *ctx, void *rx_buf, size_t len);
    int (*reset)(spi_init_ctx_t *ctx);
} wiring_abstraction_ctx_t, *wiring_abstraction_handle_t;

#endif /* WIRING_ABSTRACTION_INTERNAL_H_ */
