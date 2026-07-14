#include "MPU6050_1602A.h"

uint8_t dma_rx_buffer[DMA_BUFFER_SIZE]; /* Buffer for MPU6050 data */
uint8_t lcd_dma_buffer[64]; /* Buffer for LCD data */
volatile uint8_t i2c_busy_flag = 0; /* Flag to indicate I2C bus is busy (set in LCD DMA function) */

void DMA1_Stream0_IRQHandler(void) {
    /* Check for Transfer Complete Interrupt */
    if (DMA1->LISR & DMA_LISR_TCIF0) {
        DMA1->LIFCR = DMA_LIFCR_CTCIF0; /* Clear flag */

        /* Process Data (Register-level conversion) */
        int16_t ax = (dma_rx_buffer[0] << 8) | dma_rx_buffer[1];
        
        /* Format string */
        char msg[16]; /* Buffer for formatted string */
        sprintf(msg, "Ax: %d   ", ax); /* Format with padding to clear old data */

        /* Trigger LCD update via DMA */
        LCD_Send_String_DMA(msg);
        i2c_busy_flag = 0; /* Clear busy flag after LCD update is triggered */
    }
}

void I2C1_Init(void) {
    /* Enable GPIOB and I2C1 clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* Configure PB8 (SCL) and PB9 (SDA) for I2C1 (AF4) */
    GPIOB->MODER   &= ~((3 << (8 * 2)) | (3 << (9 * 2)));  /* Clear mode */
    GPIOB->MODER   |=  ((2 << (8 * 2)) | (2 << (9 * 2)));  /* Alternate function */
    GPIOB->OTYPER  |=  ((1 << 8) | (1 << 9));             /* Open-drain */
    GPIOB->OSPEEDR |=  ((3 << (8 * 2)) | (3 << (9 * 2))); /* High speed */
    GPIOB->PUPDR   &= ~((3 << (8 * 2)) | (3 << (9 * 2))); /* No pull-up/down */
    GPIOB->AFR[1] |=  ((4 << ((8 - 8) * 4)) | (4 << ((9 - 8) * 4))); /* AF4 */

    /* Reset I2C1 */
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    /* Configure I2C1: Standard mode, 100 kHz, APB1 at 16 MHz */
    I2C1->CR2 = 16;           /* APB1 clock frequency in MHz */
    I2C1->CCR = 80;           /* Clock control for 100 kHz */
    I2C1->TRISE = 17;         /* Maximum rise time */

    /* Enable I2C1 peripheral */
    I2C1->CR1 |= I2C_CR1_PE;
}

void DMA1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; /* Enable DMA1 clock */

    /* Stream 0: I2C1_RX (Channel 1) */
    DMA1_Stream0->CR = 0; /* Clear control register */
    while (DMA1_Stream0->CR & DMA_SxCR_EN); /* Ensure stream is disabled */
    DMA1_Stream0->PAR = (uint32_t)&I2C1->DR; /* Peripheral address */
    DMA1_Stream0->M0AR = (uint32_t)dma_rx_buffer; /* Memory address */
    DMA1_Stream0->NDTR = DMA_BUFFER_SIZE; /* Number of data items to transfer */
    DMA1_Stream0->CR |= (1 << 25) | DMA_SxCR_MINC | DMA_SxCR_TCIE; /* Memory increment, transfer complete interrupt */

    /* Stream 6: I2C1_TX (Channel 1) */
    DMA1_Stream6->CR = 0; /* Clear control register */
    while (DMA1_Stream6->CR & DMA_SxCR_EN); /* Ensure stream is disabled */
    DMA1_Stream6->PAR = (uint32_t)&I2C1->DR; /* Peripheral address */
    DMA1_Stream6->M0AR = (uint32_t)lcd_dma_buffer; /* Memory address */
    DMA1_Stream6->CR |= (1 << 25) | DMA_SxCR_DIR_0 | DMA_SxCR_MINC; /* Memory increment, memory-to-peripheral */

    NVIC_EnableIRQ(DMA1_Stream0_IRQn); /* Enable DMA1 Stream 0 interrupt in NVIC */
}

