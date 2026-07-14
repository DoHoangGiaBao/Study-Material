#ifndef MPU6050_H
#define MPU6050_H

#include <stm32f4xx.h>

/*
 * Hardware connections (NUCLEO-F401RE):
 *   PB8 = SCL  (I2C1, AF4)
 *   PB9 = SDA  (I2C1, AF4)
 *   DMA1 Stream 0, Channel 1 → I2C1_RX 
 *
 * MPU6050 AD0 pin = LOW      →  7-bit addr = 0x68
 *                            →  write byte = 0xD0
 *                            →  read  byte = 0xD1
 */

/* MPU6050 register addresses */
#define MPU6050_ADDR        0xD0    /* Device address               */
#define REG_SMPLRT_DIV      0x19    /* Sample rate divider          */
#define REG_CONFIG          0x1A    /* DLPF configuration           */
#define REG_GYRO_CONFIG     0x1B    /* Gyroscope full-scale range   */
#define REG_ACCEL_CONFIG    0x1C    /* Accelerometer full-scale     */
#define REG_ACCEL_XOUT_H    0x3B    /* First data register (burst)  */
#define REG_GYRO_XOUT_H     0x43    /* Gyro X high byte             */
#define REG_PWR_MGMT_1      0x6B    /* Power management             */
#define REG_WHO_AM_I        0x75    /* Identity register (= 0x68)   */


#define DMA_BUFFER_SIZE     14

void i2c1_init(void);
void dma1_stream0_init(void);
void mpu6050_init(void);
void mpu6050_read_all_dma(void);
void mpu6050_parse_data(void);
#endif