#include "SDRAM.h"

void SDRAMInitialize(void){
	FMC_SDRAMInitTypeDef  FMC_SDRAMInitStructure;
	FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure;
	
	SDRAMGPIOInitialize();
	
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);
	
	/* FMC Configuration ---------------------------------------------------------*/
	/* FMC SDRAM Bank configuration */   
	/* Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2) */
	/* 90MHz = 11.1ns */
	/* TMRD: 加载模式寄存器命令后的延迟 */
	FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay    = 2;      
	/* TXSR: 自刷新命令后的延迟 */
	FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 10;
	/* TRAS: 自刷新时间 */
	FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime      = 6;
	/* TRC:  行循环延迟 */        
	FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay        = 9;         
	/* TWR:  恢复延迟 */
	FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime    = 6;      
	/* TRP:  行预充电延迟 */
	FMC_SDRAMTimingInitStructure.FMC_RPDelay              = 3;                
	/* TRCD: 行到列的延迟 */
	FMC_SDRAMTimingInitStructure.FMC_RCDDelay             = 3;

	/* FMC SDRAM control configuration */
	FMC_SDRAMInitStructure.FMC_Bank               = FMC_Bank1_SDRAM;
	/* Row addressing: [7:0] */
	FMC_SDRAMInitStructure.FMC_ColumnBitsNumber   = FMC_ColumnBits_Number_8b;
	/* Column addressing: [10:0] */
	FMC_SDRAMInitStructure.FMC_RowBitsNumber      = FMC_RowBits_Number_11b;
	FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth  = SDRAM_MEMORY_WIDTH;
	FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_2;
	/* CL: Cas Latency = 2 clock cycles */
	FMC_SDRAMInitStructure.FMC_CASLatency         = FMC_CAS_Latency_2; 
	FMC_SDRAMInitStructure.FMC_WriteProtection    = FMC_Write_Protection_Disable;
	FMC_SDRAMInitStructure.FMC_SDClockPeriod      = FMC_SDClock_Period_2;			// 二分频
	FMC_SDRAMInitStructure.FMC_ReadBurst          = FMC_Read_Burst_Enable;
	FMC_SDRAMInitStructure.FMC_ReadPipeDelay      = FMC_ReadPipe_Delay_0;	// SDRAM读取时钟同步延迟
	FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct  = &FMC_SDRAMTimingInitStructure;

	/* FMC SDRAM bank initialization */
	FMC_SDRAMInit(&FMC_SDRAMInitStructure); 

	/* FMC SDRAM device initialization sequence */
	SDRAM_InitSequence(); 
}

/**
  * @brief  Configures all SDRAM memory I/Os pins. 
  * @param  None. 
  * @retval None.
  */
void SDRAMGPIOInitialize(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clock */
	RCC_AHB1PeriphClockCmd(
		RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
		RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

/*-- GPIOs Configuration -----------------------------------------------------*/
/*
+-------------------+--------------------+--------------------+--------------------+
+                       SDRAM pins assignment                                      +
+-------------------+--------------------+--------------------+--------------------+
| PD0  <-> FMC_D2  V| PE0  <-> FMC_NBL0 V| PF0  <-> FMC_A0  V | PG0 <-> FMC_A10   V|
| PD1  <-> FMC_D3  V| PE1  <-> FMC_NBL1 V| PF1  <-> FMC_A1  V | PG1 <-> FMC_A11   X|
| PD8  <-> FMC_D13 V| PE7  <-> FMC_D4   V| PF2  <-> FMC_A2  V | PG4 <-> FMC_A14   V> FMC_A11/BA0
| PD9  <-> FMC_D14 V| PE8  <-> FMC_D5   V| PF3  <-> FMC_A3  V | PG5 <-> FMC_A15   X|
| PD10 <-> FMC_D15 V| PE9  <-> FMC_D6   V| PF4  <-> FMC_A4  V | PG8 <-> FC_SDCLK  V|
| PD14 <-> FMC_D0  V| PE10 <-> FMC_D7   V| PF5  <-> FMC_A5  V | PG15 <-> FMC_NCAS V|
| PD15 <-> FMC_D1  V| PE11 <-> FMC_D8   V| PF11 <-> FC_NRAS V |--------------------+ 
+-------------------| PE12 <-> FMC_D9   V| PF12 <-> FMC_A6  V | 
					| PE13 <-> FMC_D10  V| PF13 <-> FMC_A7  V |    
					| PE14 <-> FMC_D11  V| PF14 <-> FMC_A8  V |
					| PE15 <-> FMC_D12  V| PF15 <-> FMC_A9  V |
+-------------------+--------------------+--------------------+
| PH2 <-> FMC_SDCKE0X PI4 <-> FMC_NBL2  X|
| PH3 <-> FMC_SDNE0 X PI5 <-> FMC_NBL3  X|
| PH5 <-> FMC_SDNWE X--------------------+
+-------------------+
+-------------------+------------------+
+   32-bits Mode: D31-D16              +
+-------------------+------------------+
| PH8 <-> FMC_D16   | PI0 <-> FMC_D24  |
| PH9 <-> FMC_D17   | PI1 <-> FMC_D25  |
| PH10 <-> FMC_D18  | PI2 <-> FMC_D26  |
| PH11 <-> FMC_D19  | PI3 <-> FMC_D27  |
| PH12 <-> FMC_D20  | PI6 <-> FMC_D28  |
| PH13 <-> FMC_D21  | PI7 <-> FMC_D29  |
| PH14 <-> FMC_D22  | PI9 <-> FMC_D30  |
| PH15 <-> FMC_D23  | PI10 <-> FMC_D31 |
+------------------+-------------------+

+-------------------+
+  Pins remapping   +
+-------------------+
| PC0 <-> FMC_SDNWE > PA7
| PC2 <-> FMC_SDNE0 > PC4
| PC3 <-> FMC_SDCKE0> V 
+-------------------+

*/

	/* Common GPIO configuration */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	/* GPIOA configuration */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_FMC);
	
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_7;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* GPIOC configuration */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_FMC);
	
	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_3 |GPIO_Pin_4;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* GPIOD configuration */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8  | GPIO_Pin_9 |
									GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* GPIOE configuration */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
									GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
									GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* GPIOF configuration */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
									GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
									GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;      

	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* GPIOG configuration */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);


	GPIO_InitStructure.GPIO_Pin =	GPIO_Pin_0 /*|GPIO_Pin_1*/ |GPIO_Pin_4 /*|GPIO_Pin_5*/ |
									GPIO_Pin_8 | GPIO_Pin_15;

	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* GPIOH configuration */
	/*GPIO_PinAFConfig(GPIOH, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource15 , GPIO_AF_FMC);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_5 | GPIO_Pin_8  | 
								GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | 
								GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;      

	GPIO_Init(GPIOH, &GPIO_InitStructure);*/

	/* GPIOI configuration */
	/*GPIO_PinAFConfig(GPIOI, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource6 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource10 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
								GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
										GPIO_Pin_9 | GPIO_Pin_10; 

	GPIO_Init(GPIOI, &GPIO_InitStructure);*/
}

