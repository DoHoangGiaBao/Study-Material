#include <stm32f4xx.h>

void timers_config() {
	// Enable clock for TIM2 and TIM5
	RCC->AHB1ENR |= (1U << 0) | (1U << 3);

	// Enable clock for PA0 and PA1
	RCC->AHB1ENR |= (1U << 0);

	// TIM2 configuration (trigger pulse)
	TIM2->PSC = 16 - 1;
	TIM2->ARR = 60000 - 1;
	TIM2->CCR1 = 10;
	TIM2->CCMR1 |=
}

int main() {

	return 0;
}
