#ifndef __GO_H
#define __GO_H

#include "system.h"

#define     TASK_TIM_Unit   1  //FreeRTOS一毫秒单位,软实时1ms单位是1000
#define     FreeRTOS_1S_Unit     1000
/* 任务时数 */
#define     SYSTEM_STATE_TASK_TIM    ( 1    *TASK_TIM_Unit)
#define     GIMBAL_TASK_TIM          ( 2    *TASK_TIM_Unit)
#define     CHASSIS_TASK_TIM         ( 2    *TASK_TIM_Unit)
#define     RUDDER_TASK_TIM          ( 2    *TASK_TIM_Unit)
#define     RIFLE_TASK_TIM           ( 1    *TASK_TIM_Unit)
#define     IMU_TASK_TIM             ( 1    *TASK_TIM_Unit)
#define     VISION_TASK_TIM          ( 2    *TASK_TIM_Unit)
#define     MODULE_TASK_TIM          ( 2    *TASK_TIM_Unit)
#define     PID_TASK_TIM             ( 2    *TASK_TIM_Unit)
#define     MASTERLINK_TASK_TIM      ( 1    *TASK_TIM_Unit)
#define     VISION_TRIGGER_TASK_TIM  ( 3    *TASK_TIM_Unit)
#define     ZIGBEE_TASK_TIM          ( 30    *TASK_TIM_Unit)
#define     CUSUI_TASK_TIM           ( 100   *TASK_TIM_Unit)
#define     SONIC_TASK_TIM           ( 5     *TASK_TIM_Unit) 

typedef enum
{
  Defense_SPIN  = 1,   //基础陀螺
  Defense_BIG_DODGE ,   //大圈陀螺
  Defense_SMALL_DODGE ,  //变速小陀螺
}Defense_Mode_t;

/*系统整车运动状态标志位*/
typedef struct 
{
  bool IF_SPIN_START;//小陀螺开关
  bool IF_CALIBRATE_OK;//初始回正状态
  bool IF_IMU_InitOver;//IMU初始化完成
}CAR_ACT_FLAG_t;

/* 系统本机状态枚举 */
typedef enum 
{
	SYSTEM_RCERR	 = 0,   //遥控错误模式
	SYSTEM_NORMAL  = 1,   //系统正常模式
	SYSTEM_RCLOST	 = 2,   //遥控失联模式
}System_State_t;

typedef struct
{
  bool CAN1_Normal;
  bool CAN2_Normal;
  bool Remote_Normal;
  bool Referee_Normal;
  bool DUM_Normal;
}User_state_t;

/* 系统控制状态枚举 */
typedef enum
{
  Ctrl_Err = 0,
  RC_CTRL_MODE = 1,     //遥控控制模式
  KEY_CTRL_MODE = 2,    //键盘控制模式
  CTRL_MODE_CNT ,

}System_Ctrl_Mode_t;

/* 系统PID模式枚举 */
typedef enum
{
  PID_Err = 0,
	MECH = 1,             //机械模式
	GYRO = 2,             //陀螺仪模式
  PID_MODE_CNT ,
  
}System_Pid_Mode_t;

/* 系统运动行为枚举 */
typedef enum
{
  ACT_Err = 0,
  SYS_ACT_NORMOL = 1,   //普通模式
  SYS_ACT_BIG_BUFF,     //大符模式
  SYS_ACT_SMALL_BUFF,   //小符模式
  SYS_ACT_AUTO_AIM,     //自瞄模式
  SYS_ACT_SPEED,        //极速模式
  SYS_ACT_PARK,         //对位模式
  ACT_MODE_CNT,
}System_Action_t;

typedef struct
{
  float time[30];//用于测量时间的函数，测得结果
  int cal_time ;
}time_cal_t;//用于测量时间

/* 系统信息结构体 */
typedef struct 
{
  System_State_t state;
  System_Ctrl_Mode_t ctrl_mode;
  System_Action_t action_mode;
  System_Pid_Mode_t pid_mode;
  CAR_ACT_FLAG_t car_actFLAG;
  User_state_t user_state;
  Defense_Mode_t Defense_Mode;
  time_cal_t TimeCal;
}Sys_Info_t;


/* 系统信息需要引出接口供各个本地模块查看 */
extern Sys_Info_t System;

void Loop(void);
void System_Init(void);
void IF_First_GetInto_Normal(void);
System_Action_t Vision_Handler(void);
#endif


