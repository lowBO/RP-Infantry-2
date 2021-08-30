#ifndef __CAN1_H__
#define __CAN1_H__

#include "Driver.h"


#if (NEW_CTRL == 0)
/*CAN1*/
#define CAN1_IO_CLOCK            RCC_AHB1Periph_GPIOA
#define CAN1_CLOCK               RCC_APB1Periph_CAN1
#define CAN1_IO_L                GPIO_Pin_11
#define CAN1_IO_H                GPIO_Pin_12
#define CAN1_IO_PinSource_L      GPIO_PinSource11
#define CAN1_IO_PinSource_H      GPIO_PinSource12
#define CAN1_IO_Port             GPIOA
#define CAN1_NVIC_IRQChannel     CAN1_RX0_IRQn

#elif (NEW_CTRL == 1)
/*CAN1*/
#define CAN1_IO_CLOCK            RCC_AHB1Periph_GPIOD
#define CAN1_CLOCK               RCC_APB1Periph_CAN1
#define CAN1_IO_L                GPIO_Pin_1
#define CAN1_IO_H                GPIO_Pin_0
#define CAN1_IO_PinSource_L      GPIO_PinSource1
#define CAN1_IO_PinSource_H      GPIO_PinSource0
#define CAN1_IO_Port             GPIOD
#define CAN1_NVIC_IRQChannel     CAN1_RX0_IRQn
#endif

extern uint32_t CAN1_time;
extern uint32_t DUM_Connect_time ;//上下主控联系

void CAN1_Init(void);
void CAN1_Send(uint32_t Equipment_ID,int16_t *Data)	;
uint8_t CAN1_DUM_Send(uint32_t Equipment_ID,uint8_t *Data);
void DataUpdate(CanRxMsg *Rx);
bool Judge_IF_CAN1_Normal(void);
#define IF_CAN1_NORMAL   Judge_IF_CAN1_Normal()
bool Judge_IF_DUM_Normal(void);
#define IF_DUM_NORMAL   Judge_IF_DUM_Normal()

#endif

