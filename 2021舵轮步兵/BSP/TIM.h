#ifndef __TIM_H
#define __TIM_H

#include "system.h"

typedef enum
{
  up = 0,
  down = 1,
}KEY_STATE_t;

#define KEY_Left  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)
#define KEY_Righ  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)

void PWM_Friction_Init(u16 arr,u16 psc);
void PWM_Clip_Init(u16 arr,u16 psc);
void Trigger_Cnt_Init(u16 arr,u16 psc);
void KEY_Init(void);
void KEY_SCAN(void);
bool Report_IF_TurnLeft_allow(void);
bool Report_IF_TurnRigh_allow(void);
#endif 
