#ifndef __ClockH
#define __ClockH

#include "Root.h"

#define VECT_TAB_OFFSET		0x00

extern void ClockInitialize(void);
extern void HSI_SetSysClock(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r);
extern void HSE_SetSysClock(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r);
extern void SystemInit(void);

#endif
