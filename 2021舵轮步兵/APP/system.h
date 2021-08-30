#ifndef __SYSTEM_H
#define __SYSTEM_H


typedef enum {
	NONE_ERR,		// 正常(无错误)
	DEV_ID_ERR,		// 设备ID错误
	DEV_INIT_ERR,	// 设备初始化错误
	DEV_DATA_ERR,	// 设备数据错误
} dev_errno_t;

typedef enum {
	DEV_ONLINE,
	DEV_OFFLINE,
} dev_work_state_t;


/*上下主控*/
#define MASTER_DOWN  0
#define MASTER_UP    1
/*摩擦轮电机类型*/
#define Fric_3508   0
#define Fric_Bee    1


////1：4号    2：老步兵  3：3号

///*步兵编号*/
//#define Infantry  3

/*上下主控选择*/ //1主机：上主控   0从机：下主控
#define  Master     MASTER_UP
//摩擦轮是否用裁判系统的射速信息
#define   IF_REFER_CONNECT   1        //裁判系统是否有装上
//下主控是否有连接
#define IF_MASTER_DOWN_CONNECT   1


  #define Fric_motor  Fric_Bee  
  #if (Master == MASTER_UP)
    #define NEW_CTRL  1 //新主控
    /*选定imu的类型*/
    #define MPU6050     0
    #define BMI270      1
    #define BMI_Kp      2.f
  #elif (Master == MASTER_DOWN)
    #define NEW_CTRL  1 //新主控
    /*选定imu的类型*/
    #define MPU6050 0
    #define BMI270  1
    #define BMI_Kp  14.f 
    #define BMI_Kp_2  2.f
  #endif




#define YES true
#define NO  false

#include <stdio.h>
#include "stdbool.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx.h"
#include "stdint.h"
#include "sys.h" 
#include "usart.h"
#include "string.h"

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "myiic.h" 
#include "mpu6050.h"

#include "bmi270.h"
#include "bmi.h"

#include "can1.h"
#include "can2.h"
#include "led.h"
#include "uart4.h"
#include "uart3.h"
#include "uart1.h"

#include "crc.h"
#include "SinCosTable.h"
#include "kalman.h"
#include "arm_math.h"
#include "referee.h"

#include "iwdg.h"
#include "TIM.h"

/**
 * @brief 
 * @param 
 */
extern volatile uint32_t sysTickUptime;

#define abs(x) ((x)>0? (x):(-(x)))       //绝对值宏定义

/**
 * @@@@辅助函数
 */
float constrain(float amt,float high,float low);
float anti_constrain(float amt,float high,float low);
float Slope(float M ,float *queue ,uint16_t len);

uint32_t micros(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

uint32_t millis(void);
void systemInit(void);
void Time_Init(void);

void Single_time_cal(void);
bool Judge_IF_NAN(float x );
#endif


