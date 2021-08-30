#ifndef __MODULE_H
#define __MODULE_H

#include "system.h"

//#define   IF_REFER_CONNECT   0        //裁判系统是否有装上

#define FRICTION_INIT_DELAT       ( 1 *FreeRTOS_1S_Unit / MODULE_TASK_TIM )

  #if (Fric_motor == Fric_Bee)//小蜜蜂电机
    #define FRICTION_INIT_SPEED 1000
    #define FRICTION_STOP_SPEED 0
    #define FRICTION_L1_SPEED 515
    #define FRICTION_L2_SPEED 570
    #define FRICTION_L3_SPEED 841
  #elif (Fric_motor == Fric_3508)//3508电机
    #define FRICTION_INIT_SPEED 0
    #define FRICTION_STOP_SPEED 0
    #define FRICTION_L1_SPEED 4495//649
    #define FRICTION_L2_SPEED 4882
    #define FRICTION_L3_SPEED 6823
  #endif




typedef enum 
{
  SHOOT_L1_SPEED = 15, //15射速
  SHOOT_L2_SPEED = 18, //18射速
  SHOOT_L3_SPEED = 30, //30射速
}Shoot_Speed_Level_t;


//弹舱舵机 开关数值
typedef enum 
{
  CLIP_OPEN_NUM   =  46,
  CLIP_CLOSE_NUM  =  106,
}Clip_State_Num_t;

typedef enum
{
  left = 0,
  righ = 1,
  Fric_num = 2,
}Fric_motor_position_t;

typedef struct
{
  Motor_Data_t motor_Data;
  PID_Info_t PID;
}Fric_3508_Info_t;

/* 摩擦轮 */
typedef struct
{
  bool IF_WORK;
  bool IF_INIT_OK;
  float friction_num;
  float friciont_num_target;
  uint8_t now_speed_level;
  uint8_t speed_limit_level;
  float Level_1_fix_num;
  float Level_2_fix_num;
  float Level_3_fix_num;
  
  Fric_3508_Info_t Fric[Fric_num];//3508摩擦轮使用
}Friction_Info_t;


/* LED */
typedef struct
{
  bool IF_WORK;
}AimLED_Info_t;

/* 舵机 */
typedef struct
{
  Clip_State_Num_t Clip_State;
  bool IF_CLIP_OPEN;
}Clip_Info_t;

/* 总结构体 */
typedef struct
{
  Friction_Info_t Friction;
  AimLED_Info_t AimLED;
  Clip_Info_t Clip;
  
  System_Ctrl_Mode_t ctrl_mode;
  System_Pid_Mode_t pid_mode;
  
  bool REMOTE_or_REFER_NORMAL;
}Module_Info_t;

extern Module_Info_t Module;

#define   IF_MODULE_WORK    (Module.Friction.IF_WORK && Module.AimLED.IF_WORK)
#define   CLIP_STATE        (Module.Clip.Clip_State)

void MODULE_Ctrl(void);
void First_GetInto_Mode(void);

void RC_MODULE_State_Update(void);
void Module_KEY_ON(void);
void Module_KEY_OFF(void);

void RC_Clip_State_Updata(void);
void KEY_Clip_State_Updata(void);

void RC_Friction_Ctrl(void);
void KEY_Friction_Ctrl(void);

void RC_AimLED_Ctrl(void);
void KEY_AimLED_Ctrl(void);  

void RC_Clip_Ctrl(void);
void KEY_Clip_Ctrl(void);
void Clip_turn(FunctionalState state);

void Friction_Reset(void);
void AimLED_Reset(void);
void Clip_Reset(void);
void MODULE_Reset(void);

void MODULE_GetInfo(void);
void Module_GetReferInfo(void);

void Friction_Init(void);
void Friction_Init_Delay(void);
void Friction_SetInitNum(void);
void Clip_Init(void);
void MODULE_Init(void);

//功能层------------------------------
void Clip_Close(void);
void Clip_Open(void);
void Judge_Clip_state(void);
bool Report_IF_CLIP_OPEN(void);

void AimLED_On(void);
void AimLED_Off(void);
  
void Friction_On(void);
void Friction_Off(void);
void Check_IF_ArmorBooster_Deinit(void);

void Fric_Speed_Change(void);
void Fric_Speed_Slope(float * now_speed,float change_target_speed);
uint8_t Report_Shoot_Speed(void);
bool Report_IF_Fric_Init_Over(void);

void CLIP_Set_CloseNum(void);
void CLIP_Set_OpenNum(void);

//自适应转速
void Friction_SpeedFIX(void);


//摩擦轮使用3508--------------------------
void Fric_L_Set_SpeedTarget(float TargetSpeed);
void Fric_R_Set_SpeedTarget(float TargetSpeed);
void Fric_3508_Handler(void);
void Fric_3508_Reset(void);
void Fric_3508_SetPID(void);
void Fric_3508_stop(void);
bool Report_IF_Fric3508_SetSpeed(void);

#endif
