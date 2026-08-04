#ifndef SPI_ABSTRACTION_H_
#define SPI_ABSTRACTION_H_

#include "spi_init_ctx.h"
#include <stdint.h>
#include <stddef.h>

/** spi_abstraction_t - abstraction context **/
typedef struct {
    uint16_t (*transfer)(spi_abstraction_t *ctx, uint8_t opcode, const void *tx, size_t tx_len);
    int (*send)(spi_abstraction_t *ctx, const void *data, size_t len);
    int (*receive)(spi_abstraction_t *ctx, void *rx_buf, size_t len);
    int (*reset)(spi_abstraction_t *ctx);
} spi_abstraction_ctx_t, *spi_abstraction_handle_t;

/** spi_init_abstraction - initialize abstraction layer **/
spi_abstraction_handle_t spi_init_abstraction(spi_init_ctx_t *init, const void *config);

/** spi_transfer - SPI transfer function **/
uint16_t spi_transfer(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len);

/** spi_send - SPI send function **/
int spi_send(spi_abstraction_handle_t ctx, const void *data, size_t len);

/** spi_receive - SPI receive function **/
int spi_receive(spi_abstraction_handle_t ctx, void *rx_buf, size_t len);

/** spi_reset - SPI reset function **/
int spi_reset(spi_abstraction_handle_t ctx);

#ifndef ODAbstractValidateExecute
#define ODAbstractValidateExecute(val, ctx, opcode, data, len) ((val && ctx) ? val(ctx, opcode, data, len) : -1)
#endif

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_abstraction_handle_t ctx, uint8_t opcode);

#endif /* SPI_ABSTRACTION_H_ */
