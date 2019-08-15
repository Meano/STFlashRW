#include "Delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;

/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

RCC_ClocksTypeDef RCC_Clocks;

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void DelayInitialize(uint32_t nms)
{
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency * nms / 1000 );
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
	uwTimingDelay = nTime;
	while(uwTimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
	if (uwTimingDelay != 0x00)
	{ 
		uwTimingDelay--;
	}
}

void SysTick_Handler(void)
{
	TimingDelay_Decrement();
	FrameRateUpdate();
}
