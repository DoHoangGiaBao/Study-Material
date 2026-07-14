#include "mpu6050.h"

/* Global variables */

uint8_t          dma_rx_buffer[DMA_BUFFER_SIZE];
int16_t          Accel_X_Raw, Accel_Y_Raw, Accel_Z_Raw;
int16_t          Gyro_X_Raw,  Gyro_Y_Raw,  Gyro_Z_Raw;
volatile uint8_t dma_transfer_complete = 0;

/* Static helper: write one byte to an MPU6050 register (polling) */
static void MPU6050_Write_Register(uint8_t reg, uint8_t data)
{
    /* Wait until bus is free */
    while (I2C1->SR2 & I2C_SR2_BUSY);

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    /* Slave address + WRITE (LSB = 0) */
    I2C1->DR = MPU6050_ADDR;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1;   /* Clear ADDR flag: read SR1 ... */
    (void)I2C1->SR2;   /*                  ... then SR2  */

    /* Register address */
    I2C1->DR = reg;
    while (!(I2C1->SR1 & I2C_SR1_TXE));

    /* Data byte */
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_BTF)); /* Both DR and shift-reg empty */

    /* STOP */
    I2C1->CR1 |= I2C_CR1_STOP;
}

/* I2C1 peripheral init
 *
 * Pins   : PB8 = SCL, PB9 = SDA  (AF4, open-drain, pull-up)
 * APB1   : 16 MHz (HSI default clock)
 * Mode   : standard, 100 kHz
 */
static void I2C1_Init(void)
{
    /* Clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* GPIO */

    /* PB8, PB9 → Alternate Function mode (MODER = 10) */
    GPIOB->MODER &= ~((3UL << 16) | (3UL << 18));  /* Clear bits */
    GPIOB->MODER |=  ((2UL << 16) | (2UL << 18));  /* Set AF mode */

    /* Open-drain output type */
    GPIOB->OTYPER |= (1UL << 8) | (1UL << 9);

    /* High speed */
    GPIOB->OSPEEDR |= (3UL << 16) | (3UL << 18);

    /* Internal pull-up (external pull-ups on the board also fine) */
    GPIOB->PUPDR &= ~((3UL << 16) | (3UL << 18));
    GPIOB->PUPDR |=  ((1UL << 16) | (1UL << 18));  /* 01 = pull-up */

    /* AF4 (I2C1) for PB8 (AFRH bits [3:0]) and PB9 (AFRH bits [7:4]) */
    GPIOB->AFR[1] &= ~(0xFFUL);
    GPIOB->AFR[1] |=  (4UL << 0) | (4UL << 4);

    /* I2C1 */

    /* Software reset clears any stuck state */
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    /* Peripheral clock in MHz (must match APB1) */
    I2C1->CR2 = 16;

    /* Standard-mode 100 kHz:
     *   CCR = PCLK1 / (2 * f_SCL) = 16 000 000 / 200 000 = 80 */
    I2C1->CCR = 80;

    /* Max rise time for standard mode = 1000 ns:
     *   TRISE = floor(1000 ns / 62.5 ns) + 1 = 17 */
    I2C1->TRISE = 17;

    /* Enable peripheral */
    I2C1->CR1 |= I2C_CR1_PE;
}

/* DMA1 Stream 0 init  →  I2C1_RX  (Channel 1)
 *
 * Only the receive direction is DMA-driven.
 * The short transmit phase (register pointer) stays polling.
 */
