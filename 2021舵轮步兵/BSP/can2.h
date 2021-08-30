#ifndef __CAN2_H__
#define __CAN2_H__

#include "Driver.h"

extern uint32_t CAN2_time;

/*CAN2*/
#if (NEW_CTRL == 0)
#define CAN2_IO_CLOCK           RCC_AHB1Periph_GPIOB
#define CAN2_CLOCK              RCC_APB1Periph_CAN2
#define CAN2_IO_L               GPIO_Pin_12
#define CAN2_IO_H               GPIO_Pin_13
#define CAN2_IO_PinSource_L     GPIO_PinSource12
#define CAN2_IO_PinSource_H     GPIO_PinSource13
#define CAN2_IO_Port            GPIOB

#elif (NEW_CTRL == 1)
#define CAN2_IO_CLOCK           RCC_AHB1Periph_GPIOB
#define CAN2_CLOCK              RCC_APB1Periph_CAN2
#define CAN2_IO_L               GPIO_Pin_5
#define CAN2_IO_H               GPIO_Pin_6
#define CAN2_IO_PinSource_L     GPIO_PinSource5
#define CAN2_IO_PinSource_H     GPIO_PinSource6
#define CAN2_IO_Port            GPIOB
#endif

void CAN2_Init(void);
void CAN2_Send(uint32_t Equipment_ID,int16_t *Data);
bool Judge_IF_CAN2_Normal(void);
uint8_t CAN2_DUM_Send(uint32_t Equipment_ID,uint8_t *Data);
#define IF_CAN2_NORMAL   Judge_IF_CAN2_Normal()

#endif 
