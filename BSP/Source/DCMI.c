#include "DCMI.h"

FrameInfo_s * CurrentFrame;
FrameInfo_s * ReadyFrame;
FrameInfo_s * CaptureFrame;
FrameInfo_s FrameList[MAX_SPLIT_COUNT];
DeviceInfo_s * DeviceInfo;
uint8_t DeviceInfoBuffer[512];
uint8_t * FrameBuffer;


uint32_t VSnum = 0;
uint32_t FrameRate;
uint32_t FrameTickCount;
void FrameRateUpdate(void){
	FrameTickCount++;
	if(FrameTickCount >= 2000){
		FrameRate = VSnum/2;
		VSnum = 0;
		FrameTickCount = 0;
	}
}

uint32_t FRAMEnum = 0;
uint32_t FLAG = 0;

uint32_t BlockCount = 0;
uint32_t BlockIndex = 0;
uint8_t DMAComp = 0;

uint32_t FrameBufferFlag;
uint32_t FrameIndex;
uint8_t * CaptureFrameAddress;
uint8_t * FrameBufferNow;
uint8_t * FrameBufferReady;

#include "string.h"
#include "stdlib.h"

uint8_t * GetDeviceInfoBuffer(void){
	DeviceInfo->BufferStatus = CurrentFrame->Status;
	DeviceInfo->BufferSequence = CurrentFrame->Sequence;
	DeviceInfo->TestTimeout = 0;
	return DeviceInfoBuffer;
}

void Setup(void){
	for(int i = 0; i < MAX_SPLIT_COUNT; i++) {
		FrameList[i].Order = i;
		FrameList[i].Sequence = 0;
		FrameList[i].Buffer = 0;
		FrameList[i].Status = 0;
		FrameList[i].Sector = 0;
		FrameList[i].Brightness = 0;
	}
	CurrentFrame = 0;
	ReadyFrame = 0;
	CaptureFrame = &FrameList[0];
	FrameBuffer = (uint8_t *)SDRAM_BANK_ADDR;
	
	//DeviceInfoBuffer = (uint8_t *)malloc(512);
	DeviceInfo = (DeviceInfo_s *)DeviceInfoBuffer;
	strncpy((char *)DeviceInfo->DeviceName, "WaveSamp", 16);
	DeviceInfo->CurrentSequence = 0;
	DeviceInfo->CurrentOrder = 0;
	DeviceInfo->CurrentStatus = 0;
	DeviceInfo->SplitCount = 0;
	
	SplitBuffer(0x0004B000);		// 640*480 Buffer
	DeviceInfo->ImageWidth = 640;
	DeviceInfo->ImageHeight = 480;
	
	DCMIInitialize();
}

void SplitBuffer(uint32_t ImageSize) {
	DeviceInfo->SplitCount = MAX_BUFFER_SIZE / ImageSize;
	uint32_t BufferSize = MAX_BUFFER_SIZE / DeviceInfo->SplitCount;
	for(int i = 0; i < DeviceInfo->SplitCount; i++){
		FrameList[i].Buffer = FrameBuffer + BufferSize * i;
		FrameList[i].Sector = ImageSize / 512;
		FrameList[i].Sequence = 0;
		FrameList[i].Brightness = 0;
	}
	for(int i = DeviceInfo->SplitCount; i < MAX_SPLIT_COUNT; i++){
		FrameList[i].Buffer = 0;
		FrameList[i].Status = 4;
		FrameList[i].Sector = 0;
		FrameList[i].Sequence = 0;
		FrameList[i].Brightness = 0;
	}
}

void InterruptEOF(void){
	if(0){
//		DeviceInfo->ImageWidth = SensorModule->GetImageWidth();
//		DeviceInfo->ImageHeight = SensorModule->GetImageHeight();
//		SplitBuffer(SensorModule->GetImageBufferSize());
//		DeviceInfo->CurrentOrder = DeviceInfo->SplitCount - 1;
//		CurrentFrame = &FrameList[DeviceInfo->CurrentOrder];
//		ReadyFrame = CurrentFrame;
//		CurrentFrame->Status = 1;
//		CaptureFrame = &FrameList[0];
	}else{
		DeviceInfo->CurrentSequence++;
		ReadyFrame = CaptureFrame;
		ReadyFrame->Sequence = DeviceInfo->CurrentSequence;
		ReadyFrame->Status = 0;
	}
	if(DeviceInfo->CurrentStatus == 0 && ReadyFrame->Status == 0){
		CurrentFrame = ReadyFrame;
	}
	do{
		DeviceInfo->CurrentOrder++;
		DeviceInfo->CurrentOrder %= DeviceInfo->SplitCount;
	}while(DeviceInfo->CurrentOrder == CurrentFrame->Order || DeviceInfo->CurrentOrder == ReadyFrame->Order);
	CaptureFrame = &FrameList[DeviceInfo->CurrentOrder];
	CaptureFrame->Status = 2;
	CaptureFrame->Sequence = 0;
}

