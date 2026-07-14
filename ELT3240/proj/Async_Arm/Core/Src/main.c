#include "MPU6050_DMA.h"

int main() {
    I2C1_Init();
    DMA1_Init();
    MPU6050_Init();

    MPU6050_Start_DMA_Read();

    while(1) {
        if (!obstacle_detected) {
            // Access pitch and roll here for PWM control
            pitch;
            roll;
        } else {
            // Handle IR Sensor Interrupt
        }
    }

    return 0;
}
