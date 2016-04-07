/*
 * state_machine.h
 *
 *  Created on: 6. apr. 2016
 *      Author: kristofferkoch
 */
#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H
#include <stdint.h>
void state_machine_handle_rx_byte(uint8_t rx);
void state_machine_handle_tx_ready(void);
#endif
