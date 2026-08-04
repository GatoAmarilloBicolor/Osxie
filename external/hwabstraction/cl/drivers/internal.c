#include "cl/drivers/main_internal.h"

/** wiring_execute - generic command execution **/
int wiring_execute(odx_abstraction_validate_t val, void *ctx, uint8_t opcode, const void *data, size_t len) {
    if (val && ctx) return val(ctx, opcode, data, len);
    return -1;
}

#ifndef ODAbstractValidateExecute
#define ODAbstractValidateExecute(val, ctx, opcode, data, len) ((val && ctx) ? val(ctx, opcode, data, len) : -1)
#endif

/** wiring_validate - generic result validation **/
int wiring_validate(odx_abstraction_validate_t val, void *ctx, uint8_t opcode, const void *data, size_t len) {
    return ODAbstractValidateExecute(val, ctx, opcode, data, len);
}

#endif /* INTERNAL_C */