/**
  * @brief  Executes the SDRAM memory initialization sequence. 
  * @param  None. 
  * @retval None.
  */
void SDRAM_InitSequence(void)
{
	FMC_SDRAMCommandTypeDef FMC_SDRAMCommandStructure;
	uint32_t tmpr = 0;
	uint32_t timeout = SDRAM_TIMEOUT; 

	/* Step 3 --------------------------------------------------------------------*/
	/* Configure a clock configuration enable command */
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_CLK_Enabled;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
	/* Wait until the SDRAM controller is ready */ 
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);  

	/* Step 4 --------------------------------------------------------------------*/
	/* Insert 100 ms delay */
	__Delay(100);

	/* Step 5 --------------------------------------------------------------------*/
	/* Configure a PALL (precharge all) command */ 
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_PALL;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

	/* Wait until the SDRAM controller is ready */  
	timeout = SDRAM_TIMEOUT; 
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 6 --------------------------------------------------------------------*/
	/* Configure a Auto-Refresh command */ 
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_AutoRefresh;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 8;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

	/* Wait until the SDRAM controller is ready */ 
	timeout = SDRAM_TIMEOUT; 
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 7 --------------------------------------------------------------------*/
	/* Program the external memory mode register */
	tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
				   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
				   SDRAM_MODEREG_CAS_LATENCY_2           |
				   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
				   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	/* Configure a load Mode register command*/ 
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_LoadMode;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;

	/* Wait until the SDRAM controller is ready */ 
	timeout = SDRAM_TIMEOUT; 
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
	timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 8 --------------------------------------------------------------------*/

	/* Set the refresh rate counter */
	/* (15.62 us x Freq) - 20 */
	/* Set the device refresh counter */
	FMC_SetRefreshCount(1385);

	/* Wait until the SDRAM controller is ready */ 
	timeout = SDRAM_TIMEOUT; 
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
}


/**
  * @brief  Writes a Entire-word buffer to the SDRAM memory. 
  * @param  pBuffer: pointer to buffer. 
  * @param  uwWriteAddress: SDRAM memory internal address from which the data will be 
  *         written.
  * @param  uwBufferSize: number of words to write. 
  * @retval None.
  */
void SDRAM_WriteBuffer(uint32_t* pBuffer, uint32_t uwWriteAddress, uint32_t uwBufferSize)
{
	__IO uint32_t write_pointer = (uint32_t)uwWriteAddress;

	/* Disable write protection */
	FMC_SDRAMWriteProtectionConfig(FMC_Bank1_SDRAM, DISABLE);

	/* Wait until the SDRAM controller is ready */ 
	while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET)
	{
	}

	/* While there is data to write */
	for (; uwBufferSize != 0; uwBufferSize--) 
	{
		/* Transfer data to the memory */
		*(uint32_t *) (SDRAM_BANK_ADDR + write_pointer) = *pBuffer++;

		/* Increment the address*/
		write_pointer += 4;
	}
}

/**
  * @brief  Reads data buffer from the SDRAM memory. 
  * @param  pBuffer: pointer to buffer. 
  * @param  ReadAddress: SDRAM memory internal address from which the data will be 
  *         read.
  * @param  uwBufferSize: number of words to write. 
  * @retval None.
  */
void SDRAM_ReadBuffer(uint32_t* pBuffer, uint32_t uwReadAddress, uint32_t uwBufferSize)
{
	__IO uint32_t write_pointer = (uint32_t)uwReadAddress;

	/* Wait until the SDRAM controller is ready */ 
	while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET)
	{
	}

	/* Read data */
	for(; uwBufferSize != 0x00; uwBufferSize--)
	{
		*pBuffer++ = *(__IO uint32_t *)(SDRAM_BANK_ADDR + write_pointer );

		/* Increment the address*/
		write_pointer += 4;
	} 
}