bool FrameBufferLock(CEU_BufferBusyFlag BusyFlag) {
	if(DeviceInfo->CurrentStatus != 0 || CurrentFrame->Status != 0)
		return false;
	if(BusyFlag == QRBusyFlag){
		if(DeviceInfo->BufferOwner == FrameForUSB) {
			//if(IsUSBConnected)return false;
		}
	}else if(BusyFlag == USBBusyFlag){
		if(DeviceInfo->BufferOwner == FrameForDecode) {
			return false;
		}
	}
	DeviceInfo->CurrentStatus |= BusyFlag;
	return true;
}

void FrameBufferUnLock(CEU_BufferBusyFlag BusyFlag, bool IsUpdateBuffer, FrameBufferOwner_s NewOwner){
	if(NewOwner == FrameForUSB){
//		if(!IsUSBConnected){
//			IsUpdateBuffer = true;
//			NewOwner = FrameForDecode;
//		}
	}
	DeviceInfo->CurrentStatus &= (~BusyFlag);
	if(
		IsUpdateBuffer &&
		ReadyFrame->Status == 0 &&
		DeviceInfo->CurrentStatus == 0
	){
		CurrentFrame = ReadyFrame;
	}
	DeviceInfo->BufferOwner = NewOwner;
}

//void FrameBufferLock(void){
//	FrameBufferFlag = 1;
//}

//void FrameBufferUnLock(void){
//	FrameBufferFlag = 0;
//	FrameBufferNow = FrameBufferReady;
//}

