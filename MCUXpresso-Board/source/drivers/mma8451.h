/**
 * @file mma8451.h
 * @brief MMA8451Q Accelerometer Driver (Using NXP SDK I2C Transfer)
 */

#ifndef __MMA8451_H__
#define __MMA8451_H__

#include <stdint.h>

#define MMA8451_I2C_ADDR 0x1DU
#define MMA8451_WHOAMI_VAL 0x1AU
#define ACCEL_STATUS 0x00U
#define ACCEL_XYZ_DATA_CFG 0x0EU
#define ACCEL_CTRL_REG1 0x2AU
#define ACCEL_WHOAMI_REG 0x0DU

#define ACCEL_READ_LEN 7U

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t s;
} SRAWDATA;

int ACCEL_Init(void);

int ACCEL_getAccelDat(SRAWDATA *accelDat);

#endif /* __MMA8451_H__ */
