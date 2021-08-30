#ifndef __FREERTOS_INIT_H
#define __FREERTOS_INIT_H

#include "system.h"
	#include "FreeRTOS.h"					//支持FreeRTOS时，使用	
	#include "task.h"
void FreeRTOS_Init(void);
void Delay_init(u8 SYSCLK);
void delay_us(u32 nus);
void delay_ms(u32 nms);
void delay_xms(u32 nms);
#endif


