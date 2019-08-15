#include "Clock.h"

void ClockInitialize(void){
	//HSE_SetSysClock(25, 384, 2, 8, 8);
	HSI_SetSysClock(16, 384, 2, 8, 8);			// S1
	//HSI_SetSysClock(11, 384, 2, 8, 8);				// S2
}

void HSE_SetSysClock(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r)	
{
	__IO uint32_t HSEStartUpStatus = 0;
	
	// 使能HSE
	RCC_HSEConfig(RCC_HSE_ON);
	
	// 等待HSE启动稳定
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		// 调压器电压输出级别配置为1，以便在器件为最大频率
		// 工作时使性能和功耗实现平衡
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;
		
		// HCLK = SYSCLK / 1
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		
		// PCLK2 = HCLK / 2
		RCC_PCLK2Config(RCC_HCLK_Div2);
		
		// PCLK1 = HCLK / 4
		RCC_PCLK1Config(RCC_HCLK_Div4);

		// 设置PLL来源时钟，设置VCO分频因子m，设置VCO倍频因子n，
		// 设置系统时钟分频因子p
		// 设置OTG FS,SDIO,RNG分频因子q
		// 设置I2S时钟分频因子r
		RCC_PLLConfig(RCC_PLLSource_HSE, m, n, p, q, r);
		
		// 使能PLL
		RCC_PLLCmd(ENABLE);
	
		// 等待 PLL稳定
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); 

		// 开启 OVER-RIDE模式，以能达到更高频率
		PWR->CR |= PWR_CR_ODEN;
		while((PWR->CSR & PWR_CSR_ODRDY) == 0);

		PWR->CR |= PWR_CR_ODSWEN;
		while((PWR->CSR & PWR_CSR_ODSWRDY) == 0);

		// 配置FLASH预取指,指令缓存,数据缓存和等待状态
		FLASH->ACR = FLASH_ACR_PRFTEN 
								| FLASH_ACR_ICEN 
								| FLASH_ACR_DCEN 
								| FLASH_ACR_LATENCY_5WS;
		
		// 当PLL稳定之后，把PLL时钟切换为系统时钟SYSCLK
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// 读取时钟切换状态位，确保PLLCLK被选为系统时钟
		while (RCC_GetSYSCLKSource() != 0x08);
	}
	else
	{
		// HSE启动出错处理
		while (1);
	}
}

void HSI_SetSysClock(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r)	
{
	__IO uint32_t HSIStartUpStatus = 0;

	// 把RCC外设初始化成复位状态
	RCC_DeInit();

	// 使能HSI, HSI=16M
	RCC_HSICmd(ENABLE);

	// 等待 HSI 就绪
	HSIStartUpStatus = RCC->CR & RCC_CR_HSIRDY;
	
	// 只有HSI就绪之后则继续往下执行
	if (HSIStartUpStatus == RCC_CR_HSIRDY)
	{
		// 调压器电压输出级别配置为1，以便在器件为最大频率
		// 工作时使性能和功耗实现平衡
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;
		
		// HCLK = SYSCLK / 1
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		
		// PCLK2 = HCLK / 2
		RCC_PCLK2Config(RCC_HCLK_Div2);
		
		// PCLK1 = HCLK / 4
		RCC_PCLK1Config(RCC_HCLK_Div4);
		
		// 设置PLL来源时钟，设置VCO分频因子m，设置VCO倍频因子n，
		// 设置系统时钟分频因子p
		// 设置OTG FS,SDIO,RNG分频因子q
		// 设置I2S时钟分频因子r
		RCC_PLLConfig(RCC_PLLSource_HSI, m, n, p, q, r);
		
		// 使能PLL
		RCC_PLLCmd(ENABLE);
	
		// 等待 PLL稳定
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		//开启 OVER-RIDE模式，以能达到更高频率
		PWR->CR |= PWR_CR_ODEN;
		while((PWR->CSR & PWR_CSR_ODRDY) == 0)
		{
		}
		PWR->CR |= PWR_CR_ODSWEN;
		while((PWR->CSR & PWR_CSR_ODSWRDY) == 0)
		{
		}      
		// 配置FLASH预取指,指令缓存,数据缓存和等待状态
		FLASH->ACR = FLASH_ACR_PRFTEN 
						| FLASH_ACR_ICEN 
						|FLASH_ACR_DCEN 
						|FLASH_ACR_LATENCY_5WS;
		
		// 当PLL稳定之后，把PLL时钟切换为系统时钟SYSCLK
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// 读取时钟切换状态位，确保PLLCLK被选为系统时钟
		while (RCC_GetSYSCLKSource() != 0x08);
	}
	else
	{
		// HSI启动出错处理
		while (1);
	}
}

void SystemInit(void)
{
	/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
#endif

	HSI_SetSysClock(16, 192, 2, 4, 2);
	
	/* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
	SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}
