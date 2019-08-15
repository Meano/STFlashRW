#ifndef __SCCBH
#define __SCCBH

#include "Root.h"

#define SCCB_I2C					I2C2
#define SCCB_I2C_CLK				RCC_APB1Periph_I2C2

#define SCCB_I2C_SCL_PIN			GPIO_Pin_10
#define SCCB_I2C_SCL_GPIO_PORT		GPIOB
#define SCCB_I2C_SCL_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SCCB_I2C_SCL_SOURCE			GPIO_PinSource10
#define SCCB_I2C_SCL_AF				GPIO_AF_I2C2

#define SCCB_I2C_SDA_PIN			GPIO_Pin_11
#define SCCB_I2C_SDA_GPIO_PORT		GPIOB
#define SCCB_I2C_SDA_GPIO_CLK		RCC_AHB1Periph_GPIOB
#define SCCB_I2C_SDA_SOURCE			GPIO_PinSource11
#define SCCB_I2C_SDA_AF				GPIO_AF_I2C2

extern void SCCBInitialize(void);
extern uint8_t SCCBWriteReg(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t RegVal);
extern uint8_t SCCBReadReg(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t * RegVal);

#endif
