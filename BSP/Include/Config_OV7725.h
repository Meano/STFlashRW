#ifndef __Config_OV7725H
#define __Config_OV7725H

#include "Root.h"
#include "SCCB.h"
#include "DCMI.h"

#define OV7725_PWON			GPIO_ResetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN)
#define OV7725_PWOFF		GPIO_SetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN)

#define OV7725_RST			GPIO_ResetBits(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN)
#define OV7725_UNRST		GPIO_SetBits(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN)

#define OV7725_SCCB_ADDR	0x42

#define OV7725_ID			0x21

extern const uint8_t OV7725Config[12][2];

extern uint8_t OV7725Initialize(void);
extern uint8_t OV7725ReadID(void);
extern uint8_t OV7725Identification(void);
extern void OV7725SoftRst(void);
extern void OV7725Start(void);

#endif
