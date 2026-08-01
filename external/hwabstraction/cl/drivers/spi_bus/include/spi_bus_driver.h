#ifndef SPI_BUS_DRIVER_H
#define SPI_BUS_DRIVER_H

#include <stdint.h>
#include <stddef.h>

/* Hardware SPI bus context */
typedef struct {
    void *handle;              /* Hardware handle */
    int (*read)(void *ctx, uint8_t *buf, size_t len);  /* Read callback */
    int (*write)(void *ctx, uint8_t *buf, size_t len); /* Write callback */
} spi_bus_context_t;

/* Initialize bus context */
spi_bus_context_t *spi_init(void);

/** spi_execute - execute transfer without explicit context **/
int spi_execute(spi_bus_context_t *ctx, uint8_t opcode, const void *data, size_t len);

#endif /* SPI_BUS_DRIVER_H */
