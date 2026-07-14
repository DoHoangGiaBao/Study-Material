#ifndef MPU6050_H
#define MPU6050_H

#include <stm32f4xx.h>
#include <stdint.h>

/*
 * Hardware connections (NUCLEO-F401RE):
 *   PB8 = SCL  (I2C1, AF4)
 *   PB9 = SDA  (I2C1, AF4)
 *   DMA1 Stream 0, Channel 1 → I2C1_RX
 *
 * MPU6050 AD0 pin = LOW    →  7-bit addr = 0x68
 *                          →  write byte = 0xD0
 *                          →  read  byte = 0xD1
 */

/* I2C device address */
#define MPU6050_ADDR        0xD0    /* Device address */

/* MPU6050 register addresses */
#define REG_SMPLRT_DIV      0x19    /* Sample rate divider          */
#define REG_CONFIG          0x1A    /* DLPF configuration           */
#define REG_GYRO_CONFIG     0x1B    /* Gyroscope full-scale range   */
#define REG_ACCEL_CONFIG    0x1C    /* Accelerometer full-scale     */
#define REG_ACCEL_XOUT_H    0x3B    /* First data register (burst)  */
#define REG_GYRO_XOUT_H     0x43    /* Gyro X high byte             */
#define REG_PWR_MGMT_1      0x6B    /* Power management             */
#define REG_WHO_AM_I        0x75    /* Identity register (= 0x68)   */

/*
 * A burst read from REG_ACCEL_XOUT_H captures 14 consecutive bytes:
 *   [0-1]   Accel X high/low
 *   [2-3]   Accel Y high/low
 *   [4-5]   Accel Z high/low
 *   [6-7]   Temperature high/low  (parsed separately if needed)
 *   [8-9]   Gyro X high/low
 *   [10-11] Gyro Y high/low
 *   [12-13] Gyro Z high/low
 */
#define DMA_BUFFER_SIZE     14

/* Exported data */
extern uint8_t          dma_rx_buffer[DMA_BUFFER_SIZE];
extern int16_t          Accel_X_Raw, Accel_Y_Raw, Accel_Z_Raw;
extern int16_t          Gyro_X_Raw,  Gyro_Y_Raw,  Gyro_Z_Raw;
extern volatile uint8_t dma_transfer_complete;   /* Set by DMA ISR */

/* Public API */
void MPU6050_Init(void);          /* Call once at startup                */
void MPU6050_Read_All_DMA(void);  /* Non-blocking: starts DMA read       */
void MPU6050_Parse_Data(void);    /* Called from ISR; also callable manually */

#endif /* MPU6050_H */