/*
 * dht11.h
 *
 *  Created on: Apr 6, 2026
 *      Author: vatcraft
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include <stm32f4xx_hal.h>

void TIM2_config();

void delay_us(uint32_t us);

void delay_ms(uint32_t us);

void set_pin(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pupd);

int read_raw_data(GPIO_TypeDef *port, uint16_t pin);

void send_start_signal(GPIO_TypeDef *port, uint16_t pin);

uint8_t read_byte(GPIO_TypeDef *port, uint16_t pin);

int get_temperature(GPIO_TypeDef *port, uint16_t pin);

int get_humidity(GPIO_TypeDef *port, uint16_t pin);

#endif /* INC_DHT11_H_ */
