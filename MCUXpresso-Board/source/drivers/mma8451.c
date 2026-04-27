/**
 * @file mma8451.c
 * @brief MMA8451Q Accelerometer Driver Implementation
 */

#include "mma8451.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"

#define BOARD_ACCEL_I2C_BASEADDR I2C0
#define I2C_BAUDRATE 100000U
#define ACCEL_I2C_CLK_FREQ CLOCK_GetFreq(I2C0_CLK_SRC)

#define I2C_RELEASE_SDA_PORT PORTE
#define I2C_RELEASE_SCL_PORT PORTE
#define I2C_RELEASE_SDA_GPIO GPIOE
#define I2C_RELEASE_SDA_PIN 25U
#define I2C_RELEASE_SCL_GPIO GPIOE
#define I2C_RELEASE_SCL_PIN 24U
#define I2C_RELEASE_BUS_COUNT 100U

i2c_master_handle_t g_m_handle;
volatile bool completionFlag = false;
volatile bool nakFlag = false;

/*-----------------------------------------------------------------------*/
/*  Internal Static Helper Functions                   */
/*-----------------------------------------------------------------------*/

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

static void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;

    CLOCK_EnableClock(kCLOCK_PortE);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SCL_PIN, &i2c_pin_config);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SDA_PIN, &i2c_pin_config);

    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();
        GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();
        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();
    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();
    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();
    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        completionFlag = true;
    }
    if ((status == kStatus_I2C_Nak) || (status == kStatus_I2C_Addr_Nak))
    {
        nakFlag = true;
    }
}

static bool I2C_WriteAccelReg(uint8_t device_addr, uint8_t reg_addr, uint8_t value)
{
    i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = device_addr;
    masterXfer.direction = kI2C_Write;
    masterXfer.subaddress = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data = &value;
    masterXfer.dataSize = 1;
    masterXfer.flags = kI2C_TransferDefaultFlag;

    I2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, &masterXfer);
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;
    if (completionFlag == true)
    {
        completionFlag = false;
        return true;
    }
    return false;
}

static bool I2C_ReadAccelRegs(uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = device_addr;
    masterXfer.direction = kI2C_Read;
    masterXfer.subaddress = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data = rxBuff;
    masterXfer.dataSize = rxSize;
    masterXfer.flags = kI2C_TransferDefaultFlag;

    I2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, &masterXfer);
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;
    if (completionFlag == true)
    {
        completionFlag = false;
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------*/
/* Public API                                             */
/*-----------------------------------------------------------------------*/

int ACCEL_Init(void)
{
    i2c_master_config_t masterConfig;
    uint8_t who_am_i_value = 0;

    BOARD_I2C_ReleaseBus();

    CLOCK_EnableClock(kCLOCK_PortE);
    PORTE->PCR[24] = PORT_PCR_MUX(5);
    PORTE->PCR[25] = PORT_PCR_MUX(5);

    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;
    I2C_MasterInit(BOARD_ACCEL_I2C_BASEADDR, &masterConfig, ACCEL_I2C_CLK_FREQ);
    I2C_MasterTransferCreateHandle(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, i2c_master_callback, NULL);
    if (I2C_ReadAccelRegs(MMA8451_I2C_ADDR, ACCEL_WHOAMI_REG, &who_am_i_value, 1) != true)
    {
        return 0;
    }
    if (who_am_i_value != MMA8451_WHOAMI_VAL)
    {
        return 0;
    }
    if (!I2C_WriteAccelReg(MMA8451_I2C_ADDR, ACCEL_CTRL_REG1, 0x00))
        return 0;
    if (!I2C_WriteAccelReg(MMA8451_I2C_ADDR, ACCEL_XYZ_DATA_CFG, 0x01))
        return 0;
    if (!I2C_WriteAccelReg(MMA8451_I2C_ADDR, ACCEL_CTRL_REG1, 0x0D))
        return 0;
    return 1;
}

int ACCEL_getAccelDat(SRAWDATA *accelDat)
{
    uint8_t readBuff[ACCEL_READ_LEN];

    if (I2C_ReadAccelRegs(MMA8451_I2C_ADDR, ACCEL_STATUS, readBuff, ACCEL_READ_LEN) == true)
    {
        accelDat->s = readBuff[0];

        accelDat->x = ((int16_t)(((readBuff[1] * 256U) | readBuff[2]))) / 4U;
        accelDat->y = ((int16_t)(((readBuff[3] * 256U) | readBuff[4]))) / 4U;
        accelDat->z = ((int16_t)(((readBuff[5] * 256U) | readBuff[6]))) / 4U;

        return 1;
    }

    return 0;
}
