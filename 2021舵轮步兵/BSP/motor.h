#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "Driver.h"

  #define   YAW_MOTOR_MID_ANGLE   968  //电机角度中间值
  #define   PIT_MOTOR_MID_ANGLE   8099 //电机角度中间值 必须为实际角度
  #define   PIT_MOTOR_MAX_ANGLE   (300 + 8192) //PIT低头最大值
  #define   PIT_MOTOR_MIN_ANGLE   7227 //PIT抬头最大值

  #define   PIT_IMU_MID_ANGLE     4128 //IMU角度中间值
  #define   PIT_IMU_MAX_ANGLE     4436 //PIT低头最大值
  #define   PIT_IMU_MIN_ANGLE     3640 //PIT抬头最大值

  #define   PIT_SENTRY            7681



void MOTOR_Init(void);
void RM_3508_Init(void);
void Fric_3508_Init(void);
void GM_6020_Init(void);
void M_2006_Init(void);
void RUD_ParamInit(void);
void Slow(float *rec , float target , float slow_Inc);
void RUD_Slow(float *rec , float target , float slow_Inc , float max);
void MotorSlow(float *rec , float target , float slow_Inc);
void global_slow(float *rec , float target , float slow_Inc);
void CHAS_Target_Slow(float *rec , float target , float slow_Inc);
void M_2006_ParamInit(void);
void CHAS_ParamInit(void);
void RM_3508_ParamInit(PID_Loop_t *str);

#endif



