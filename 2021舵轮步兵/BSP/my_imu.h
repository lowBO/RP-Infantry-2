#ifndef __MY_IMU_H
#define __MY_IMU_H

#include "system.h"
#define CALCULATE_LENGTH 3 //计算静差用的数组长度，不可调整
#define JUDGE_ERR   0      //静差判断的允许误差范围，越小越精细
#define SPEED_FILTER_LEN 3 //速度滤波长度

#define prev_Imu_Yaw_LEN   20//记录陀螺仪前n帧数据
#define prev_Imu_Pit_LEN   20//记录陀螺仪前n帧数据

///* IMU的云台抬头低头边界 */ /*MAX和MIN指的是抬头和低头的意思*/
//#if (Infantry == 1)
//#define   PIT_IMU_MAX_ANGLE   154 //PIT低头最大值
//#define   PIT_IMU_MIN_ANGLE   192 //PIT抬头最大值

//#elif (Infantry == 2)
//#define   PIT_IMU_MAX_ANGLE   -20 //PIT低头最大值
//#define   PIT_IMU_MIN_ANGLE   15 //PIT抬头最大值

//#elif (Infantry == 3)
//#endif

typedef struct 
{ 
  bool IF_GET_StaticErr;    //获取静差成功标志
  float YAW_Err_Init;       //陀螺仪YAW静差
  float PIT_Err_Init;       //陀螺仪PIT静差
  short gyrox,gyroy,gyroz;  //陀螺仪反馈角速度
  float IMU_YAW_Speed;      //IMU YAW 速度
  float IMU_YAW_Angle;      //IMU YAW 角度
  float IMU_PIT_Speed;      //IMU PIT 速度
  float IMU_PIT_Angle;      //IMU PIT 角度
  float prev_Imu_Yaw[prev_Imu_Yaw_LEN];
  float prev_Imu_Pit[prev_Imu_Pit_LEN];
}IMU_Info_t;

void IMU_Init(void);
void IMU_Ctrl(void);

bool Static_Error_Calculate(void);
void Speed_Filter(short gx,short gy,short gz);//速度滤波
void IMU_get_angle_to_use(void);          //获取陀螺仪绝对角度

float Report_prev_imu_yaw_angle(int i);
float Report_prev_imu_pit_angle(int i);
float Report_YawSpeed(void);
float Report_YawAngle(void);
float Report_PitSpeed(void);
float Report_PitAngle(void);
bool IF_IMU_InitOver(void);


void Gimbal_Measure_Data(IMU_Info_t *str);

#endif



