#ifndef __GIMBAL_H
#define __GIMBAL_H

#include "Driver.h"

#define GIM_Calibrate_TIM    2    //回正时间           单位为 秒
#define GIM_CalibrateOK_TIM  0.1  //判断回正完成时间   单位为 秒
#define CHANGE_Unit   (FreeRTOS_1S_Unit/GIMBAL_TASK_TIM) //换算单位
#define GIM_Calibrate_CNT    (GIM_Calibrate_TIM *CHANGE_Unit)
#define GIM_CalibrateOK_CNT  (GIM_CalibrateOK_TIM *CHANGE_Unit)
#define Calibrate_AngleErr  3   //判断回正完成允许的最大误差

#define Motor_90          2048
#define Motor_180         4096
#define Motor_360         8192

/*IMU*/

#define Turn_HalfMove     680
#define Turn_45           1024
#define Turn_90           2048
#define Turn_180          4096
#define Turn_360          8192

#define IMU_180           Turn_180
#define IMU_360           Turn_360


/* 通过最大误差限制回正速度 */

#define Yaw_CalErrLimit    250

/* 云台电机数组下标 */
typedef enum 
{
  GIM_YAW = 0,
  GIM_PIT = 1,
  GIM_MOTOR_CNT = 2,
}GIM_Motor_cnt_t;

/* 回正状态 */
typedef enum 
{
  Calibrating = 0,
  NO_Cal ,
  Cal_OK ,
}Calibrate_State_t;

/* 半角移动模式 */
typedef enum
{
  Normal_yaw = 0,
  LF_yaw ,
  RF_yaw ,
  L_yaw , 
  R_yaw ,
}Car_HalfMove_type_t;


typedef struct
{
  float AXIS_Angle_Inc;
  float AXIS_Angle_k;//角度增加的幅度调节因子
  float Slow_Inc;
}GIM_Move_t;

typedef struct
{
  bool IF_Turn_90;
  bool IF_Turn_180;
  int cnt;
}Car_Turn_t;

typedef struct
{
  bool IF_TurnReverse;
  bool IF_ReverseOver;
  int cnt;
  int Over_cnt;
}Gim_Reverse_t;

typedef struct
{
  Car_Turn_t Left;
  Car_Turn_t Righ;
  Gim_Reverse_t GIM_Reverse;
}Turn_Info_t;



typedef struct
{
  int ING_cnt;
  int OK_cnt;
  Calibrate_State_t State;
}Calibrate_Info_t;

typedef struct
{
  Motor_Info_t motor_info;
  Motor_Data_t motor_data;
  PID_Info_t PID;
  
  GIM_Move_t RC_Move;
  GIM_Move_t Key_Move;
  //角度环卡尔曼
  extKalman_t KF_Angle[PID_TYPE_CNT];
  
  float MECH_Mid_Angle;
  float (*Kalman)(extKalman_t* p,float data);
  
  Calibrate_Info_t Calibrate_Info;
}AXIS_Info_t;

/* 云台信息结构体 */
typedef struct
{
  System_Ctrl_Mode_t ctrl_mode;
  System_Pid_Mode_t pid_mode;
  System_Action_t action_mode;
  PID_type_t PID_type;  
  AXIS_Info_t YAW;
  AXIS_Info_t PIT;
  Calibrate_State_t GIM_Cal_state;
  bool IF_NAN;
  bool IF_CALIBRATE_OK;
  bool IF_LRDenfense_START;
  Turn_Info_t Turn_Info;
  Defense_Mode_t Defense_Mode;
  Car_HalfMove_type_t HalfMove_type;
}GIM_Info_t;




extern GIM_Info_t Gimbal;
void GIM_AXIS_SetPID_type(void);
void GIM_SetPID(void);
void GIM_PID_Switch(AXIS_Info_t *strAxis);
void GIMBAL_Ctrl(void);
void GIM_GET_Info(void);

void GIM_RC_Ctrl(void);
void GIM_KET_Ctrl(void); 

void YAW_MECH_Mid_Angle_Proc(void);
float YAW_MotorAngle_Proc(int16_t Angle);
void GIM_YawDataSync_GYRO(Motor_Data_t *str);
void GIM_YawDataSync_MECH(Motor_Data_t *str);
void GIM_PitchDataSync(Motor_Data_t *str);
void GIM_PitchDataSync_GYRO(Motor_Data_t *str);

float GIM_GetOutput(AXIS_Info_t *str);
void GIM_Output(void);

void GIM_Reset(void);
void GIM_SetEmpty(PID_Info_t *str);
void GIM_Stop(void);

void IMU_Data_Report(void);

void GIM_GYRO_FirstYawAngle(AXIS_Info_t *str);
void GIM_MECH_FirstYawAngle(AXIS_Info_t *str);
void GIM_FirstPitAngle(AXIS_Info_t *str); 
void GIM_MECH_FirstPitAngle(AXIS_Info_t *str);
void GIM_GYRO_FirstPitAngle(AXIS_Info_t *str);

void GIM_Remote_Ctrl(void);
void Remote_Ctrl_YAW(AXIS_Info_t *str);
void Remote_Ctrl_PIT(AXIS_Info_t *str);

void GIM_Keyboard_Ctrl(void);
void Keyboard_Ctrl_YAW(AXIS_Info_t *str);
void Keyboard_Ctrl_PIT(AXIS_Info_t *str);

float GIM_Get_AngleErr(AXIS_Info_t *str);
float GIM_Get_SpeedErr(AXIS_Info_t *str);

float YAW_AngleErrReport(void);
float PIT_AngleErrReport(void);
float YAW_GetGyroAngle(void);
float PIT_GetGyroAngle(void);
float PIT_GetMechAngle(void);


void GIM_Calibrate(void);//回正控制函数
bool GIM_CalOK_Report(void);
void GIM_Cal_StateSync(Calibrate_State_t state);
void Start_GIM_Calibrate(void);//回正状态开启函数，调用后可进入回正状态


//快速转向：
void TurnLeft_90(void);
void TurnRigh_90(void);
void TurnLeft_180(void);
void TurnRigh_180(void);
void TurnLeftReset(void);
void TurnRighReset(void);
void TurnReset(void);
void Judge_IF_TurnOver(void);

//整车斜着走
void UserSet_W_move(bool a);
void UserSet_S_move(bool a);
void UserSet_A_move(bool a);
void UserSet_D_move(bool a);
void HalfMove_KeyReset(void);
void Judge_HalfMove_type(void);
void Reset_IF_MECH_StopHalfMove(void);
void Set_IF_MECH_StopHalfMove(void);
void Reset_IF_SPIN_StopHalfMove(void);
void Set_IF_SPIN_StopHalfMove(void);
void Reset_IF_CarHalfMove(void);
void Set_IF_CarHalfMove(void);
void Reset_CarHalfMove(void);//斜向模式总复位，初始状态为关闭
void IF_CarHalfMove_Switch(void);//跳变开关
void SystemSet_IF_CarHalfMove(void);
float Get_CarHalfMove_Angle(void);

//上报NAN
bool GIM_Report_IF_NAN(void);

//击打哨兵快速抬头
void Gim_Sentry_UP(void);

//头部方向
void Judge_DIR(void);
void CHAS_Turn_180(void);
void GimReverseTurn(void);
void Judge_IF_ReverseOver(void);
bool Report_IF_Reverse(void);

#endif
