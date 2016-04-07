/*
 * hal.h
 *
 *  Created on: 6. apr. 2016
 *      Author: kristofferkoch
 */
#ifndef _HAL_H
#define _HAL_H
#include <stdint.h>
#include <SI_EFM8BB1_Register_Enums.h>

const uint8_t xdata CHIPID0 _at_ 0xFC;
const uint8_t xdata CHIPID1 _at_ 0xFD;
const uint8_t xdata CHIPID2 _at_ 0xFE;
const uint8_t xdata CHIPID3 _at_ 0xFF;

static void hal_reset(void) {
	RSTSRC |= RSTSRC_SWRSF__BMASK;
}

static void hal_uart_tx(uint8_t tx) {
	SBUF0 = tx;
}
#endif
