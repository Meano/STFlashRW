#ifndef __Config_GC0308H
#define __Config_GC0308H

#include "Root.h"
#include "SCCB.h"
#include "DCMI.h"

#define GC0308_PWON			GPIO_ResetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN)
#define GC0308_PWOFF		GPIO_SetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN)

#define GC0308_RST			GPIO_ResetBits(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN)
#define GC0308_UNRST		GPIO_SetBits(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN)

#define GC0308_SCCB_ADDR	0x42
#define GC0308_ID			0x9B

#define GC0308_Hb			(0x06A)
#define GC0308_Hb_L8		(GC0308_Hb & 0x0FF)
#define GC0308_Hb_H4		(GC0308_Hb >> 8)
#define GC0308_Vb			(0x064)//(0x070) (Vb+VWv)*rowtime = AntiTime
#define GC0308_Vb_L8		(GC0308_Vb & 0x0FF)
#define GC0308_Vb_H4		(GC0308_Vb >> 8)

#define GC0308_ShDelay		(0x42)

#define GC0308_HW			(640+8)
#define GC0308_VW			(480+8)

#define GC0308_RowTime		((GC0308_Hb + GC0308_ShDelay + GC0308_HW + 4)/24)		// 34us
#define GC0308_AntiTime		(1000000/100)			// 10000us 50HzÈÕ¹âµÆ

#define GC0308_Step			(GC0308_AntiTime/GC0308_RowTime)
#define GC0308_Step_L8		(GC0308_Step & 0x0FF)
#define GC0308_Step_H4		(GC0308_Step >> 8)

#define GC0308_Exp1			(GC0308_Step*1)
#define GC0308_Exp1_L8		(GC0308_Exp1 & 0x0FF)
#define GC0308_Exp1_H4		(GC0308_Exp1 >> 8)
#define GC0308_Exp2			(GC0308_Step*2)
#define GC0308_Exp2_L8		(GC0308_Exp2 & 0x0FF)
#define GC0308_Exp2_H4		(GC0308_Exp2 >> 8)
#define GC0308_Exp3			(GC0308_Step*3)
#define GC0308_Exp3_L8		(GC0308_Exp3 & 0x0FF)
#define GC0308_Exp3_H4		(GC0308_Exp3 >> 8)
#define GC0308_Exp4			(GC0308_Step*4)
#define GC0308_Exp4_L8		(GC0308_Exp4 & 0x0FF)
#define GC0308_Exp4_H4		(GC0308_Exp4 >> 8)


extern uint8_t GC0308Config[56][2];

extern uint8_t GC0308Initialize(void);
extern uint8_t GC0308ReadID(void);
extern uint8_t GC0308Identification(void);
extern void GC0308SoftRst(void);
extern void GC0308Start(void);

#endif
