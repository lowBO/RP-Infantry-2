#ifndef __CHASSIS_H
#define __CHASSIS_H




#include "Driver.h"


#define CHAS_PARK_FB_k  3.7
#define CHAS_PARK_LR_k  3.7
#define CHAS_PARK_Z_k   1

#define   SPIN_FBk_50W   10
#define   SPIN_FBk_60W   10
#define   SPIN_FBk_80W   11
#define   SPIN_FBk_100W  11
#define   SPIN_FBk_120W  12
#define   SPIN_FBk_SUPER  12
//#define   SPIN_LRk_50W   4
//#define   SPIN_LRk_60W   4.5
//#define   SPIN_LRk_80W   5.5
//#define   SPIN_LRk_100W  6.5
//#define   SPIN_LRk_120W  7.5
//#define   SPIN_LRk_SUPER  12

#define   RC_CHAS_GYRO_FB_k   8
#define   RC_CHAS_GYRO_LR_k   8
#define   RC_CHAS_GYRO_Z_k    1
#define   RC_CHAS_MECH_FB_k   13.2
#define   RC_CHAS_MECH_LR_k   13.2//4
#define   RC_CHAS_MECH_Z_k    10
#define   KEY_CHAS_GYRO_FB_k   13.2
#define   KEY_CHAS_GYRO_LR_k   13.2
#define   KEY_CHAS_GYRO_Z_k    1
#define   KEY_CHAS_MECH_FB_k   13.2
#define   KEY_CHAS_MECH_LR_k   13.2
#define   KEY_CHAS_MECH_Z_k    70

#define  CHAS_45W_K      5+1.5
#define  CHAS_50W_K      5.5+1.5
#define  CHAS_55W_K      6+1.5
#define  CHAS_60W_K      6.5+1.5
#define  CHAS_80W_K      7.5+1.5
#define  CHAS_100W_K     8.3+1.5
#define  CHAS_120W_K     10+1.5
#define  CHAS_SC_K       11+1.5



#define   CHAS_Z_SPEED_MAX  11000  //11*660 = 7260 限7500
#define   CHAS_MOTOR_SPEED_MAX  13000//10000

//功率限制
#define   ANCESTRAL_ALGORITHM  YES //是否是祖传算法
#define   CHAS_TOTAL_OUTPUT_MAX   50000    //底盘最大总输出 50000
#define   CHAS_LIMIT_POWERBUFFER  60

//每个轮子的最大转速
 //之前是6000   是用于分配速度比例的最大值，跑起来不一定是这个值，因为有功率限制
#define Car_SPEED   CHAS_MOTOR_SPEED_MAX 


/* 底盘电机数组下标 */
typedef enum 
{
  CHAS_LF = 0,
  CHAS_RF = 1,
  CHAS_LB = 2,
  CHAS_RB = 3,
  CHAS_MOTOR_CNT = 4,
}CHAS_Motor_cnt_t;

typedef struct 
{
  extKalman_t KF_GYRO_Angle;
  PID_Info_t GYRO_Ctrl;  
}CHAS_GYRO_t;//陀螺仪模式底盘跟随云台的数据结构体

typedef struct 
{
  float FB_Speed,LR_Speed,Z_Speed ;
  float FB_Speed_k,LR_Speed_k,Z_Speed_k; //遥控->速度 变化的幅度调节因子
  float Motor_SpeedTarget[CHAS_MOTOR_CNT];
  float Slow_Inc;
  CHAS_GYRO_t GYRO_Move;
}CHAS_Move_t;

typedef struct 
{
  Motor_Data_t motor_data;
  PID_Info_t PID;
}CHAS_Motor_t;

typedef struct
{
  float Limit_k;
  float Real_PowerBuffer;
  float Max_PowerBuffer;
  float CHAS_TotalOutput;
  float CHAS_LimitOutput;
}CHAS_PowerLimit_t;

typedef enum
{
  HEAD = 1,
  TAIL = -1,
}Dir_t;

typedef struct
{
  Dir_t mode;
  float num;
}CHAS_Dir_t;



