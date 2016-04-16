/*
 * state_machine.h
 *
 *  Created on: 6. apr. 2016
 *      Author: kristofferkoch
 */
#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H
#include <stdint.h>

enum cmd {
	CMD_EOF, CMD_ID, CMD_PWM, CMD_SYNC, CMD_ADC, CMD_SET_GPIO, CMD_GET_GPIO,
};

enum state {
	IDLE,
	PWM1, PWM_rx_msb0, PWM_rx_lsb0, PWM_rx_msb1, PWM_rx_lsb1, PWM_rx_msb2, PWM_rx_lsb2,
	PWM2, FW6, FW5, FW4, FW3, FW2, FW1,
	SYNC,
	ID, ID_tx0, ID_tx1, ID_tx2, ID_tx3, ID_EOF,
	ADC, ADC_tx_msb, ADC_tx_lsb, ADC_EOF,
	SET_GPIO1, SET_GPIO_rx, SET_GPIO2,
	GET_GPIO, GET_GPIO_tx, GET_GPIO_EOF,
};

void state_machine_handle_rx_byte(uint8_t rx);
void state_machine_handle_tx_ready(void);
#endif
