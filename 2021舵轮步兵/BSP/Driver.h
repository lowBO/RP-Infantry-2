/**
 * 
 *           _____                    _____                    _____                    _____          
 *          /\    \                  /\    \                  /\    \                  /\    \         
 *         /::\____\                /::\    \                /::\    \                /::\    \        
 *        /:::/    /                \:::\    \              /::::\    \              /::::\    \       
 *       /:::/    /                  \:::\    \            /::::::\    \            /::::::\    \      
 *      /:::/    /                    \:::\    \          /:::/\:::\    \          /:::/\:::\    \     
 *     /:::/____/                      \:::\    \        /:::/__\:::\    \        /:::/__\:::\    \    
 *    /::::\    \                      /::::\    \      /::::\   \:::\    \      /::::\   \:::\    \   
 *   /::::::\    \   _____    ____    /::::::\    \    /::::::\   \:::\    \    /::::::\   \:::\    \  
 *  /:::/\:::\    \ /\    \  /\   \  /:::/\:::\    \  /:::/\:::\   \:::\____\  /:::/\:::\   \:::\    \ 
 * /:::/  \:::\    /::\____\/::\   \/:::/  \:::\____\/:::/  \:::\   \:::|    |/:::/__\:::\   \:::\____\
 * \::/    \:::\  /:::/    /\:::\  /:::/    \::/    /\::/   |::::\  /:::|____|\:::\   \:::\   \::/    /
 *  \/____/ \:::\/:::/    /  \:::\/:::/    / \/____/  \/____|:::::\/:::/    /  \:::\   \:::\   \/____/ 
 *           \::::::/    /    \::::::/    /                 |:::::::::/    /    \:::\   \:::\    \     
 *            \::::/    /      \::::/____/                  |::|\::::/    /      \:::\   \:::\____\    
 *            /:::/    /        \:::\    \                  |::| \::/____/        \:::\   \::/    /    
 *           /:::/    /          \:::\    \                 |::|  ~|               \:::\   \/____/     
 *          /:::/    /            \:::\    \                |::|   |                \:::\    \         
 *         /:::/    /              \:::\____\               \::|   |                 \:::\____\        
 *         \::/    /                \::/    /                \:|   |                  \::/    /        
 *          \/____/                  \/____/                  \|___|                   \/____/  
 */
#ifndef __DRIVER_H
#define __DRIVER_H

#include "GO.h"
 
/**
 * @brief ??????-----------------------------
 * @param 
 */
/* ???????????? */
typedef enum 
{
  RM_3508 = 0,
  GM_6020_YAW ,
  GM_6020_PIT ,
  M_2006  ,
  FRIC_3508 ,
  GM_6020_RUDDER ,
  MOTOR_TYPE_CNT  ,
}Motor_Type_t;

/* ????????ID???? */
typedef enum 
{ 
  CHAS_ESC   = 0X200,   //????????ID  3508
  CHAS_LF_ID = 0x201,   //??+?? 
  CHAS_RF_ID = 0x202,   //??+??
  CHAS_LB_ID = 0x203,   //??+??
  CHAS_RB_ID = 0x204,   //??+??
}CHAS_MOTOR_ID;

/* ????????ID???? */
typedef enum 
{
  GIM_YAW_ID = 0x205,   //YAW
  GIM_PIT_ID = 0x206,   //PITCH
  GIM_ESC    = 0x1FF,   //????????ID  6020
}GIM_MOTOR_ID;


/* ????????ID???? */
typedef enum
{
  RUD_LF_ID = 0x205,   //??+??
  RUD_RF_ID = 0x206,   //??+??
  RUD_LB_ID = 0x207,   //??+??
  RUD_RB_ID = 0x208,   //??+??
  RUD_ESC   = 0x1FF,   //????????ID  6020
}RUD_MOTOR_ID;


/* ????????ID???? */
typedef enum 
{
  RIFLE_ID  = 0x201,   //????????1??
  RIFLE_ESC = 0x200,
}RIFLE_MOTOR_ID;

/* ??????????ID???? */
typedef enum 
{
  Fric_3508_ESC = 0x200,
  Fric_L_ID     = 0x201,   
  Fric_R_ID     = 0x202,
}FRIC_MOTOR_ID;

/*??????????????*/
typedef struct 
{
  Motor_Type_t motor_type;
}Motor_Info_t;

/*????CAN???????????? ??????*/
typedef __packed struct 
{
  int16_t Motor_Speed;
  int16_t Motor_Angle;
  int16_t Motor_ELC;
  uint8_t temp;   
}CAN_GET_DATA_t;

/*????IMU???????????? ??????*/
typedef __packed struct
{
  float IMU_Speed;
  float IMU_Angle;
}IMU_GET_DATA_t;


/*????????????????*/
typedef struct
{
  CAN_GET_DATA_t CAN_GetData;
  IMU_GET_DATA_t IMU_GetData;
  /* ??????????pid???? */
  float PID_Speed;
  float PID_Speed_target;
  float PID_Angle; 
  float PID_Angle_target; 
}Motor_Data_t;



/**
 * @brief PID??-----------------------------
 * @note  ?????????????? 
 */
/*PID????*/
typedef enum
{
  Turn_off = 0,
  RC_MECH = 1,
  RC_GYRO = 2,
  KEY_MECH = 3,
  KEY_GYRO = 4,
  ACT_BIG_BUFF,
  ACT_SMALL_BUFF,
  ACT_AUTO_AIM ,
  ACT_SPEED ,
  ACT_PARK ,
  PID_TYPE_CNT ,
}PID_type_t;

/* PID???? */
typedef __packed struct 
{
  float P,I,D;
}PID_Parameter_t;


/* PID?????? */
typedef __packed struct
{
  PID_Parameter_t PID_Param;
  float Err;
  float Last_Err;
  float I_Accu_Err;
  float I_Accu_Err_Max;
  float PID_P_Out;
  float PID_I_Out;
  float PID_D_Out;
  float PID_Output;
  float PID_Err_Max;
  float PID_Err_Dead;
  float PID_Output_Max;
  float PID_I_Out_Max;
  float PID_P_Out_Max;
}PID_Loop_t;

/*PID??????????????*/
typedef __packed struct 
{
  PID_Loop_t Speed_Loop;
  PID_Loop_t Angle_Loop;
  PID_type_t PID_type;
}PID_Info_t;

/* PID???????????? */
extern PID_Parameter_t PID_Speed_Param[MOTOR_TYPE_CNT][PID_TYPE_CNT];
extern PID_Parameter_t PID_Angle_Param[MOTOR_TYPE_CNT][PID_TYPE_CNT];


#endif


