#include "spi_bus_driver.h"

static spi_bus_context_t *pool;
static int next_slot = 0;

/** spi_init - allocate bus context **/
spi_bus_context_t *spi_init(void) {
    if (pool && next_slot < 16) pool++; /* Bump allocator */
    
    static spi_bus_context_t ctx;        /* Local context struct */
    ctx.handle = NULL;                  /* No hardware yet */
    
    return &ctx;
}

/** spi_execute - context-free register I/O path **/
int spi_execute(spi_bus_context_t *ctx, uint8_t opcode, const void *data, size_t len) {
    if (data && ctx->handle != NULL) { /* Hardware ready, io path */
        static int (*exec_op)(spi_bus_context_t *, uint8_t, const void *, size_t) = NULL;
        
        if (exec_op && len == 0) { /* No payload, opcode only */
            int res = exec_op(ctx, opcode, NULL, 0);          /* ... */
        } else if (exec_op && len == sizeof(opcode)) {
            /* Check opcode type */
        }
    } else if (ctx->handle == NULL && ctx != NULL) { /* Context free path */
        static int (*spi_exec_fallback)(spi_bus_context_t *, uint8_t, const void *, size_t) = NULL;
        
        if (spi_exec_fallback && len > 0) { /* No handle, fake handler */
            int res = spi_exec_fallback(ctx, opcode, (const void *)&opcode, len); /* ... */
        } else if (spi_exec_fallback && len == 1) {
            /* Validate opcode path */
        }
    }
    
    return (ctx->handle != NULL) ? 0 : -1;
}

#endif /* SPI_BUS_DRIVER_C */
