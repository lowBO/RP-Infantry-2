#ifndef __RIFLE_H
#define __RIFLE_H


#include "Driver.h"

/*单发一格的角度*/
#define SINGLE_ANGLE   -36859.5
/*连发拨盘的转速*/
#define CONTIN_FREQ_20  20
#define CONTIN_FREQ_15  15
#define CONTIN_FREQ_12  12
#define CONTIN_FREQ_10  10  //射频  （发/秒）
#define CONTIN_FREQ_05   5
#define CONTIN_FREQ_03   3

#define MOTOR_1RPS_SPEED     2160          //60*36一秒转一圈1RPM*60 *36(1：36减速比)= 1RPS
#define GRID_NUM    8    //格子数
#define MOTOR_1GPS_SPEED   (MOTOR_1RPS_SPEED/GRID_NUM)//一秒转一格GPS: Grid Per Second 

#define MOTOR_SPEED_20  (CONTIN_FREQ_20 * MOTOR_1GPS_SPEED)   //固定射频的连发电机转速
#define MOTOR_SPEED_15  (CONTIN_FREQ_15 * MOTOR_1GPS_SPEED)   //固定射频的连发电机转速
#define MOTOR_SPEED_10  (CONTIN_FREQ_10 * MOTOR_1GPS_SPEED)   //固定射频的连发电机转速
#define MOTOR_SPEED_05  (CONTIN_FREQ_05 * MOTOR_1GPS_SPEED)   //固定射频的连发电机转速
#define MOTOR_SPEED_03  (CONTIN_FREQ_03 * MOTOR_1GPS_SPEED)   //固定射频的连发电机转速


/* 判断发射完成的角度误差容忍度 */
#define Judge_AngleErr  600//800
#define Judge_SinBlock_TIM 500  //0.7S  检测单发卡弹 最低300
#define Judge_ConBlock_TIM 250   //0.4S检测连发卡弹 150

/*堵转检测*/
#define Judge_Stuck_SpeedMAX  520 //500
#define Judge_Stuck_OutputMIN  800 //400

/*摩擦轮开启延时*/
#define FricNormal_Delay    700

/*提醒补弹的触发打弹量*/
#define ShootWarningNum     140

/*提醒卡弹次数过多的卡弹量*/
#define Block_WarningNUM    2

/*单发热量限制*/
#define SingleHeatLimitErr    20
#define ContinHeatLimitErr    20

/*连发热量限制*/

/* 拨盘电机数组下标 */
typedef enum 
{
  RIFLE = 0,
  RIFLE_MOTOR_CNT = 1,
}RIFLE_Motor_cnt_t;

typedef enum
{
  NO_Block = 0,
  SinBlock ,
  ConBlock ,
}Block_type_t;

typedef enum
{
  ID_Left = 1,
  ID_Righ = -1,
}Barrel_ID_t;

typedef struct
{
  bool IF_FireBlock ; //通过这个标志位分开控制
  bool BlockReact_Over;
  int cnt;
  Block_type_t type;
}BlockState_t;

typedef struct
{
  Motor_Info_t motor_info;
  Motor_Data_t motor_data;
  PID_Info_t PID;
  BlockState_t BlockState;
}RIFLE_Motor_t;

typedef struct
{
  bool IF_SinFire_Request; //单发请求位
  bool IF_ConFire_Request; //连发请求位
  bool IF_SingleFire_OVER; //单发结束标志位
  bool IF_SinFire_Block;
  bool IF_ConFire_Block;
  bool IF_Block;
  bool IF_Stuck;
}Flag_t;

/* 拨盘信息结构体 */
typedef struct
{
  System_Ctrl_Mode_t ctrl_mode;
  System_Pid_Mode_t pid_mode;
  System_Action_t action_mode;
  PID_type_t PID_type;  
  RIFLE_Motor_t Motor[RIFLE_MOTOR_CNT];
  Flag_t flag;
  float SingleAngle;//单发角度
  float ContinuFreq;//射频
  Barrel_ID_t Barrel_ID; 
}RIFLE_Info_t;

extern RIFLE_Info_t Rifle;

void RIFLE_Ctrl(void);
void RIFLE_GET_Info(void);
void RIFLE_SetPID(void);
void RIFLE_PID_Switch(PID_Info_t *str );
void RIFLE_GET_PID_Type(PID_Info_t *str);
void RIFLE_RC_Ctrl(void);
void RIFLE_KET_Ctrl(void);
void RIFLE_MECH_RC_Ctrl(void);
void RIFLE_GYRO_RC_Ctrl(void);
void Keyboard_Ctrl(void);

void FlagUpdate(void);
void SingleFire(void);
void ContinFire(void);
void RIFLE_MotorAngleInit(void);
void RIFLE_Output(void);

void RIFLE_Reset(void);
void FlagReset(void);

bool Judge_IF_SingleFire_OVER(void);
bool Judge_FireBlock(void);

void RIFLE_Stop(void);
void RIFLE_Handler(void);

void SingleFireRequest(void);
bool SinFireRequest_Report(void);
bool ConFireRequest_Report(void);

void ContinFire_ON(void);
void ContinFire_OFF(void);

void BlockReact(void);
bool Judge_SinFireBlock(void);
bool Judge_ConFireBlock(void);
void BlockState_Reset(void);
bool IF_FireBlock(void);
bool Stuck_Judge(void);

void Single_Forward(void);
void Single_Forward_HALF(void);
void Single_Reverse(void);
void Single_Reverse_HALF(void);

void ContinFreq_20(void);
void ContinFreq_15(void);
void ContinFreq_12(void);
void ContinFreq_10(void);
void ContinFreq_05(void);
void ContinFreq_03(void);

//热量限制
bool Judge_IF_SingleHeat_ShootAllow(void);
bool Judge_IF_Heat_ShootAllow(void);//与下一个函数反相
bool Report_IF_ShootHeatLimit(void);//与上一个函数反相

void Brust_ON(void);
void Brust_OFF(void);

void Check_IF_FricSpeed_Normal(void);
bool Report_IF_FricSpeed_Normal(void);

void Reset_Block_num(void);
bool Judge_IF_SinBlocks(void);
bool Report_IF_SinBlocks(void);

void Reset_BulletWarning(void);
bool Judge_IF_BulletWarning(void);
bool Report_IF_BulletWarning(void);

//双枪管切换
void Judge_Barrel_ID(void);
Barrel_ID_t Get_Barrel_ID(void);
void Set_IF_SELF_CHANGE_Barrel_left(void);
void Reset_IF_SELF_CHANGE_Barrel_left(void);
bool Report_IF_SELF_CHANGE_Barrel_left(void);
void Set_IF_SELF_CHANGE_Barrel_righ(void);
void Reset_IF_SELF_CHANGE_Barrel_righ(void);
bool Report_IF_SELF_CHANGE_Barrel_righ(void);

float AutoAim_Barrelturn(void);

#endif

