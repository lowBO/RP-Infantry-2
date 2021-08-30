#ifndef __CUSUI_H
#define __CUSUI_H

#include "system.h"


typedef struct
{
  uint8_t SPIN;
  uint8_t CLIP;
  uint8_t AUTO;
  uint8_t BlockMuch;
  uint8_t Shoot_heat_limit;
  uint8_t Bullet_Warning;
  float Vcap_show;

}User_CMD_t;


typedef struct
{
  bool IF_Init_Over;
  User_CMD_t User;
}UI_Info_t;

void Startjudge_task(void);


#endif


