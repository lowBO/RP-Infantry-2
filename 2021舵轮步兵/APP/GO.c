/***      《 整车控制代码 》

 *      ┌─┐       ┌─┐ + +
 *   ┌──┘ ┴───────┘ ┴──┐++
 *   │                 │
 *   │       ───       │++ + + +
 *   ███████───███████ │+
 *   │                 │+
 *   │       ─┴─       │
 *   │                 │
 *   └───┐         ┌───┘
 *       │         │
 *       │         │   + +
 *       │         │
 *       │         └──────────────┐
 *       │                        │
 *       │                        ├─┐
 *       │                        ┌─┘
 *       │                        │
 *       └─┐  ┐  ┌───────┬──┐  ┌──┘  + + + +
 *         │ ─┤ ─┤       │ ─┤ ─┤
 *         └──┴──┘       └──┴──┘  + + + +
 *              
 *               代码无BUG!
 */
 

#include "system.h"
#include "Device.h"
#include "Function.h"
#include "FreeRTOS_Init.h"

uint32_t currentTime = 0;

/**
 * @brief 设备层初始化
 * @param 
 */
void Device_Init(void)
{
  /* 常规任务设备层初始化 */
  MODULE_Init();
  MOTOR_Init();
  IMU_Init();
  Kalman_Init();
}

void Function_Init(void)
{
  /* 特殊功能初始化 */
    //视觉
    Vision_Init();
    AutoAimInit();
//    Trigger_Cnt_Init(40-1,8400-1);//硬触发//4ms进入一次定时中断
}
/**
 * @brief 系统初始化函数
 */
void System_Init(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	Delay_init(168);// 1ms Systick
	CRC_init();	
	CAN1_Init();
	CAN2_Init();
  Led_Init();
	usart2_Init();
  UART5_Init();
  UART4_Init();
  USART1_Init();
  USART3_Init(); 
  KEY_Init();
  Device_Init();
  Function_Init(); 

  Time_Init();
  IWDG_Init(4,250); //看门狗初始化：与分频数为64,重载值为250,溢出时间为0.5s	
  FreeRTOS_Init();
}

/* -----------------Init start--------------------- */

/* 系统信息结构体初始化 */
Sys_Info_t System = 
{
  .state = SYSTEM_RCLOST,         //默认失联
  .ctrl_mode = Ctrl_Err,      //默认遥控器控制
  .pid_mode = PID_Err,               //默认机械模式
  .action_mode = ACT_Err,  //默认普通运动方式
  .Defense_Mode = Defense_SPIN,
};

/* ------------------Init end---------------------- */

/**
 * @brief 状态机任务
 * @param 任务间隔：SYSTEM_STATE_TASK_TIM
 */
void SYSTEM_STATE_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    /* 系统喂狗 */
    IWDG_Feed();
    /* 系统状态处理 */
    system_state_Ctrl();
    /* 系统状态灯 */
    System_LED_Hint(Hint_LED);
    /* 系统整车状态同步读取 */
    Car_StateSync(&System.car_actFLAG);
    /* 系统整车外设/驱动状态读取 */
    User_StateSyne(&System.user_state);
    
    #if (Master == MASTER_DOWN)
    /* 下主控陀螺仪Kp调节稳定 */
    Calm_BMI_Kp();
    #endif
    
    vTaskDelayUntil(&currentTime, SYSTEM_STATE_TASK_TIM);//绝对延时
	}
}


/**
 * @brief 云台任务
 * @param 任务间隔：GIMBAL_TASK_TIM
 */
void GIMBAL_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    GIM_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        GIMBAL_Ctrl();
        break;
      case SYSTEM_RCLOST:
        GIM_Reset();
        break;
      case SYSTEM_RCERR :
        GIM_Reset();
        break;
    }
    vTaskDelayUntil(&currentTime, GIMBAL_TASK_TIM);//绝对延时
	}
}

/**
 * @brief 舵轮任务
 * @param 任务间隔：RUDDER_TASK_TIM
 */
void RUDDER_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    RUD_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        RUDDER_Ctrl();
        break;
      case SYSTEM_RCLOST:
        RUD_Reset();
        break;
      case SYSTEM_RCERR :
        RUD_Reset();
        break;
    }
    vTaskDelayUntil(&currentTime, RUDDER_TASK_TIM);//绝对延时
	}
}

/**
 * @brief 底盘任务
 * @param 任务间隔：CHASSIS_TASK_TIM
 */
