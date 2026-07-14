#include <stm32f4xx_hal.h>

int flag = 0;
volatile uint32_t ms_ticks = 0;

void TIM2_IRQHandler() {
	if (TIM2->SR & (1U << 0)) {
		ms_ticks++;

		TIM2->SR &= ~(1U << 0);
	}
}

void outputs_init() {
	// Enable clock for port A
	RCC->AHB1ENR |= (1U << 0);

	// Set PA5, PA6, PA7 as outputs
	GPIOA->MODER &= ~((3U << 10) | (3U << 12) | (3U << 14));
	GPIOA->MODER |= (1U << 10) | (1U << 12) | (1U << 14);

	// Set PA5, PA6, PA7 as no pull-up/pull-down
	GPIOA->PUPDR &= ~((3U << 10) | (3U << 12) | (3U << 14));
}

void input_init() {
	// Enable clock for port C
	RCC->AHB1ENR |= (1U << 1);

	// Set PC13 as input
	GPIOC->MODER &= ~(3U << 26);

	// Set PC13 as pull-up
	GPIOC->PUPDR &= ~(3U << 26);
	GPIOC->PUPDR |= (1U << 26);
}

void tim2_init() {
	// Enable clock for TIM2
	RCC->APB1ENR |= (1U << 0);

	// Enable counter FOR TIM2
	TIM2->CR1 |= (1U << 0);

	// Enable TIM2 interrupt
	TIM2->DIER |= (1U << 0);

	// Set up Prescaler for TIM2
	TIM2->PSC = 16 - 1;

	// Set up Auto reload value for TIM2
	TIM2->ARR = 1000 - 1;

	// Enable TIM2 IRQ in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
}

int main() {
	outputs_init();
	input_init();
	tim2_init();
	uint32_t start_time = ms_ticks;
	int last_button_state = (GPIOC->IDR & (1U << 13)) ? 1 : 0;
	GPIOA->ODR |= (1U << 5);

	while (1) {
		uint32_t curr_time = ms_ticks;
		int curr_button_state = (GPIOC->IDR & (1U << 13)) ? 1 : 0;

		// Button handling
		if (curr_button_state == 0 && last_button_state == 1) {
			uint32_t debounce_start = ms_ticks;
			while((ms_ticks - debounce_start) < 50);

			if ((GPIOC->IDR & (1U << 13)) == 0) {
				if (GPIOA->ODR & (1U << 5)) {
					GPIOA->ODR &= ~(1U << 5);
					GPIOA->ODR |= (1U << 6);
					start_time = ms_ticks;
				} else {
					flag = 1;
				}
			}
		}

		last_button_state = curr_button_state;

		// Normal operation
		if (GPIOA->ODR & (1U << 5)) {
			if (curr_time - start_time >= 5000) {
				GPIOA->ODR &= ~(1U << 5);
				GPIOA->ODR |= (1U << 6);
				start_time = ms_ticks;
			}
		} else if (GPIOA->ODR & (1U << 6)) {
			if (curr_time - start_time >= 2000) {
				GPIOA->ODR &= ~(1U << 6);
				GPIOA->ODR |= (1U << 7);
				start_time = ms_ticks;
			}
		} else if (GPIOA->ODR & (1U << 7)) {
			if ((curr_time - start_time >= (5000 + (flag ? 3000 : 0)))) {
				flag = 0;
				GPIOA->ODR &= ~(1U << 7);
				GPIOA->ODR |= (1U << 5);
				start_time = ms_ticks;
			}
		}

	}

	return 0;
}
