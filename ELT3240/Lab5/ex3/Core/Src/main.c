#include <stm32f4xx_hal.h>

void EXTI1_IRQHandler() {
	if (EXTI->PR & (1UL << 1)) {
		for (int i = 0; i < 5; i++) {
			GPIOA->BSRR = (1UL << 5);
			for (int j = 0; j < 1000000; j++);
			GPIOA->BSRR = (1UL << 21);
			for (int j = 0; j < 1000000; j++);
		}

		EXTI->PR |= (1UL << 1);
	}
}

void EXTI2_IRQHandler() {
	if (EXTI->PR & (1UL << 2)) {
		for (int i = 0; i < 10; i++) {
			GPIOA->BSRR = (1UL << 6);
			for (int j = 0; j < 200000; j++);
			GPIOA->BSRR = (1UL << 22);
			for (int j = 0; j < 200000; j++);
		}

		EXTI->PR |= (1UL << 2);
	}
}

void EXTI3_IRQHandler() {
	if (EXTI->PR & (1UL << 3)) {
		GPIOA->BSRR = (1UL << 7);
		for (int i = 0; i < 3000000; i++);
		GPIOA->BSRR = (1UL << 23);

		EXTI->PR |= (1UL << 3);
	}
}

void init_inputs() {
	// Enable Port C
	RCC->AHB1ENR |= (1UL << 2);

	// Configure PC1, PC2, PC3 as inputs
	GPIOC->MODER &= ~((3UL << 2) | (3UL << 4) | (3UL << 6));

	// Configure PC1, PC2, PC3 as pull-up
	GPIOC->PUPDR &= ~((3UL << 2) | (3UL << 4) | (3UL << 6));
	GPIOC->PUPDR |= (1UL << 2) | (1UL << 4) | (1UL << 6);
}

void init_outputs() {
	// Enable Port A
	RCC->AHB1ENR |= (1UL << 0);

	// Configure PA5, PA6, PA7 as outputs
	GPIOA->MODER &= ~((3UL << 10) | (3UL << 12) | (3UL << 14));
	GPIOA->MODER |= (1UL << 10) | (1UL << 12) | (1UL << 14);

	// Configure PA5, PA6, PA7 as no pull-up/pull-down
	GPIOA->PUPDR &= ~((3UL << 10) | (3UL << 12) | (3UL << 14));
}

void init_interrupts() {
	// Enable SYSCFG
	RCC->APB2ENR |= (1UL << 14);

	// Mask interrupt for PC1, PC2, PC3
	EXTI->IMR &= ~((1UL << 1) | (1UL << 2) | (1UL << 3));
	EXTI->IMR |= (1UL << 1) | (1UL << 2) | (1UL << 3);

	// Configure PC1, PC2, PC3 for falling edge
	EXTI->FTSR &= ~((1UL << 1) | (1UL << 2) | (1UL << 3));
	EXTI->FTSR |= (1UL << 1) | (1UL << 2) | (1UL << 3);

	// Map PC1, PC2, PC3
	SYSCFG->EXTICR[0] &= ~((15UL << 4) | (15UL << 8) | (15UL << 12));
	SYSCFG->EXTICR[0] |= (2UL << 4) | (2UL << 8) | (2UL << 12);

	// Enable interrupt and priority
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);

	NVIC_SetPriority(EXTI1_IRQn, 3);
	NVIC_SetPriority(EXTI2_IRQn, 2);
	NVIC_SetPriority(EXTI3_IRQn, 1);
}


int main() {
	init_inputs();
	init_outputs();
	init_interrupts();

	while (1) {
		GPIOA->BSRR = (1UL << 21) | (1UL << 22) | (1UL << 23);
	}
	return 0;
}
