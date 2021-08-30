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
 * @brief 电机区-----------------------------
 * @param 
 */
/* 电机类型枚举 */
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

/* 底盘电机ID信息 */
typedef enum 
{ 
  CHAS_ESC   = 0X200,   //底盘电调ID  3508
  CHAS_LF_ID = 0x201,   //左+前 
  CHAS_RF_ID = 0x202,   //右+前
  CHAS_LB_ID = 0x203,   //左+后
  CHAS_RB_ID = 0x204,   //右+后
}CHAS_MOTOR_ID;

/* 云台电机ID信息 */
typedef enum 
{
  GIM_YAW_ID = 0x205,   //YAW
  GIM_PIT_ID = 0x206,   //PITCH
  GIM_ESC    = 0x1FF,   //云台电调ID  6020
}GIM_MOTOR_ID;


/* 舵轮电机ID信息 */
typedef enum
{
  RUD_LF_ID = 0x205,   //左+前
  RUD_RF_ID = 0x206,   //右+前
  RUD_LB_ID = 0x207,   //左+后
  RUD_RB_ID = 0x208,   //右+后
  RUD_ESC   = 0x1FF,   //舵轮电调ID  6020
}RUD_MOTOR_ID;


/* 拨盘电机ID信息 */
typedef enum 
{
  RIFLE_ID  = 0x201,   //电调闪灯1下
  RIFLE_ESC = 0x200,
}RIFLE_MOTOR_ID;

/* 摩擦轮电机ID信息 */
typedef enum 
{
  Fric_3508_ESC = 0x200,
  Fric_L_ID     = 0x201,   
  Fric_R_ID     = 0x202,
}FRIC_MOTOR_ID;

/*电机信息结构体*/
typedef struct 
{
  Motor_Type_t motor_type;
}Motor_Info_t;

/*电机CAN接收到的数据 结构体*/
typedef __packed struct 
{
  int16_t Motor_Speed;
  int16_t Motor_Angle;
  int16_t Motor_ELC;
  uint8_t temp;   
}CAN_GET_DATA_t;

/*电机IMU接收到的数据 结构体*/
typedef __packed struct
{
  float IMU_Speed;
  float IMU_Angle;
}IMU_GET_DATA_t;


/*电机总数据结构体*/
typedef struct
{
  CAN_GET_DATA_t CAN_GetData;
  IMU_GET_DATA_t IMU_GetData;
  /* 参与计算的pid数据 */
  float PID_Speed;
  float PID_Speed_target;
  float PID_Angle; 
  float PID_Angle_target; 
}Motor_Data_t;



/**
 * @brief PID区-----------------------------
 * @note  不能共存的模式 
 */
/*PID类型*/
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

/* PID参数 */
typedef __packed struct 
{
  float P,I,D;
}PID_Parameter_t;


/* PID环数据 */
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

/*PID双环数据结构体*/
typedef __packed struct 
{
  PID_Loop_t Speed_Loop;
  PID_Loop_t Angle_Loop;
  PID_type_t PID_type;
}PID_Info_t;

/* PID参数外调汇总 */
extern PID_Parameter_t PID_Speed_Param[MOTOR_TYPE_CNT][PID_TYPE_CNT];
extern PID_Parameter_t PID_Angle_Param[MOTOR_TYPE_CNT][PID_TYPE_CNT];


#endif