static void DMA1_Stream0_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    /* Disable stream; wait for hardware to confirm */
    DMA1_Stream0->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream0->CR & DMA_SxCR_EN);

    /* Clear all interrupt flags for Stream 0
     * LIFCR bits for Stream 0: TCIF0[5] HTIF0[4] TEIF0[3] DMEIF0[2] FEIF0[0]
     * = 0b00111101 = 0x3D  (bit 1 is reserved) */
    DMA1->LIFCR = 0x3DUL;

    /* Configure stream:
     *   Channel 1        → I2C1_RX
     *   Direction        → Peripheral to Memory (DIR = 00, default)
     *   Memory increment → enabled
     *   Data size        → byte / byte (PSIZE=00, MSIZE=00, default)
     *   TC interrupt     → enabled                                    
     */
    DMA1_Stream0->CR = (1UL << DMA_SxCR_CHSEL_Pos) |   /* Channel 1  */
                        DMA_SxCR_MINC              |   /* Mem incr   */
                        DMA_SxCR_TCIE;                 /* TC irq     */

    /* Peripheral source: I2C1 data register */
    DMA1_Stream0->PAR  = (uint32_t)&I2C1->DR;

    /* Memory destination: our receive buffer */
    DMA1_Stream0->M0AR = (uint32_t)dma_rx_buffer;

    /* Transfer count (reset before each read in MPU6050_Read_All_DMA) */
    DMA1_Stream0->NDTR = DMA_BUFFER_SIZE;

    /* Enable DMA1 Stream 0 interrupt in NVIC */
    NVIC_SetPriority(DMA1_Stream0_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

/* Public: initialise hardware and wake up MPU6050 */
void MPU6050_Init(void)
{
    I2C1_Init();
    DMA1_Stream0_Init();

    /* Exit sleep mode */
    MPU6050_Write_Register(REG_PWR_MGMT_1,   0x01);

    /* Sample rate = gyro rate / (1 + divider)
     * divider = 0  →  1 kHz with DLPF enabled
     */
    MPU6050_Write_Register(REG_SMPLRT_DIV,   0x00);

    /* Gyroscope full-scale: FS_SEL = 0  →  ±250 °/s
     * LSB sensitivity = 131 LSB per °/s
     */
    MPU6050_Write_Register(REG_GYRO_CONFIG,  0x00);

    /* Accelerometer full-scale: AFS_SEL = 0  →  ±2 g
     * LSB sensitivity = 16384 LSB per g
     */
    MPU6050_Write_Register(REG_ACCEL_CONFIG, 0x00);
}

/* Public: start a non-blocking DMA read of all sensor registers
 *
 * Phase 1 – CPU (polling):
 *   Send a repeated-start register-pointer write to 0x3B so the
 *   MPU6050 will stream 14 bytes starting at ACCEL_XOUT_H.
 *
 * Phase 2 – DMA:
 *   DMA receives the 14 bytes autonomously. The LAST bit in
 *   I2C_CR2 makes the hardware send NACK after the final byte.
 *   The CPU returns immediately and is free to do other work.
 *   DMA1_Stream0_IRQHandler() generates STOP and parses the data.
 */
void MPU6050_Read_All_DMA(void)
{
    dma_transfer_complete = 0;

    /* WRITE PHASE: set the MPU6050 register pointer */

    while (I2C1->SR2 & I2C_SR2_BUSY);    /* Wait for bus free */

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    /* Slave address + WRITE */
    I2C1->DR = MPU6050_ADDR;             /* 0xD0 */
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1;
    (void)I2C1->SR2;                     /* Clear ADDR */

    /* Register address to start reading from */
    I2C1->DR = REG_ACCEL_XOUT_H;        /* 0x3B */
    while (!(I2C1->SR1 & I2C_SR1_BTF)); /* Wait: byte fully clocked out */

    /* Prepare DMA before issuing RESTART */

    /* Reset stream (NDTR auto-decrements, must reload before each read) */
    DMA1_Stream0->CR  &= ~DMA_SxCR_EN;
    while (DMA1_Stream0->CR & DMA_SxCR_EN);
    DMA1->LIFCR        = 0x3DUL;
    DMA1_Stream0->NDTR = DMA_BUFFER_SIZE;
    DMA1_Stream0->M0AR = (uint32_t)dma_rx_buffer;
    DMA1_Stream0->CR  |= DMA_SxCR_EN;   /* Enable stream */

    /* DMAEN: I2C will issue a DMA request after each received byte.
     * LAST:  after the (N-1)th byte, hardware clears ACK so the
     *        Nth byte is NACKed – tells slave to stop sending.   
     */
    I2C1->CR2 |= I2C_CR2_DMAEN | I2C_CR2_LAST;

    /* ACK must be set before clearing ADDR on the read address  */
    I2C1->CR1 |= I2C_CR1_ACK;

    /* READ PHASE: repeated START + slave address + READ */

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    /* Slave address + READ (LSB = 1) */
    I2C1->DR = MPU6050_ADDR | 0x01;     /* 0xD1 */
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1;
    (void)I2C1->SR2;   /* Clear ADDR – DMA immediately starts receiving */

    /* CPU returns; DMA ISR handles the rest */
}

/* Public: convert the raw byte buffer into signed 16-bit values
 * Called automatically from the DMA ISR; can also be called
 * manually if you manage transfers yourself.
 */
void MPU6050_Parse_Data(void)
{
    Accel_X_Raw = (int16_t)((dma_rx_buffer[0]  << 8) | dma_rx_buffer[1]);
    Accel_Y_Raw = (int16_t)((dma_rx_buffer[2]  << 8) | dma_rx_buffer[3]);
    Accel_Z_Raw = (int16_t)((dma_rx_buffer[4]  << 8) | dma_rx_buffer[5]);

    Gyro_X_Raw  = (int16_t)((dma_rx_buffer[8]  << 8) | dma_rx_buffer[9]);
    Gyro_Y_Raw  = (int16_t)((dma_rx_buffer[10] << 8) | dma_rx_buffer[11]);
    Gyro_Z_Raw  = (int16_t)((dma_rx_buffer[12] << 8) | dma_rx_buffer[13]);
}

/* DMA1 Stream 0 interrupt handler
 *
 * Fires when all 14 bytes have been received.
 * Generates the I2C STOP, parses the buffer, and sets the flag.
 */
void DMA1_Stream0_IRQHandler(void)
{
    if (DMA1->LISR & DMA_LISR_TCIF0)
    {
        /* Clear all Stream 0 interrupt flags */
        DMA1->LIFCR = 0x3DUL;

        /* Disable DMA stream */
        DMA1_Stream0->CR &= ~DMA_SxCR_EN;

        /* Stop I2C from issuing further DMA requests */
        I2C1->CR2 &= ~(I2C_CR2_DMAEN | I2C_CR2_LAST);

        /* Generate STOP condition */
        I2C1->CR1 |= I2C_CR1_STOP;

        /* Clear ACK (default state; set again before next multi-byte read) */
        I2C1->CR1 &= ~I2C_CR1_ACK;

        /* Parse raw bytes into signed 16-bit values */
        MPU6050_Parse_Data();

        /* Signal main loop that fresh data is available */
        dma_transfer_complete = 1;
    }
}