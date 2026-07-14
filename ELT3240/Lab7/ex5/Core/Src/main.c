#include <stm32f4xx_hal.h>

void tim2_config() {
	// Enable clocks
	RCC->ABB1ENR |= (1U << 0);
	RCC->AHB1ENR |= (1U << 0);

	// Configure TIM2
	TIM2->PSC = 16 - 1; // Presclaer for TIM2
	TIM2->ARR = 1000 - 1; // ARR for TIM2 (F = 1KHz)
	TIM2->CR1 |= (1U << 0); // Enable counter for TIM2
	TIM2->CCMR1 |= (6U << 4); // Enable PWM mode 1 on channel 1
	TIM2->CCER |= (1U << 0); // Enable Output compare

	// Configure PA5
	GPIOA->MODER |= (2U << 10); // Alternate function for PA5
	GPIOA->AFR[0] |= (1U << 20); // Set up AF1 for PA5
}

int main() {

	while(1) {

	}

	return 0;
}
