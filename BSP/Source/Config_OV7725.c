#include "Config_OV7725.h"

const uint8_t OV7725Config[12][2] = 
{
	{0x09, 0x03},		// 4x驱动能力
	{0x11, 0x40},		// 使用外部时钟
	{0x0D, 0x01},		// 不倍频
	{0x12, 0x00},		// VGA配置，YUV输出
	{0x13, 0x8E},		// 优化关闭
	{0x15, 0x00},		// PCLK上升沿采集数据
	{0x17, 0x23},
	{0x18, 0xA0},
	{0x19, 0x07},
	{0x1A, 0xF0},
	{0x0C, 0x10},		// 不交换Y/UV,Test Image output
	{0x66, 0x00}		// 不交换UV,Test Image outbut
};

uint8_t OV7725Identification(void){
	OV7725Start();
	
	if(OV7725ReadID() != OV7725_ID){
		return 0x01;
	}else{
		return 0x00;
	}
}

uint8_t OV7725Initialize(){
	uint32_t i = 0;
	OV7725Start();
	
	for(i=0; i<(sizeof(OV7725Config)/2); i++)
	{
		SCCBWriteReg(OV7725_SCCB_ADDR, OV7725Config[i][0], OV7725Config[i][1]);
		Delay(1);
	}
	
	return 0x00;
}

uint8_t OV7725ReadID(){
	uint8_t tmp = 0;
	__IO uint8_t error = 0;
	error = SCCBReadReg(OV7725_SCCB_ADDR, 0x0B, &tmp);
	return tmp;
}

void OV7725SoftRst(){
	SCCBWriteReg(OV7725_SCCB_ADDR, 0x12, 0x80);
}

void OV7725Start(void){
	OV7725_RST;			// 复位
	OV7725_PWOFF;		// 关机
	Delay(10);
	
	OV7725_PWON;		// 开机
	Delay(10);
	
	OV7725_UNRST;		// 复位完成
	Delay(10);
	
	OV7725SoftRst();	// 软复位
	Delay(10);
}
