#ifndef __DCMIH
#define __DCMIH

#include "Root.h"
#include "SDRAM.h"
#include "SCCB.h"
#include "Timer.h"

#include "Config_GC0308.h"
#include "Config_OV7725.h"

//VSYNC
#define DCMI_VSYNC_GPIO_PORT    	GPIOB
#define DCMI_VSYNC_GPIO_CLK     	RCC_AHB1Periph_GPIOB
#define DCMI_VSYNC_GPIO_PIN     	GPIO_Pin_7
#define DCMI_VSYNC_PINSOURCE    	GPIO_PinSource7
#define DCMI_VSYNC_AF				GPIO_AF_DCMI

// HSYNC
#define DCMI_HSYNC_GPIO_PORT    	GPIOA
#define DCMI_HSYNC_GPIO_CLK     	RCC_AHB1Periph_GPIOA
#define DCMI_HSYNC_GPIO_PIN     	GPIO_Pin_4
#define DCMI_HSYNC_PINSOURCE    	GPIO_PinSource4
#define DCMI_HSYNC_AF				GPIO_AF_DCMI

//PIXCLK
#define DCMI_PIXCLK_GPIO_PORT   	GPIOA
#define DCMI_PIXCLK_GPIO_CLK    	RCC_AHB1Periph_GPIOA
#define DCMI_PIXCLK_GPIO_PIN    	GPIO_Pin_6
#define DCMI_PIXCLK_PINSOURCE		GPIO_PinSource6
#define DCMI_PIXCLK_AF				GPIO_AF_DCMI

//XCK
#define DCMI_XCK_GPIO_PORT      	GPIOA
#define DCMI_XCK_GPIO_CLK       	RCC_AHB1Periph_GPIOA
#define DCMI_XCK_GPIO_PIN       	GPIO_Pin_8
#define DCMI_XCK_PINSOURCE      	GPIO_PinSource8
#define DCMI_XCK_AF					GPIO_AF_MCO

//PWDN
#define DCMI_PWDN_GPIO_PORT     	GPIOD
#define DCMI_PWDN_GPIO_CLK      	RCC_AHB1Periph_GPIOD
#define DCMI_PWDN_GPIO_PIN      	GPIO_Pin_6

//RST
#define DCMI_RST_GPIO_PORT      	GPIOD
#define DCMI_RST_GPIO_CLK       	RCC_AHB1Periph_GPIOD
#define DCMI_RST_GPIO_PIN       	GPIO_Pin_7

//数据信号线
#define DCMI_D0_GPIO_PORT       	GPIOC
#define DCMI_D0_GPIO_CLK        	RCC_AHB1Periph_GPIOC
#define DCMI_D0_GPIO_PIN        	GPIO_Pin_6
#define DCMI_D0_PINSOURCE       	GPIO_PinSource6
#define DCMI_D0_AF					GPIO_AF_DCMI

#define DCMI_D1_GPIO_PORT       	GPIOC
#define DCMI_D1_GPIO_CLK        	RCC_AHB1Periph_GPIOC
#define DCMI_D1_GPIO_PIN        	GPIO_Pin_7
#define DCMI_D1_PINSOURCE       	GPIO_PinSource7
#define DCMI_D1_AF					GPIO_AF_DCMI

#define DCMI_D2_GPIO_PORT       	GPIOC
#define DCMI_D2_GPIO_CLK        	RCC_AHB1Periph_GPIOC
#define DCMI_D2_GPIO_PIN        	GPIO_Pin_8
#define DCMI_D2_PINSOURCE       	GPIO_PinSource8
#define DCMI_D2_AF					GPIO_AF_DCMI

#define DCMI_D3_GPIO_PORT        	GPIOC
#define DCMI_D3_GPIO_CLK         	RCC_AHB1Periph_GPIOC
#define DCMI_D3_GPIO_PIN         	GPIO_Pin_9
#define DCMI_D3_PINSOURCE        	GPIO_PinSource9
#define DCMI_D3_AF					GPIO_AF_DCMI

#define DCMI_D4_GPIO_PORT        	GPIOC
#define DCMI_D4_GPIO_CLK         	RCC_AHB1Periph_GPIOC
#define DCMI_D4_GPIO_PIN         	GPIO_Pin_11
#define DCMI_D4_PINSOURCE        	GPIO_PinSource11
#define DCMI_D4_AF					GPIO_AF_DCMI

