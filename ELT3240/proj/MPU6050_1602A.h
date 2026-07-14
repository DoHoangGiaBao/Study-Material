#ifndef MPU6050_1602A_H
#define MPU6050_1602A_H

#include <stm32f4xx.h>
#include <stdio.h>

#define MPU6050_ADDR        0xD0    /* Device address */
#define LCD_ADDR           0x27    /* I2C address for 1602A LCD (with PCF8574) */
#define REG_PWR_MGMT_1      0x6B    /* Power management register */
#define REG_ACCEL_XOUT_H    0x3B    /* First data register for burst read */
#define DMA_BUFFER_SIZE     14      /* 14 bytes for accel+gyro data */

void I2C1_Init(void);
void DMA1_Init(void);
void MPU6050_Init(void);
void LCD_Init(void);

void LCD_Write_Cmd(uint8_t cmd);
void MPU6050_Read_DMA(void);
void LCD_Send_String_DMA(const char* str);

#endif