void CHASSIS_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    CHAS_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        CHASSIS_Ctrl();
        break;
      case SYSTEM_RCLOST:
        CHAS_Reset();
        break;
      case SYSTEM_RCERR :
        CHAS_Reset();        
        break;
    }
    vTaskDelayUntil(&currentTime, CHASSIS_TASK_TIM);//绝对延时
	}
}

/**
 * @brief 常规任务
 * @param 任务间隔：MODULE_TASK_TIM
 */
void MODULE_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    MODULE_GetInfo();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        MODULE_Ctrl();
        break;
      case SYSTEM_RCLOST: 
        MODULE_Reset();
        break;
      case SYSTEM_RCERR:  
        MODULE_Reset();
        break;
    }
    vTaskDelayUntil(&currentTime, MODULE_TASK_TIM);//绝对延时
	}
}
/**
 * @brief 拨盘任务
 * @param 任务间隔：REVOLVER_TASK_TIM
 */
void RIFLE_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    RIFLE_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        RIFLE_Ctrl();
        break;
      case SYSTEM_RCLOST:
        RIFLE_Reset();
        break;
      case SYSTEM_RCERR :
        RIFLE_Reset();
        break;        
    }     
    vTaskDelayUntil(&currentTime, RIFLE_TASK_TIM);//绝对延时
	}
}
/**
 * @brief IMU任务
 * @param 任务间隔：IMU_TASK_TIM  
 */
void IMU_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    IMU_Ctrl();
    
    vTaskDelayUntil(&currentTime, IMU_TASK_TIM);//绝对延时
	}
}

/**
 * @brief 视觉任务
 * @param 任务间隔：VISION_TASK_TIM
 */
float test_tast_tim = 2;
void VISION_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    VISION_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        VISION_Ctrl();
        break;
      case SYSTEM_RCLOST:
        Vision_Reset();
        break;
      case SYSTEM_RCERR :
        Vision_Reset();
        break;
    }  
    vTaskDelayUntil(&currentTime, VISION_TASK_TIM);//绝对延时    
	}
}

/**
 * @brief PID控制器任务
 * @param 任务间隔：PID_TASK_TIM
 */
void PID_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    PID_GET_Info();
    switch (System.state)
    {
      case SYSTEM_NORMAL:
        PID_Ctrl();
        break;
      case SYSTEM_RCLOST:
        PID_Reset();
        break;
      case SYSTEM_RCERR :
        PID_Reset();
        break;
    }    
    vTaskDelayUntil(&currentTime, PID_TASK_TIM);//绝对延时 
	}
}

/**
 * @brief 硬触发任务
 * @param 任务间隔：VISION_TRIGGER_TASK_TIM
 */
int test_task_tim_trig = 4;
void VISION_TRIGGER_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    Trigger();
    //Trigger_5ms();

    vTaskDelayUntil(&currentTime, test_task_tim_trig);//VISION_TRIGGER_TASK_TIM);//绝对延时
  }
}
//定时器5中断服务函数
void TIM5_IRQHandler(void *p_arg)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //溢出中断
	{
//    if(System.action_mode == SYS_ACT_AUTO_AIM)
		  Trigger();
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //清除中断标志位

}



/**
 * @brief 上下主控沟通任务
 * @param 任务间隔：MASTERLINK_TASK_TIM
 */
void MASTERLINK_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    Master_Sent();
    
    vTaskDelayUntil(&currentTime, MASTERLINK_TASK_TIM);//绝对延时
  }
}

/**
 * @brief Zigbee发送任务
 * @param 任务间隔：ZIGBEE_TASK_TIM
 */
void ZIGBEE_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    ZIGBEE_SendData();
    
    vTaskDelayUntil(&currentTime, ZIGBEE_TASK_TIM);//绝对延时
  }
}

/**
 * @brief 图传发送任务
 * @param 任务间隔：CUSUI_TASK_TIM
 */
void CUSUI_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    Startjudge_task();

    vTaskDelayUntil(&currentTime, CUSUI_TASK_TIM);//绝对延时
  }
}

/**
 * @brief 超声波发送任务
 * @param 任务间隔：SONIC_TASK_TIM
 */
void SONIC_TASK(void *p_arg)
{
  uint32_t currentTime;
  for(;;)   
	{	
    currentTime = xTaskGetTickCount();//当前系统时间
    
    Sonic_Ctrl();

    vTaskDelayUntil(&currentTime, SONIC_TASK_TIM);//绝对延时
  }
}






