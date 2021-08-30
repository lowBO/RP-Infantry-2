#ifndef __BUFF_H
#define __BUFF_H

#include "system.h"

#define BUFF_AUTOSHOOT_CNT_MAX  250

#define YAW_MID_CO   480
#define PIT_MID_CO   512
#define YAW_TARFIX   0//14//-2//20//越大越往左
#define PIT_TARFIX   0//15.5//29//27//越大越往上
#define YAW_INC_K    0.4
#define PIT_INC_K    0.4

typedef struct
{
  extKalman_t KF;     //卡尔曼
  float Mid_co;
  float TargerFix;
  float TargerErr;
  float inc_k;
}BuffAim_Data_t;



typedef struct
{ 
  bool IF_AUTOSHOOT_ON;
  bool IF_ArmorSwitch;
  int Buff_AutoShoot_cnt;
  int Buff_AutoShoot_cnt_MAX;
  BuffAim_Data_t YAW;
  BuffAim_Data_t PIT;
}BuffAim_t;



extern bool YawBuffAim_Data_Update;
extern bool PitBuffAim_Data_Update;
extern BuffAim_t BuffAim;
void Check_IF_BuffArmorSwitch(void);
float BuffAimYaw_Ctrl(void);
float BuffAimPit_Ctrl(void);

void Buff_AutoShoot_ON(void);
void Buff_AutoShoot_OFF(void);
void Buff_AutoShoot(void);
void BuffAim_ON(void);
void BuffAim_OFF(void);

bool Report_IF_Buff_AutoShoot_ON(void);
void Buff_AutoShoot_CntReset(void);

void YAW_KeyFix_UP(void);
void YAW_KeyFix_DOWN(void);
void PIT_KeyFix_UP(void);
void PIT_KeyFix_DOWN(void);
void YAW_KeyFix_UP_CON(void);
void YAW_KeyFix_DOWN_CON(void);
void PIT_KeyFix_UP_CON(void);
void PIT_KeyFix_DOWN_CON(void);

#endif

