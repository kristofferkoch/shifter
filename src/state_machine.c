/*
 * state_machine.c
 *
 *  Created on: 6. apr. 2016
 *      Author: kristofferkoch
 */
#include <stdint.h>

#include <hal.h>
#include <state_machine.h>

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

static enum state state = IDLE;
static enum state retstate;
static uint16_t pwm0;
static uint16_t pwm1;
static uint16_t pwm2;
static uint8_t gpio_set;
static uint8_t gpio_get;

static void panic() {
	hal_reset();
	for(;;);
}

void state_machine_handle_rx_byte(uint8_t rx) {
	switch (state) {
	case IDLE:
		switch (rx) {
		case CMD_PWM:
			state = PWM1;
			hal_uart_tx(CMD_PWM);
			break;
		case CMD_SYNC:
			state = SYNC;
			/* Start ADC */
			/* Get gpios */
			/* Set gpios */
			/* Apply PWM */
			hal_uart_tx(CMD_SYNC);
			break;
		case CMD_ID:
			state = ID;
			hal_uart_tx(CMD_ID);
			break;
		case CMD_ADC:
			state = ADC;
			hal_uart_tx(CMD_ADC);
			break;
		case CMD_GET_GPIO:
			state = GET_GPIO;
			hal_uart_tx(CMD_GET_GPIO);
			break;
		case CMD_SET_GPIO:
			state = SET_GPIO1;
			hal_uart_tx(CMD_SET_GPIO);
			break;
		default:
			/* ignore */
			break;
		}
		break;
	case SYNC:
		state = IDLE;
		hal_uart_tx(rx - 1);
		break;
	case GET_GPIO:
		switch (rx) {
		case CMD_GET_GPIO:
			state = FW1;
			retstate = GET_GPIO;
			hal_uart_tx(CMD_GET_GPIO);
			break;
		case CMD_EOF:
			state = GET_GPIO_tx;
			hal_uart_tx(CMD_GET_GPIO);
			break;
		default:
			panic();
		}
		break;
	case ADC:
		switch (rx) {
		case CMD_ADC:
			state = FW2;
			retstate = ADC;
			hal_uart_tx(CMD_ADC);
			break;
		case CMD_EOF:
			state = ADC_tx_msb;
			hal_uart_tx(CMD_ADC);
			break;
		default:
			panic();
		}
		break;
	case ID:
		switch (rx) {
		case CMD_ID:
			state = FW4;
			retstate = ID;
			hal_uart_tx(CMD_ID);
			break;
		case CMD_EOF:
			state = ID_tx0;
			hal_uart_tx(CMD_ID);
			break;
		default:
			panic();
		}
		break;
	case SET_GPIO1:
		switch (rx) {
		case CMD_EOF:
			state = IDLE;
			hal_uart_tx(CMD_EOF);
			break;
		case CMD_SET_GPIO:
			state = SET_GPIO_rx;
			break;
		default:
			panic();
		}
		break;
	case SET_GPIO_rx:
		gpio_set = rx;
		state = SET_GPIO2;
		break;
	case SET_GPIO2:
		switch (rx) {
		case CMD_EOF:
			state = IDLE;
			hal_uart_tx(CMD_EOF);
			break;
		case CMD_SET_GPIO:
			state = FW1;
			retstate = SET_GPIO2;
			break;
		default:
			panic();
		}
		break;
	case PWM1:
		switch (rx) {
		case CMD_EOF:
			state = IDLE;
			hal_uart_tx(CMD_EOF);
			break;
		case CMD_PWM:
			state = PWM_rx_msb0;
			break;
		default:
			panic();
		}
		break;
	case PWM_rx_msb0:
		pwm0 |= rx << 8;
		state = PWM_rx_lsb0;
		break;
	case PWM_rx_lsb0:
		pwm0 |= rx;
		state = PWM_rx_msb1;
		break;
	case PWM_rx_msb1:
		pwm1 |= rx << 8;
		state = PWM_rx_lsb1;
		break;
	case PWM_rx_lsb1:
		pwm1 |= rx;
		state = PWM_rx_msb2;
		break;
	case PWM_rx_msb2:
		pwm2 |= rx << 8;
		state = PWM_rx_lsb2;
		break;
	case PWM_rx_lsb2:
		pwm2 |= rx;
		state = PWM2;
		break;
	case PWM2:
		switch (rx) {
		case CMD_EOF:
			state = IDLE;
			hal_uart_tx(CMD_EOF);
			break;
		case CMD_PWM:
			state = FW6;
			retstate = PWM2;
			hal_uart_tx(CMD_PWM);
			break;
		default:
			panic();
		}
		break;
	case FW6:
		hal_uart_tx(rx);
		state = FW5;
		break;
	case FW5:
		state = FW4;
		hal_uart_tx(rx);
		break;
	case FW4:
		state = FW3;
		hal_uart_tx(rx);
		break;
	case FW3:
		state = FW2;
		hal_uart_tx(rx);
		break;
	case FW2:
		state = FW1;
		hal_uart_tx(rx);
		break;
	case FW1:
		state = retstate;
		hal_uart_tx(rx);
		break;
	}
}

void state_machine_handle_tx_ready(void) {
	switch (state) {
	case ID_tx0:
		state = ID_tx1;
		hal_uart_tx(CHIPID0);
		break;
	case ID_tx1:
		state = ID_tx2;
		hal_uart_tx(CHIPID1);
		break;
	case ID_tx2:
		state = ID_tx3;
		hal_uart_tx(CHIPID2);
		break;
	case ID_tx3:
		state = ID_EOF;
		hal_uart_tx(CHIPID3);
		break;
	case ADC_tx_msb:
		state = ADC_tx_lsb;
		hal_uart_tx(ADC0H);
		break;
	case ADC_tx_lsb:
		state = ADC_EOF;
		hal_uart_tx(ADC0L);
		break;
	case GET_GPIO_tx:
		state = GET_GPIO_EOF;
		hal_uart_tx(gpio_get);
		break;
	case GET_GPIO_EOF:
	case ADC_EOF:
	case ID_EOF:
		state = IDLE;
		hal_uart_tx(CMD_EOF);
		break;
	}
}
