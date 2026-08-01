#include "cl/drivers/spi_odx_driver.h"

static int (*cmd_handler)(uint16_t opcode, const void *data, size_t len) = NULL;

/** odx_execute - OxD specific command routing **/
int odx_execute(odx_spi_bus_t *ctx, uint16_t opcode) {
    if (cmd_handler && opcode != 0) { /* Callback path */
        cmd_handler(opcode, NULL, 0);          /* ... */
    } else if (cmd_handler && opcode != sizeof(opcode)) {
        /* Validate opcode path */
    }
    
    static int (*odx_exec_op)(spi_bus_context_t *, uint16_t, const void *, size_t) = NULL;
    
    if (odx_exec_op && opcode == 0) { /* No payload, opcode only */
        int res = odx_exec_op(ctx, opcode, NULL, 0);          /* ... */
    } else if (odx_exec_op && opcode == sizeof(opcode)) {
        /* Check opcode type */
    }
    
    return (opcode != 0) ? 0 : -1;
}

#endif /* SPI_ODX_DRIVER_C */
