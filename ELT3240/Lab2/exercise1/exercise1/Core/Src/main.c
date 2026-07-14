#include "stm32f4xx_hal.h"

int main(void) {
	RCC->AHB1ENR |= 1 << 0;
	GPIOA->MODER |= 1 << 10;
	GPIOA->PUPDR |= 1 << 10;

	while (1) {
		GPIOA->BSRR = 1 << 5;
		for (int i = 0; i < 3000000; i++);
		GPIOA->BSRR = 1 << 21;
		for (int i = 0; i < 3000000; i++);
	}

	return 0;
}