#define DCMI_D5_GPIO_PORT        	GPIOB
#define DCMI_D5_GPIO_CLK         	RCC_AHB1Periph_GPIOB
#define DCMI_D5_GPIO_PIN         	GPIO_Pin_6
#define DCMI_D5_PINSOURCE        	GPIO_PinSource6
#define DCMI_D5_AF					GPIO_AF_DCMI

#define DCMI_D6_GPIO_PORT        	GPIOB
#define DCMI_D6_GPIO_CLK         	RCC_AHB1Periph_GPIOB
#define DCMI_D6_GPIO_PIN         	GPIO_Pin_8
#define DCMI_D6_PINSOURCE        	GPIO_PinSource8
#define DCMI_D6_AF					GPIO_AF_DCMI

#define DCMI_D7_GPIO_PORT       	GPIOB
#define DCMI_D7_GPIO_CLK        	RCC_AHB1Periph_GPIOB
#define DCMI_D7_GPIO_PIN        	GPIO_Pin_9
#define DCMI_D7_PINSOURCE       	GPIO_PinSource9
#define DCMI_D7_AF					GPIO_AF_DCMI

#define DCMI_DR_ADDRESS 			0x50050028
#define FRAME_SIZE  				((uint32_t)0x0004B000)			// 640*480
#define HALF_FRAME_SIZE 			((uint32_t)(FRAME_SIZE/2))
#define SDRAM_DMA_BUF_SIZE  		((uint32_t)(HALF_FRAME_SIZE/4))
#define FRAME_COUNT 				((uint32_t)(3))

#define MAX_SPLIT_COUNT				(16)
#define MAX_BUFFER_SIZE				(0x00200000)

#ifndef __cplusplus

#define bool	_Bool
#define false	0
#define true	1

#endif /* __cplusplus */

typedef enum {
	FrameForAny,
	FrameForUSB,
	FrameForDecode
} FrameBufferOwner_s;

typedef enum {
	USBBusyFlag          = 0x00000001,
	QRBusyFlag           = 0x00000020,
} CEU_BufferBusyFlag;

typedef volatile struct{
	uint32_t Order;
	uint32_t Sequence;
	uint32_t Status;
	uint32_t Sector;
	uint8_t Brightness;
	uint8_t * Buffer;
} FrameInfo_s;

typedef volatile struct {
	uint8_t DeviceName[16];
	uint8_t SnapshotReady;
	uint8_t SnapshotCount;
	uint16_t ImageWidth;
	uint16_t ImageHeight;
	uint32_t CurrentSequence;
	uint32_t BufferOwner;
	uint8_t ReservedA[32-30];
	uint32_t CurrentOrder;
	uint32_t SplitCount;
	uint32_t CurrentStatus;
	uint32_t BufferStatus;
	uint32_t BufferSequence;
	uint32_t TestTimeout;
//	uint8_t ReservedC[512 - 32 - 16 - 8];
} __attribute__((packed)) DeviceInfo_s;

extern FrameInfo_s * CurrentFrame;
extern FrameInfo_s * ReadyFrame;
extern FrameInfo_s * CaptureFrame;
extern FrameInfo_s FrameList[MAX_SPLIT_COUNT];
extern DeviceInfo_s * DeviceInfo;
extern uint8_t DeviceInfoBuffer[512];
extern uint8_t * FrameBuffer;

extern uint8_t * GetDeviceInfoBuffer(void);
extern void Setup(void);
extern void SplitBuffer(uint32_t ImageSize);

extern uint32_t FrameRate;
extern uint8_t * FrameBufferNow;
extern void FrameRateUpdate(void);

extern void DCMIInitialize(void);
extern void DCMIGPIOInitialize(void);
extern void DCMIDMAInitialize(void);
extern void DCMIGC0308Initialize(void);
extern void DCMI_IRQHandler(void);
extern void DMA2_Stream1_IRQHandler(void);
extern bool FrameBufferLock(CEU_BufferBusyFlag BusyFlag);
extern void FrameBufferUnLock(CEU_BufferBusyFlag BusyFlag, bool IsUpdateBuffer, FrameBufferOwner_s NewOwner);

#endif
