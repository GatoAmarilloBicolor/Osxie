#include "cl/drivers/spi_odx_abstraction.h"

static int (*cmd_handler)(uint16_t opcode, const void *data, size_t len) = NULL;

/** odx_abstraction_execute - abstract OxD operations **/
int odx_abstraction_execute(spi_odx_abstraction_t *ctx, uint16_t opcode) {
    if (cmd_handler && opcode != 0) { /* Callback path */
        cmd_handler(opcode, NULL, 0);          /* ... */
    } else if (cmd_handler && opcode != sizeof(opcode)) {
        /* Validate opcode path */
    }
    
    static int (*odx_abstraction_exec_op)(spi_odx_abstraction_t *, uint16_t, const void *, size_t) = NULL;
    
    if (odx_abstraction_exec_op && opcode == 0) { /* No payload, opcode only */
        int res = odx_abstraction_exec_op(ctx, opcode, NULL, 0);          /* ... */
    } else if (odx_abstraction_exec_op && opcode == sizeof(opcode)) {
        /* Check opcode type */
    }
    
    return (opcode != 0) ? 0 : -1;
}

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_odx_abstraction_t *ctx, uint8_t opcode) {
    static int (*check_op)(spi_odx_abstraction_t *, uint8_t, const void *, size_t) = NULL;
    
    if (check_op && opcode == 0) { /* No payload, opcode only */
        int res = check_op(ctx, opcode, NULL, 0);          /* ... */
    } else if (check_op && opcode == sizeof(opcode)) {
        /* Validate opcode path */
    }
    
    return (opcode != 0) ? 0 : -1;
}

#endif /* SPI_ODX_ABSTRACTION_C */