void DCMIInitialize(void){
	DCMI_InitTypeDef DCMI_InitStructure;
	DCMI_CROPInitTypeDef DCMI_CROPStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* DCMI GPIO 初始化 */
	DCMIGPIOInitialize();
	
	/* 使能DCMI时钟 */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

	/* DCMI 配置 */ 
	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;		// 快照模式
	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;		// 硬件同步模式
	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;			// PCLK上升沿读取数据
	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_High;				// 帧同步高电平无效
	DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_Low;				// 水平同步低电平无效
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;		// 整个帧图像
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;	// 每次数据8位
	
	DCMI_CROPStructure.DCMI_CaptureCount = 640 - 1;
	DCMI_CROPStructure.DCMI_HorizontalOffsetCount = 0;
	DCMI_CROPStructure.DCMI_VerticalLineCount = 480 - 1;
	DCMI_CROPStructure.DCMI_VerticalStartLine = 0;

	if(OV7725Identification() == 0){
		TIM1ChangeDiv(2);
		OV7725Initialize();
		DCMI->CR |= 0 << 18;
		DCMI->CR |= 3 << 16;
		DCMI_Init(&DCMI_InitStructure);
	}else if(GC0308Identification() == 0){
		TIM1ChangeDiv(4);
		GC0308Initialize();
		DCMI_Init(&DCMI_InitStructure);
	}else{
		TIM1ChangeDiv(8);
		DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_Low;				// 帧同步高电平无效
		DCMI_Init(&DCMI_InitStructure);
		DCMI_CROPConfig(&DCMI_CROPStructure);
		DCMI_CROPCmd(ENABLE);
	}
	
	/* 配置中断 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DCMI_ITConfig(DCMI_IT_VSYNC,ENABLE);
	DCMI_ITConfig(DCMI_IT_FRAME,ENABLE);
	DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
	DCMI_ClearITPendingBit(DCMI_IT_FRAME);
	
	DCMI_Cmd(ENABLE);
	
	DCMIDMAInitialize();
	
	DCMI_CaptureCmd(ENABLE);
}

void DCMIGPIOInitialize(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能DCMI时钟 */
	RCC_AHB1PeriphClockCmd(
		DCMI_PWDN_GPIO_CLK| DCMI_RST_GPIO_CLK |
		DCMI_VSYNC_GPIO_CLK | DCMI_HSYNC_GPIO_CLK | DCMI_PIXCLK_GPIO_CLK | DCMI_XCK_GPIO_CLK |
		DCMI_D0_GPIO_CLK| DCMI_D1_GPIO_CLK| DCMI_D2_GPIO_CLK| DCMI_D3_GPIO_CLK|
		DCMI_D4_GPIO_CLK| DCMI_D5_GPIO_CLK| DCMI_D6_GPIO_CLK| DCMI_D7_GPIO_CLK, ENABLE);

	/*控制/同步信号线*/
	GPIO_InitStructure.GPIO_Pin = DCMI_VSYNC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_VSYNC_GPIO_PORT, DCMI_VSYNC_PINSOURCE, DCMI_VSYNC_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_HSYNC_GPIO_PIN ;
	GPIO_Init(DCMI_HSYNC_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_HSYNC_GPIO_PORT, DCMI_HSYNC_PINSOURCE, DCMI_HSYNC_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_PIXCLK_GPIO_PIN ;
	GPIO_Init(DCMI_PIXCLK_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_PIXCLK_GPIO_PORT, DCMI_PIXCLK_PINSOURCE, DCMI_PIXCLK_AF);

	/*数据信号*/
	GPIO_InitStructure.GPIO_Pin = DCMI_D0_GPIO_PIN ;
	GPIO_Init(DCMI_D0_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D0_GPIO_PORT, DCMI_D0_PINSOURCE, DCMI_D0_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D1_GPIO_PIN ;
	GPIO_Init(DCMI_D1_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D1_GPIO_PORT, DCMI_D1_PINSOURCE, DCMI_D1_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D2_GPIO_PIN ;
	GPIO_Init(DCMI_D2_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D2_GPIO_PORT, DCMI_D2_PINSOURCE, DCMI_D2_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D3_GPIO_PIN ;
	GPIO_Init(DCMI_D3_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D3_GPIO_PORT, DCMI_D3_PINSOURCE, DCMI_D3_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D4_GPIO_PIN ;
	GPIO_Init(DCMI_D4_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D4_GPIO_PORT, DCMI_D4_PINSOURCE, DCMI_D4_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D5_GPIO_PIN ;
	GPIO_Init(DCMI_D5_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D5_GPIO_PORT, DCMI_D5_PINSOURCE, DCMI_D5_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D6_GPIO_PIN ;
	GPIO_Init(DCMI_D6_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D6_GPIO_PORT, DCMI_D6_PINSOURCE, DCMI_D6_AF);

	GPIO_InitStructure.GPIO_Pin = DCMI_D7_GPIO_PIN ;
	GPIO_Init(DCMI_D7_GPIO_PORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(DCMI_D7_GPIO_PORT, DCMI_D7_PINSOURCE, DCMI_D7_AF);


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = DCMI_PWDN_GPIO_PIN ;
	GPIO_Init(DCMI_PWDN_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = DCMI_RST_GPIO_PIN ;
	GPIO_Init(DCMI_RST_GPIO_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN);
	GPIO_SetBits(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN);
}

void DCMIDMAInitialize(void) {
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/* 使能DMA时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);  
	DMA_DeInit(DMA2_Stream1);

	DMA_InitStructure.DMA_Channel = DMA_Channel_1;							//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = DCMI_DR_ADDRESS;				//DCMI数据寄存器地址
	DMA_InitStructure.DMA_Memory0BaseAddr = SDRAM_BANK_ADDR;				//DMA传输的目的地址(传入的参数)	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = SDRAM_DMA_BUF_SIZE;					//传输的数据大小(传入的参数)
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//寄存器地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							//循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;					//FIFO模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;			//FIFO阈值
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发传输
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);

	DMA_DoubleBufferModeConfig(DMA2_Stream1, SDRAM_BANK_ADDR + HALF_FRAME_SIZE, DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA2_Stream1, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*DMA中断配置 */

	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
	
	DMA_Cmd(DMA2_Stream1, ENABLE);
}

void DCMI_IRQHandler(void) {
	if(DCMI_GetITStatus(DCMI_IT_VSYNC) != RESET){
		VSnum ++;
		DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
	}
	if(DCMI_GetITStatus(DCMI_IT_FRAME) != RESET){
		DMA_Cmd(DMA2_Stream1, DISABLE);
		InterruptEOF();
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		DMA_MemoryTargetConfig(DMA2_Stream1, (uint32_t)(CaptureFrame->Buffer), DMA_Memory_0);
		DMA_MemoryTargetConfig(DMA2_Stream1, ((uint32_t)(CaptureFrame->Buffer)) + HALF_FRAME_SIZE, DMA_Memory_1);
		DMA_Cmd(DMA2_Stream1, ENABLE);
		DCMI_CaptureCmd(ENABLE);
		DCMI_ClearITPendingBit(DCMI_IT_FRAME);
	}
}


void DMA2_Stream1_IRQHandler(void) {
	if(DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) != RESET){
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream1) == 1){	// 开始了Memory 1的传输，配置下一帧Memory 0的地址
/*			if(FLAG == 0xFFFF)FLAG = BlockCount;
			do{
				BlockCount ++;
				if(BlockCount >= FRAME_COUNT){
					BlockCount = 0;
				}
			}while(FLAG == 0xEEEE && BlockCount == BlockIndex); */
			//DMA_MemoryTargetConfig(DMA2_Stream1, (uint32_t)CaptureFrameAddress, DMA_Memory_0);
		} else {											// 开始了Memory 0的传输，配置当前帧Memory 1的地址
			DMA_Cmd(DMA2_Stream1, DISABLE);
			//DMA_MemoryTargetConfig(DMA2_Stream1, ((uint32_t)CaptureFrameAddress) + HALF_FRAME_SIZE, DMA_Memory_1);
		}
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
	}
}

