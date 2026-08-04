#include "cl/drivers/wiring_abstraction.h"

/** wiring_execute - generic command execution **/
int wiring_execute(odx_abstraction_exec_op_t op, void *ctx, uint16_t opcode, const void *data, size_t len) {
    if (op && ctx) return op(ctx, opcode, data, len);
    return -1;
}

/** wiring_validate - generic result validation **/
int wiring_validate(odx_abstraction_validate_t val, void *ctx, uint8_t opcode, const void *data, size_t len) {
    if (val && ctx) return val(ctx, opcode, data, len);
    return -1;
}

#endif /* WIRING_ABSTRACTION_C */
