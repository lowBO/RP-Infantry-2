#ifndef __PWM_H__
#define __PWM_H__

#include "system.h"

void PWM1_Init(void);
void PWM3_Init(void);
void Friction_PWM(int16_t pwm1,int16_t pwm2);
void Feeding_Bullet_PWM(int16_t pwm1);

#define PWM1  TIM1->CCR2
#define PWM2  TIM1->CCR1

#endif

