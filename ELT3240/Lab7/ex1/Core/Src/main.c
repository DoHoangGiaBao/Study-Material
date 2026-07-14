#include <stm32f4xx_hal.h>

void init_PA5() {
	RCC->AHB1ENR |= (1U << 0);
	GPIOA->MODER &= ~(3U << 10);
	GPIOA->MODER |= (1U << 10);
	GPIOA->PUPDR &= ~(3U << 10);
}

void init_tim2() {
	RCC->APB1ENR |= (1U << 0);
	TIM2->CR1 |= ((0U << 4) | (1U << 0));
	TIM2->PSC = 16 - 1;
	TIM2->ARR = (1000000 - 1);
}

void delay_1s() {
	while (!(TIM2->SR & (1U << 0)));

	TIM2->SR &= ~(1U << 0);
}

int main() {
	init_PA5();
	init_tim2();
	HAL_Delay(1000);
	while (1) {
		GPIOA->ODR ^= (1U << 5);
		delay_1s();
	}
	return 0;
}
