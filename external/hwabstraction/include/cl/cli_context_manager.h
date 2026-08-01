#ifndef CLI_CONTEXT_MANAGER_H
#define CLI_CONTEXT_MANAGER_H

#include "cl/drivers/spi_odx_driver.h"
#include <stdint.h>
#include <stddef.h>

/* CLI context */
typedef struct {
    odx_spi_bus_t *bus;            /* OxD driver handle */
    void (*cmd_handler)(void);      /* Command handler callback */
} cli_context_t;

/* Context-free operation wrappers */
int cli_init(odx_spi_bus_t *bus, void (*handler)(void));
int cli_execute(cli_context_t *ctx, uint16_t opcode, const void *data, size_t len);

#endif /* CLI_CONTEXT_MANAGER_H */
