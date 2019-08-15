#ifndef __DelayH
#define __DelayH

#include "Root.h"

extern void DelayInitialize(uint32_t nms);
extern void TimingDelay_Decrement(void);
extern void Delay(__IO uint32_t nTime);
extern void SysTick_Handler(void);

#endif