void MPU6050_Init(void) {
    while (I2C1->SR2 & I2C_SR2_BUSY); /* Wait until I2C bus is free */
    I2C1->CR1 |= I2C_CR1_START; /* Generate start condition */
    while (!(I2C1->SR1 & I2C_SR1_SB)); /* Wait for start bit */
    I2C1->DR = MPU6050_ADDR; /* Send device address with write bit */
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); /* Wait for address to be sent */
    (void)I2C1->SR2; /* Clear address flag */
    
    I2C1->DR = REG_PWR_MGMT_1; /* Register address */
    while (!(I2C1->SR1 & I2C_SR1_TXE)); /* Wait for data register empty */
    I2C1->DR = 0x00; /* Set to zero to wake up MPU6050 */
    while (!(I2C1->SR1 & I2C_SR1_BTF)); /* Wait for byte transfer finished */
    I2C1->CR1 |= I2C_CR1_STOP; /* Generate stop condition */
}

void LCD_Write_Cmd(uint8_t cmd) {
    uint8_t backlight = 0x08; /* Backlight ON bit */
    uint8_t data_u = (cmd & 0xf0); /* High nibble with backlight */
    uint8_t data_l = ((cmd << 4) & 0xf0); /* Low nibble with backlight */
    uint8_t pkg[4] = { data_u|backlight|0x04, data_u|backlight, data_l|backlight|0x04, data_l|backlight }; /* EN=1 then EN=0 for each nibble */
 
    for(int i=0; i<4; i++) {
        while (I2C1->SR2 & I2C_SR2_BUSY); /* Wait until I2C bus is free */
        I2C1->CR1 |= I2C_CR1_START; /* Generate start condition */
        while (!(I2C1->SR1 & I2C_SR1_SB)); /* Wait for start bit */
        I2C1->DR = LCD_ADDR; /* Send LCD I2C address with write bit */
        while (!(I2C1->SR1 & I2C_SR1_ADDR)); /* Wait for address to be sent */
        (void)I2C1->SR2; /* Clear address flag */
        I2C1->DR = pkg[i]; /* Send command nibble */
        while (!(I2C1->SR1 & I2C_SR1_BTF)); /* Wait for byte transfer finished */
        I2C1->CR1 |= I2C_CR1_STOP; /* Generate stop condition */
    }
}

void LCD_Init(void) {
    for(int i=0; i<500000; i++); /* Delay >40ms after power-up */
    LCD_Write_Cmd(0x30); LCD_Write_Cmd(0x30); LCD_Write_Cmd(0x30); /* Initialization sequence */
    LCD_Write_Cmd(0x20); /* Set 4-bit mode */
    LCD_Write_Cmd(0x28); /* Function set: 4-bit, 2 lines, 5x8 dots */
    LCD_Write_Cmd(0x0C); /* Display ON, Cursor OFF */
    LCD_Write_Cmd(0x01); /* Clear display */
    for(int i=0; i<200000; i++); /* Delay for clear operation */
}

