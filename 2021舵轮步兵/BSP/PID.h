#ifndef __PID_H
#define __PID_H

#include "Driver.h"


typedef struct 
{
  System_Ctrl_Mode_t ctrl_mode;
  System_Pid_Mode_t pid_mode;
  System_Action_t action_mode; 
  PID_type_t PID_type;
}PID_Core_Ctrl_t;


float PID_Algorithm(PID_Loop_t *str);
void PID_Ctrl(void);
void PID_GET_Info(void);
void Judge_PID_Type(void);
void PID_Reset(void);
PID_type_t GET_PID_Type(void);

/* 此函数用于debug调试电机参数，直接在模式控制函数调用即可
  （要放在一次执行if之外） */
void PID_Debug(PID_Info_t *str);
#endif

