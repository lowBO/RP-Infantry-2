#ifndef __AUTO_H
#define __AUTO_H

#include "system.h"

#define  OmigaSlope_LEN   5   //滑动滤波长度
#define  AngleSlope_LEN   5   //滑动滤波长度
#define  VErrSlope_LEN    1   //视觉误差滑动滤波  5


#define  PIT_FIX       0//-78
#define  AutoAim_Ke          0.4
#define  ShootSpeed_15_PRE   10.5
#define  ShootSpeed_18_PRE   9.5
#define  ShootSpeed_30_PRE   8.7


typedef enum
{
  YAW = 1,
  PIT = 2,
}AxisType_t;

typedef struct
{
  float Ke,Ko,Ka;
}AutoAim_Param_t;

typedef struct
{
  AxisType_t Axis;
  float abso_Angle;
  float Gim_Err;
  float Vision_Err;
  float Vision_Err_MAX;
  float fix;
}Coordinate_t;


typedef struct
{
  extKalman_t Angle_KF;
  extKalman_t Omiga_KF;
  extKalman_t Accel_KF;
  extKalman_t Out_KF;
}KF_t;


typedef struct
{
  int SinFireAllow_cnt;
  int ConFireAllow_cnt;

}AutoShoot_t;

typedef struct
{
  int start_predict_TIM;
  bool START_PREDICT;
}Predict_t;

typedef struct
{
  float Omiga_queue[OmigaSlope_LEN];
  float Angle_queue[AngleSlope_LEN];
  KF_t KF;                          //卡尔曼
  AutoAim_Param_t Param;            //参数
  Coordinate_t Coordinate;          //初始建立坐标系的角度
  float Angle;                      //角度  （坐标系的角度其实就是误差）
  float prev_Angle;
  float Omiga;                      //角速度
  float prev_Omiga;
  float Accel;                      //角加速度
  
  float Angle_Out;//角度输出
  float Omiga_Out;//速度输出
  float Accel_Out;//角速度输出

  float Out;//总输出
  Predict_t Predict;
  float TargerFix;
  AutoShoot_t AutoShoot;
}AutoAim_Data_t;


typedef struct
{
  bool Init_Over;
  AutoAim_Data_t YAW;
  AutoAim_Data_t PIT;
}AutoAim_t;


extern bool YawAutoAim_Data_Update;
extern bool PitAutoAim_Data_Update;
extern AutoAim_t AutoAim;

void SlopeData_Reset(void);
void AutoAim_ON(void);
void AutoAim_OFF(void);
void AutoAimInit(void);
float GetAngle(Coordinate_t *str);
float AutoAim_Algorithm(AutoAim_Data_t *str);
float Get_SlopeTarget(float M ,float *queue ,uint16_t len);
void Predict_Reset(void);//预测复位，自瞄跟随等待一段时间后开启预测
void VErrSlope_Reset(void);//视觉误差滑动滤波数组清0

float AutoAimYaw_Ctrl(void);
float AutoAimPit_Ctrl(void);

void Yaw_AutoShoot(void);//自动打弹控制

bool Report_IF_Auto_InitOver(void);//反馈是否启动自瞄

void AutoAim_Trigger(void);//硬触发

void PIT_AUTO_ADD(void);
void PIT_AUTO_DEC(void);
float Report_PIT_AUTO_NUM(void);

#endif

