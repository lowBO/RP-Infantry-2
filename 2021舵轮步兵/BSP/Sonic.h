#ifndef __SONIC_H
#define __SONIC_H

#include "system.h"


///////////////////////////注：此枚举再masters_RTX.c.h中都有调用到
typedef enum
{
  NO_FLY = 0,
  FLY = 1,//前轮起飞(优先级最高)
  
  PARK_FrontGood ,//对位模式中正前方对位完成
  PARK_RighGood ,//对位模式中右边对位完成
  PARK_TurnGood ,//对位模式中转向对位完成
  
  PARK_TurnLeft ,//对位模式中左转
  PARK_TurnRigh ,//对位模式中右转
  
  PARK_MoveRigh ,//对位模式中往右平移
  PARK_MoveLeft ,//对位模式中往左平移
  
  PARK_MoveFront ,//对位模式中往前进
  PARK_MoveBehind ,//对位模式中往后退
  
}Sonic_mode_t;

typedef struct
{
  int16_t time;
  float distance;
  bool IF_Data_Update;
}Sonic_Fly_Info_t;

typedef struct
{
  int16_t FL_TIM,FR_TIM,R_TIM;
  float FL_DIS,FR_DIS,R_DIS;
  bool IF_Data_Update_FL;
  bool IF_Data_Update_FR;
  bool IF_Data_Update_R;
}Sonic_Park_Info_t;

typedef struct
{
  Sonic_Fly_Info_t  Fly_Info;
  Sonic_Park_Info_t Park_Info;
  
  Sonic_mode_t Sonic_mode;
}Sonic_Info_t;

#define  FLY_DIS    200

#define FL_DIS_Standard   100
#define FR_DIS_Standard   100
#define R_DIS_Standard    100

void Sonic_Ctrl(void);

void Sonic_ReadData_Fly(uint8_t *rxBuf);
void Sonic_ReadData_FL(uint8_t *rxBuf);
void Sonic_ReadData_FR(uint8_t *rxBuf);
void Sonic_ReadData_R(uint8_t *rxBuf);

void KS103_FLY_Explore(void);
void KS103_FL_Explore(void);
void KS103_FR_Explore(void);
void KS103_R_Explore(void);

Sonic_mode_t Judge_IF_Fly(void);
Sonic_mode_t Park_Judge_F(void);
Sonic_mode_t Park_Judge_R(void);
Sonic_mode_t Park_Judge_Z(void);

float CHAS_Park_FB_Speed(void);
float CHAS_Park_LR_Speed(void);
float CHAS_Park_Z_Speed(void);



#endif 
