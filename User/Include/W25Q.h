#ifndef __W25Q_H__
#define __W25Q_H__

#include "stm32F4xx.h"
#include "stm32F4xx_conf.h"

extern void w25q_init(void);
extern void w25q_rd_data(uint32_t addr, uint16_t size, uint8_t* p_dat);
extern void w25q_wr_data(uint32_t addr, uint16_t size, uint8_t* p_dat);
extern void w25q_erase_4k(uint32_t addr);

#endif
