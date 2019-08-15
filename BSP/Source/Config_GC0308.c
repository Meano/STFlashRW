#include "Config_GC0308.h"

uint8_t GC0308Identification(){
	GC0308Start();
	
	if(GC0308ReadID() != GC0308_ID){
		return 0x01;
	}else{
		return 0x00;
	}
}

uint8_t GC0308Initialize(){
	__IO uint32_t i,error;
	
	GC0308Start();
	
	for(i=0; i<(sizeof(GC0308Config)/2); i++)
	{
		error = SCCBWriteReg(GC0308_SCCB_ADDR, GC0308Config[i][0], GC0308Config[i][1]);
		if(error)while(1);
		Delay(1);
	}
	
	for(i=0; i<(sizeof(GC0308Config)/2); i++)
	{
		//error = SCCBReadReg(GC0308_SCCB_ADDR, GC0308Config1[i][0], &(GC0308Config1[i][1]));
		if(error)while(1);
		Delay(1);
	}
	
	return 0x00;
}

void GC0308Start(){
	GC0308_RST;			// 复位
	GC0308_PWOFF;		// 关机
	Delay(10);
	
	GC0308_PWON;		// 开机
	Delay(10);
	
	GC0308_UNRST;		// 复位完成
	Delay(10);
	
	GC0308SoftRst();	// 软复位
	Delay(10);
}

uint8_t GC0308ReadID(){
	uint8_t tmp = 0;
	__IO uint8_t error = 0;
	error = SCCBReadReg(GC0308_SCCB_ADDR, 0x00, &tmp);
	return tmp;
}

void GC0308SoftRst(){
	SCCBWriteReg(GC0308_SCCB_ADDR, 0xFE, 0x80);
}

uint8_t GC0308Config[56][2] =
{
	{0xfe, 0x00},			// Reg Page 0
	{0x01, GC0308_Hb_L8},	// HB
	{0x02, GC0308_Vb_L8},	// VB
	{0x03, 0x00},			// 曝光时间
	{0x04, 0x96},			
	{0x05, 0x00},			// 行起始像素0
	{0x06, 0x00},
	{0x07, 0x00},			// 列起始像素0
	{0x08, 0x00},
	{0x09, 0x01},			// 列像素数488
	{0x0A, 0xE8},
	{0x0B, 0x02},			// 行像素数648
	{0x0C, 0x88},				
	{0x0D, 0x08},			// HSYNC前后延迟
	{0x0E, 0x08},
	{0x0F, 0x00},			// HB/VB高4位
	
	{0x10, 0x22},
	{0x11, 0x0d},
	{0x12, 0x2a},
	{0x13, 0x00},
	{0x14, 0x13},		
	{0x15, 0x0a},
	{0x16, 0x05},
	{0x17, 0x01},
	{0x1a, 0x2a},
	{0x1b, 0x00},
	{0x1c, 0x49},
	{0x1d, 0x9a},
	{0x1e, 0x61},
	{0x1f, 0x3f},
	
	{0x24, 0xB1},			// 只输出Y
	{0x25, 0x0F},			// 允许输出
	{0x26, 0x3A},			// VSYNC低有效
	
	{0xd2, 0x10},			// close AEC
	{0x22, 0x40},			// close AWB
	
	{0x5a, 0x56},
	{0x5b, 0x40},
	{0x5c, 0x4a},			
	{0x22, 0x57},			// Open AWB
	
	{0xd1, 0x21},			// 两帧调节一次AEC
	{0xd3, 0x30},			// 调节亮度
	{0xe2, GC0308_Step_H4},
	{0xe3, GC0308_Step_L8},
	{0xe4, GC0308_Exp1_H4},
	{0xe5, GC0308_Exp1_L8},
	{0xe6, GC0308_Exp2_H4},
	{0xe7, GC0308_Exp2_L8},
	{0xe8, GC0308_Exp3_H4},
	{0xe9, GC0308_Exp3_L8},
	{0xea, GC0308_Exp4_H4},
	{0xeb, GC0308_Exp4_L8},
	{0xec, 0x02<<4},
	{0xed, 0x01},
	{0xef, 0x40},
	{0xee, 0x80},
	{0xd2, 0x10}
};
