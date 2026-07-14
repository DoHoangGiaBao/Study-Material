#include <stm32f4xx_hal.h>

void TIM3_IRQHandler() {
	GPIOA->ODR ^= (1U << 5);

	TIM3->SR &= ~(1U << 0);
}

void init_outputs() {
	// Enable clock for port A and port B
	RCC->AHB1ENR |= ((1U << 0) | (1U << 1));

	// Set PA5 and PB7 as output
	GPIOA->MODER &= ~(3U << 10);
	GPIOA->MODER |= (1U << 10);

	GPIOB->MODER &= ~(3U << 14);
	GPIOB->MODER |= (1U << 14);

	// Set PA5 and PB7 as pull-up
	GPIOA->PUPDR &= ~(3U << 10);
	GPIOA->PUPDR |= (1U << 10);

	GPIOB->PUPDR &= ~(3U << 14);
	GPIOB->PUPDR |= (1U << 14);
}

void init_tim3_interrupt() {
	// Enable clock
	RCC->APB1ENR |= (1U << 1);
	TIM3->CR1 |= (1U << 0);
	TIM3->PSC = 16000 - 1;
	TIM3->ARR = 500 - 1;
	TIM3->DIER |= (1U << 0);
	NVIC_EnableIRQ(TIM3_IRQn);
}

void delay_ms() {
	while (!(TIM3->SR & (1U << 0)));

	TIM3->SR &= ~(1U << 0);
}

int main() {
	init_outputs();
	init_tim3_interrupt();

	while (1) {
		GPIOB->ODR ^= (1U << 7);
		HAL_Delay(1000);
	}
	return 0;
}