void MPU6050_Read_DMA(void) {
    while (I2C1->SR2 & I2C_SR2_BUSY); /* Wait until I2C bus is free */
    
    I2C1->CR1 |= I2C_CR1_START; /* Generate start condition */
    while (!(I2C1->SR1 & I2C_SR1_SB)); /* Wait for start bit */
    I2C1->DR = MPU6050_ADDR; /* Send device address with write bit */
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); /* Wait for address to be sent */
    (void)I2C1->SR1; (void)I2C1->SR2; /* Clear address flag */
    
    I2C1->DR = REG_ACCEL_XOUT_H; /* Register address for burst read */
    while (!(I2C1->SR1 & I2C_SR1_TXE)); /* Wait for data register empty */

    I2C1->CR1 |= I2C_CR1_START; /* Generate repeated start condition */
    while (!(I2C1->SR1 & I2C_SR1_SB)); /* Wait for start bit */
    I2C1->DR = MPU6050_ADDR | 1; /* Send device address with read bit */
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); /* Wait for address to be sent */
    (void)I2C1->SR1; (void)I2C1->SR2; /* Clear address flag */

    /* Configure I2C for DMA reception */
    I2C1->CR2 |= I2C_CR2_LAST; /* Set LAST bit for auto-stop after DMA transfer */
    I2C1->CR2 |= I2C_CR2_DMAEN; /* Enable I2C DMA */

    DMA1_Stream0->CR &= ~DMA_SxCR_EN; /* Ensure DMA stream is disabled before configuring */
    while(DMA1_Stream0->CR & DMA_SxCR_EN); /* Wait until DMA stream is disabled */
    DMA1_Stream0->NDTR = 14; /* Number of bytes to read */
    DMA1->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0; /* Clear any existing flags */
    DMA1_Stream0->CR |= DMA_SxCR_EN; /* Enable DMA stream to start reception */
}

void LCD_Send_String_DMA(char *str) {
    int i = 0, j = 0; /* Convert string to LCD data packets (4 bytes per character) */
    uint8_t backlight = 0x08; /* Backlight ON bit */
    uint8_t rs = 0x01; /* Register select bit for data (0 for command, 1 for data) */

    while (str[i] && j < 60) {
        uint8_t high_nibble = (str[i] & 0xF0) | backlight | rs; /* High nibble with backlight and RS */
        uint8_t low_nibble  = ((str[i] << 4) & 0xF0) | backlight | rs; /* Low nibble with backlight and RS */
        lcd_dma_buffer[j++] = high_nibble | 0x04; /* EN=1 for high nibble */
        lcd_dma_buffer[j++] = high_nibble; /* EN=0 for high nibble */
        lcd_dma_buffer[j++] = low_nibble  | 0x04; /* EN=1 for low nibble */
        lcd_dma_buffer[j++] = low_nibble; /* EN=0 for low nibble */
        i++;
    }

    /* Ensure bus is free and clear LAST bit for multi-byte DMA */
    I2C1->CR2 &= ~I2C_CR2_LAST; /* Clear LAST bit for multi-byte transfer */
    while (I2C1->CR1 & I2C_CR1_STOP); /* Wait until any previous STOP condition has been sent */
    while (I2C1->SR2 & I2C_SR2_BUSY); /* Wait until I2C bus is free */

    /* Configure DMA for LCD transmission */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN; /* Ensure DMA stream is disabled before configuring */
    while(DMA1_Stream6->CR & DMA_SxCR_EN); /* Wait until DMA stream is disabled */
    
    DMA1->HIFCR = DMA_HIFCR_CTCIF6; /* Clear any existing transfer complete flag */
    DMA1_Stream6->M0AR = (uint32_t)lcd_dma_buffer; /* Memory address */
    DMA1_Stream6->NDTR = j; /* Number of bytes to send */
    
    /* Start I2C transmission */
    I2C1->CR1 |= I2C_CR1_START; /* Generate start condition */
    while (!(I2C1->SR1 & I2C_SR1_SB)); /* Wait for start bit */
    I2C1->DR = 0x27 << 1; /* Send LCD I2C address with write bit */
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); /* Wait for address to be sent */
    (void)I2C1->SR1; (void)I2C1->SR2; /* Clear address flag */

    /* Enable DMA for I2C transmission */
    I2C1->CR2 |= I2C_CR2_DMAEN; /* Enable I2C DMA */
    DMA1_Stream6->CR |= DMA_SxCR_EN; /* Enable DMA stream to start transmission */

    /* Wait for DMA transfer to complete and then generate STOP condition */
    while (!(DMA1->HIFR & DMA_HIFR_CTCIF6)); /* Wait for transfer complete flag */
    while (!(I2C1->SR1 & I2C_SR1_BTF)); /* Wait for byte transfer finished */
    I2C1->CR1 |= I2C_CR1_STOP; /* Generate stop condition */
}