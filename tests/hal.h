#pragma once
#include <stdint.h>
#define STATIC_STATE

extern uint8_t CHIPID0;
extern uint8_t CHIPID1;
extern uint8_t CHIPID2;
extern uint8_t CHIPID3;

void hal_reset(void);
void hal_uart_tx(uint8_t tx);

void hal_start_adc(void);
uint8_t hal_adc_lsb(void);
uint8_t hal_adc_msb(void);

void hal_set_pwm0(uint16_t set);
void hal_set_pwm1(uint16_t set);
void hal_set_pwm2(uint16_t set);

uint8_t hal_get_gpio(void);
void hal_set_gpio(uint8_t set);
