#ifndef __LED_H
#define __LED_H

#include "system.h"

typedef void (*Hint_LED_t)(void);
extern Hint_LED_t Hint_LED;

#define Green_On		 GPIO_ResetBits(GPIOC,GPIO_Pin_10)
#define Green_Off	   GPIO_SetBits(GPIOC,GPIO_Pin_10)
#define LED_GREEN    PCout(10)

#define Red_On			 GPIO_ResetBits(GPIOC,GPIO_Pin_11)
#define Red_Off	     GPIO_SetBits(GPIOC,GPIO_Pin_11)
#define LED_RED    PCout(11)

#define Blue_On  	 GPIO_ResetBits(GPIOC,GPIO_Pin_13)
#define Blue_Off   GPIO_SetBits(GPIOC,GPIO_Pin_13)
#define LED_BLUE    PCout(13)

#define Orange_On      GPIO_ResetBits(GPIOC,GPIO_Pin_14)
#define Orange_Off     GPIO_SetBits(GPIOC,GPIO_Pin_14)
#define LED_ORANGE    PCout(14)

#define AimLED_Ctrl     PDout(9)
void Led_Init(void);
void Hint_Led_Init(void);
void AimLED_Init(void);
void System_Alive_Hint(void);
void RC_Lost_Hint(void);
void RC_Err_Hint(void);
void NMI_Handler_Hint(void);
void HardFault_Handler_Hint(void);
void BusFault_Handler_Hint(void);
void RC_Mode_Change_Hint(void);
void System_LED_Hint(Hint_LED_t p);
#endif

