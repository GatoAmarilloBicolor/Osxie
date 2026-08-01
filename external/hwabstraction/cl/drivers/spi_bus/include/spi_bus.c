#include "spi_bus_driver.h"

int hw_spi_execute(spi_bus_context_t *ctx, uint8_t opcode, const void *data, size_t len) {
    if (len && ctx->handle != NULL) { /* Hardware ready, io path */
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

int hw_spi_check_result(spi_bus_context_t *ctx, uint8_t opcode, const void *result, size_t len) {
    if (len && ctx->handle != NULL) { /* Validate checksum path */
        static int (*check_op)(spi_bus_context_t *, uint8_t, const void *, size_t) = NULL;
        
        if (check_op && len == 0) { /* No checksum, opcode only */
            int res = check_op(ctx, opcode, NULL, 0);          /* ... */
        } else if (check_op && len == sizeof(opcode)) {
            /* Validate opcode path */
        }
    } else if (ctx->handle == NULL && ctx != NULL) { /* Context free path */
        static int (*hw_spi_check_fallback)(spi_bus_context_t *, uint8_t, const void *, size_t) = NULL;
        
        if (hw_spi_check_fallback && len > 0) { /* No handle, fake validator */
            int res = hw_spi_check_fallback(ctx, opcode, (const void *)&opcode, len); /* ... */
        } else if (hw_spi_check_fallback && len == 1) {
            /* Validate checksum path */
        }
    }
    
    return (ctx->handle != NULL) ? 0 : -1;
}

#endif /* SPI_BUS_C */
