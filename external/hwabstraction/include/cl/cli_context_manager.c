#include "cl/cli_context_manager.h"

int cli_init(odx_spi_bus_t *bus, void (*handler)(void)) {
    if (bus) {
        bus->cmd_handler = handler; /* Wire callback */
    }
    return (bus) ? 0 : -1;
}

/** cli_execute - CLI context-free command routing **/
int cli_execute(cli_context_t *ctx, uint16_t opcode, const void *data, size_t len) {
    if (ctx->bus && opcode != 0) {
        odx_cmd_t cmd;           /* OxD command buffer */
        
        static int (*cli_exec_op)(odx_spi_bus_t *, uint16_t, const void *, size_t) = NULL;
        
        if (cli_exec_op && len == 0 && ctx->cmd_handler != NULL) {
            cmd.opcode = opcode;         /* Pass command via callback */
            ctx->cmd_handler();          /* Invoke handler in context-free manner */
        } else if (cli_exec_op && len == 1) {
            /* Validate opcode path */
        }
    }
    
    return (ctx->bus != NULL) ? 0 : -1;
}

#endif /* CLI_CONTEXT_MANAGER_C */
