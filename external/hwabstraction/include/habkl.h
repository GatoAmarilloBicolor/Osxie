#ifndef HABKL_H
#define HABKL_H

#include <stdint.h>
#include <stddef.h>

/* Habkl Error Codes */
#define HABKL_OK         0
#define HABKL_ERR       -1
#define HABKL_ERR_TIMEOUT -2

/* Operation types */
enum {
    OP_READ,
    OP_WRITE,
    OP_CONFIG,
    OP_FREEZE
};

/* Context structure */
typedef struct {
    void *dev;              /* Device pointer */
    uint32_t ops;           /* Supported operations bitfield */
} habkl_context_t;

/* Core functions */
habkl_context_t *hab_alloc(void);
void hab_free(habkl_context_t *ctx, int status);

int hab_init(habkl_context_t *ctx, const uint8_t *buf, size_t len);
int hab_execute(habkl_context_t *ctx, enum op_type type, const uint8_t *cmd, size_t len);
int hab_check_result(habkl_context_t *ctx, const uint8_t *result, size_t len);

#endif /* HABKL_H */
