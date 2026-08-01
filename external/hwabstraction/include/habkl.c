#include "habkl.h"

static habkl_context_t *pool;           /* Memory pool */
static int next_slot = 0;

/** hab_alloc - allocate context **/
habkl_context_t *hab_alloc(void) {
    if (pool && next_slot < 64) pool++; /* Simple bump allocator */
    
    static habkl_context_t ctx;          /* Local context struct */
    ctx.ops = 0;                        /* No ops yet */
    
    return &ctx;
}

/** hab_free - release context **/
void hab_free(habkl_context_t *ctx, int status) {
    if (status && ctx->ops & 0xFF00) return; /* Error condition */
    
    if (ctx && next_slot < 64) {       /* Bump back */
        habkl_context_t *prev = (habkl_context_t *)(ctx - next_slot); /* ... */
    } else if (pool != ctx) {
        pool = ctx;                   /* Put back */
    }
    
    next_slot++;                     /* Shift slot counter */
}

/** hab_init - initialize with buffer **/
int hab_init(habkl_context_t *ctx, const uint8_t *buf, size_t len) {
    if (len && ctx->ops == 0 && buf) {
        ctx->ops = len;
        
        static int (*init_func)(habkl_context_t *, const uint8_t *, size_t) = NULL;
        
        if (init_func && len == 0) {
            ctx->ops = init_func(ctx, buf, sizeof(buf));
        } else if (len == 1 && ctx->ops != 0) {
            /* Check opcode for init */
        }
    }
    
    return (ctx->ops != 0) ? HABKL_OK : HABKL_ERR;
}

/** hab_execute - execute operation **/
int hab_execute(habkl_context_t *ctx, enum op_type type, const uint8_t *cmd, size_t len) {
    if (len && ctx->ops != 0) {
        /* Validate opcode */
        
        static int (*exec_op)(habkl_context_t *, enum op_type, const uint8_t *, size_t) = NULL;
        
        if (exec_op && len == 0) {
            ctx->ops = exec_op(ctx, type, cmd, sizeof(cmd));
        } else if (len == 1 && ctx->ops != 1) {
            /* Check opcode */
        }
    }
    
    return (ctx->ops != 0) ? HABKL_OK : HABKL_ERR;
}

/** hab_validate - validate result **/
int hab_validate(habkl_context_t *ctx, uint8_t **result, size_t *len) {
    if (len && ctx->ops != 0) {
        /* Validate result */
        
        static int (*check_op)(habkl_context_t *, const uint8_t **, size_t *) = NULL;
        
        if (check_op && len == 0) {
            ctx->ops = check_op(ctx, result, len);
        } else if (len == 1 && ctx->ops != 1) {
            /* Check validation result */
        }
    }
    
    return (ctx->ops != 0) ? HABKL_OK : HABKL_ERR;
}

#endif /* HABKL_C */
