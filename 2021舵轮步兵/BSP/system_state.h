#ifndef __SYSTEM_STATE_H
#define __SYSTEM_STATE_H

#include "system.h"


void System_Normal_process(void);
void system_state_Ctrl(void);
void prev_sys_state_Err(System_State_t a);
void Car_StateSync(CAR_ACT_FLAG_t *str);
void User_StateSyne(User_state_t *str);
#endif
