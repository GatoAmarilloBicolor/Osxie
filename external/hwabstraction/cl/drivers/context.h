#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "spi_abstraction.h"
#include "internal.h"

/** context_transfer - combined transfer **/
uint16_t context_transfer(spi_abstraction_handle_t ctx, uint8_t opcode, const void *tx, size_t tx_len);

/** context_send - combined send **/
int context_send(spi_abstraction_handle_t ctx, const void *data, size_t len);

/** context_receive - combined receive **/
int context_receive(spi_abstraction_handle_t ctx, void *rx_buf, size_t len);

/** context_reset - combined reset **/
int context_reset(spi_abstraction_handle_t ctx);

/** odx_abstraction_validate - abstract result checking **/
int odx_abstraction_validate(spi_abstraction_handle_t ctx, uint8_t opcode);

#endif /* CONTEXT_H_ */
