/*
 * state_machine.c
 *
 *  Created on: 6. apr. 2016
 *      Author: kristofferkoch
 */
#include <stdint.h>

#include <hal.h>
#include <state_machine.h>

STATIC_STATE enum state state = IDLE;
STATIC_STATE enum state retstate;
STATIC_STATE uint16_t pwm0;
STATIC_STATE uint16_t pwm1;
STATIC_STATE uint16_t pwm2;
STATIC_STATE uint8_t gpio_set;
STATIC_STATE uint8_t gpio_get;

static void panic() {
	hal_reset();
	for(;;);
}

static void sync(void) {
  /* Start ADC */
  hal_start_adc();
  /* Get gpios */
  gpio_get = hal_get_gpio();
  /* Set gpios */
  hal_set_gpio(gpio_set);
  /* Apply PWM */
  hal_set_pwm0(pwm0);
  hal_set_pwm1(pwm1);
  hal_set_pwm2(pwm2);
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
			sync();
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
	default:
	        panic();
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
		hal_uart_tx(hal_adc_msb());
		break;
	case ADC_tx_lsb:
		state = ADC_EOF;
		hal_uart_tx(hal_adc_lsb());
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
	default:
	  break;
	}
}
