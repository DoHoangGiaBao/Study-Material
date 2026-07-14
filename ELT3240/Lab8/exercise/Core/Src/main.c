#include <stm32f4xx.h>

void USART2_Init(void) {
	// Enable clocks
	RCC->AHB1ENR |= (1U << 0);				// Enable GPIOA clock
	RCC->APB1ENR |= (1U << 17);				// Enable USART2 clock

	// Configure PA2
	GPIOA->MODER &= ~(3U << 4);				// Set PA2 as Alternate function
	GPIOA->MODER |= (2U << 4);
	GPIOA->AFR[0] &= ~(15U << 8); 			// Set AF7 for PA2
	GPIOA->AFR[0] |= (7U << 8);

	// Configure USART2
	USART2->BRR &= ~(0xFFFF << 0);			// Set baud rate to 9600
	USART2->BRR |= (104 << 4) | (3 << 0);
	USART2->CR1 &= ~(1U << 15);				// Set OVER8 sampling by 16
	USART2->CR1 |= (1U << 3);				// Enable transmitter
	USART2->CR1 |= (1U << 13);				// Enable USART
}

void USART2_SendChar(char c) {
	while(!(USART2->SR & (1 << 7)));		// Wait until TXE (Transmit Data Register Empty)
	USART2->DR = c;
}

void USART2_SendString(const char *str) {
	while(*str) {
		USART2_SendChar(*str++); // Send each character in the string
	}
}

int main() {
	USART2_Init();
	while(1) {
		USART2_SendString("Hello World\r\n");
		for (volatile int i = 0; i < 1000000; i++);
	}
	return 0;
}