/* 底盘信息结构体 */
typedef struct
{
  System_Ctrl_Mode_t ctrl_mode;
  System_Pid_Mode_t pid_mode;
  System_Action_t action_mode;  
  PID_type_t PID_type;
  
  Motor_Info_t motor_info;
  CHAS_Motor_t CHAS_Motor[CHAS_MOTOR_CNT];
  
  CHAS_Move_t RC_Move;
  CHAS_Move_t Key_Move;
  CHAS_Move_t SPIN_Move;
  float Speed_MAX;
  bool IF_NAN;
  bool IF_SPIN_START;//小陀螺开关
  bool IF_SPIN_CLOSING;//小陀螺是否正在关闭
  CHAS_PowerLimit_t CHAS_PowerLimit;
  Defense_Mode_t Defense_Mode;
  CHAS_Dir_t Dir;
}CHAS_Info_t;



extern CHAS_Info_t Chassis;
extern float power_limit_K;
float CHAS_Power_Speed_K(void);
void CHAS_SetPID_type(void);
void CHAS_SetPID(void);
void CHAS_PID_Switch(PID_Info_t *str);
void CHAS_Dir_HEAD(void);
void CHAS_Dir_TAIL(void);
void CHAS_Dir_Switch(void);
void CHASSIS_Ctrl(void);
void CHAS_GET_Info(void);
void CHAS_GET_PID_Type(PID_Info_t *str );
void CHAS_KET_Ctrl(void);
void CHAS_RC_Ctrl(void);
void CHAS_Remote_Ctrl(void);
void CHAS_Keyboard_Ctrl(void);

void CHAS_Reset(void);
void CHAS_Stop(void);
void CHAS_SetEmpty(PID_Info_t *str);

void CHAS_SetChasSpeed(CHAS_Move_t *str);
void CHAS_SetMotorSpeed(CHAS_Move_t *str);
void CHAS_DataSync(Motor_Data_t *str);
float CHAS_GetOutput(CHAS_Motor_t *str);
void CHAS_Output(void);
void CHAS_Handler(CHAS_Move_t *str);

float CHAS_Get_AngleErr(Motor_Data_t *str);
float CHAS_Get_SpeedErr(Motor_Data_t *str);
float CHAS_Get_Zerror(CHAS_Move_t *str);
float CHAS_Get_Z_Speed(CHAS_Move_t *MOVE);
  
//功率限制
void power_limit(void);

//小陀螺
float SPIN_GetSpeed_LR(float Angle , float FB_ctrl , float LR_ctrl);
float SPIN_GetSpeed_FB(float Angle , float FB_ctrl , float LR_ctrl);
void CHAS_SPIN(void);
bool CHAS_SPIN_Report(void);
bool CHAS_SPIN_Close_Report(void);
void SPIN_ON(void);
void SPIN_OFF(void);
void SPIN_Reset(void);
void SPIN_KEY_Switch(void);
void SPIN_CLOSING_Judge(void);
void SPIN_Defense_Mode_Switch(void);
Dir_t Report_CHAS_DIR(void);
bool Judge_IF_CHAS_Dir_HEAD(void);
bool Judge_IF_CHAS_Dir_TAIL(void);
//zigbee用 反馈最终输出  
float CHAS_Report_Output_LF(void);
float CHAS_Report_Output_RF(void);
float CHAS_Report_Output_LB(void);
float CHAS_Report_Output_RB(void);
//zigbee用 反馈速度
float CHAS_Report_Speed_LF(void);
float CHAS_Report_Speed_RF(void);
float CHAS_Report_Speed_LB(void);
float CHAS_Report_Speed_RB(void);
//zigbee用 调试陀螺最佳速度
//void SPIN_Speed_up20(void);
//void SPIN_Speed_down20(void);
//float Get_test_spin_speed(void);
uint8_t Report_SuperCap_cmd(void);
void SuperCap_KEY_ON(void);
void SuperCap_KEY_OFF(void);

//上报NAN
bool CHAS_Report_IF_NAN(void);

#endif

