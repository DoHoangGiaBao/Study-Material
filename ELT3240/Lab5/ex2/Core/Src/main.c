#include <stm32f4xx_hal.h>

void EXTI15_10_IRQHandler() {
	if (EXTI->PR & (1UL << 13)) {
		GPIOA->BSRR = (1UL << 5);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 21);
		GPIOA->BSRR = (1UL << 6);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 22);
		GPIOA->BSRR = (1UL << 7);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 23);
		GPIOA->BSRR = (1UL << 5);

		EXTI->PR |= (1UL << 13);
	}
}

void EXTI9_5_IRQHandler() {
	if (EXTI->PR & (1UL << 8)) {
		GPIOA->BSRR = (1UL << 21) | (1UL << 7);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 23) | (1UL << 6);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 22) | (1UL << 5);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 21) | (1UL << 7);
		for (int i = 0; i < 1000000; i++);
		GPIOA->BSRR = (1UL << 23);

		EXTI->PR |= (1UL << 8);
	}
}

void init_inputs() {
	// Enable Port C
	RCC->AHB1ENR |= (1UL << 2);

	// Set PC8 and PC13 as inputs
	GPIOC->MODER &= ~((3UL << 16) | (3UL << 26));

	// Set PC8 and PC13 as pull-up
	GPIOC->PUPDR &= ~((3UL << 16) | (3UL << 26));
	GPIOC->PUPDR |= (1UL << 16) | (1UL << 26);
}

void init_outputs() {
	// Enable Port A
	RCC->AHB1ENR |= (1ULL << 0);

	// Configure PA5, PA6, PA7 as outputs
	GPIOA->MODER &= ~((3UL << 10) | (3UL << 12) | (3UL << 14));
	GPIOA->MODER |= (1UL << 10) | (1UL << 12) | (1UL << 14);

	// Configure PA5, PA6, PA7 as no pull-up or pull-down
	GPIOA->PUPDR &= ~((3UL << 10) | (3UL << 12) | (3UL << 14));
}

void init_interrupts() {
	// Enable SYSCFG for interrupts
	RCC->APB2ENR |= (1UL << 14);

	// Mask PC8 and PC13 as interrupt inputs
	EXTI->IMR &= ~((1UL << 8) | (1UL << 13));
	EXTI->IMR |= (1UL << 8) | (1UL << 13);

	// Enable falling edge for PC8 and PC13
	EXTI->FTSR &= ~((1UL << 8) | (1UL << 13));
	EXTI->FTSR |= (1UL << 8) | (1UL << 13);

	// Configure external interrupts
	SYSCFG->EXTICR[2] &= ~(15UL << 0);
	SYSCFG->EXTICR[2] |= (2UL << 0);
	SYSCFG->EXTICR[3] &= ~(15UL << 4);
	SYSCFG->EXTICR[3] |= (2UL << 4);

	// Enable interrupts
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	// Set priority
	NVIC_SetPriority(EXTI9_5_IRQn, 0);
	NVIC_SetPriority(EXTI15_10_IRQn, 1);
}

int main() {
	init_inputs();
	init_outputs();
	init_interrupts();

	while(1) {
		GPIOA->BSRR = (1UL << 5);
	}
}
