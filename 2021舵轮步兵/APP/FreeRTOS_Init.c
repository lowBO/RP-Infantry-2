#include "FreeRTOS_Init.h"


static u8  fac_us=0;						//us延时倍乘数			   
static u16 fac_ms=0;						//ms延时倍乘数,在os下,代表每个节拍的ms数
 


//初始化延迟函数
//SYSTICK 的时钟固定为 AHB 时钟，基础例程里面 SYSTICK 时钟频率为 AHB/8
//这里为了兼容 FreeRTOS，所以将 SYSTICK 的时钟频率改为 AHB 的频率！
//SYSCLK:系统时钟频率
void Delay_init(u8 SYSCLK)
{
	u32 reload;
	//SysTick频率为HCLK
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	fac_us=SYSCLK;							//不论是否使用OS,fac_us都需要使用
	reload=SYSCLK;							//每秒钟的计数次数 单位为M	   
	reload*=1000000/configTICK_RATE_HZ;		//根configTICK_RATE_HZ定溢出时间
											//reload为24位寄存器,最大值:16777216,在168M下,约合0.0998s左右	
	fac_ms=1000/configTICK_RATE_HZ;			//代表OS可以延时的最少单位	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/delay_ostickspersec秒中断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //开启SYSTICK     
}								    

#if SYSTEM_SUPPORT_OS 
//延时nus
//nus:要延时的us数.	
//nus:0~204522252(最大值即2^32/fac_us@fac_us=168)	    								   
void delay_us(u32 nus)
{ 
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;  //LOAD 的值
	ticks=nus*fac_us; //需要的节拍数
	told=SysTick->VAL; //刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL; 
		if(tnow!=told)
		{ 
			//这里注意一下 SYSTICK 是一个递减的计数器就可以了.
			if(tnow<told)tcnt+=told-tnow; 
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;  //时间超过/等于要延迟的时间,则退出.
	}
}; 
}//延时nms
//nms:要延时的ms数
//nms:0~65535
void delay_ms(u32 nms)
{ 
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
	{ 
		if(nms>=fac_ms) //延时的时间大于 OS 的最少时间周期
		{
			vTaskDelay(nms/fac_ms);  //FreeRTOS 延时
		}
		nms%=fac_ms;  //OS 已经无法提供这么小的延时了,
		//采用普通方式延时
	}
	delay_us((u32)(nms*1000)); //普通方式延时
}
//延时 nms,不会引起任务调度
//nms:要延时的 ms 数
void delay_xms(u32 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}


#define START_TASK_PRIO		1			// 任务优先级
#define START_STK_SIZE		128			// 任务堆栈大小
TaskHandle_t SystemStartTask_Handler;			// 任务句柄
extern void System_Start_Task(void *p_arg);
/* ## Task Manangement Table ## ------------------------------------------------*/
//--- Start Task ---//
// Defined in the init.c file => #define START_TASK_PRIO	1
//--- System State Task ---//
#define SYSTEM_STATE_TASK_PRIO				1		// 任务优先级
#define SYSTEM_STATE_STK_SIZE				256		// 任务堆栈大小
TaskHandle_t SystemState_Task_Handler;				// 任务句柄
extern void SYSTEM_STATE_TASK(void *p_arg);
//--- Chassis Task ---//
#define CHASSIS_TASK_PRIO					2		// 任务优先级
#define CHASSIS_STK_SIZE					256		// 任务堆栈大小
TaskHandle_t Chassis_Task_Handler;					// 任务句柄
extern void CHASSIS_TASK(void *p_arg);
//--- Gimbal Task ---//
#define GIMBAL_TASK_PRIO					2		// 任务优先级
#define GIMBAL_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Gimbal_Task_Handler;					// 任务句柄
extern void GIMBAL_TASK(void *p_arg);
//--- Rudder Task ---//
#define RUDDER_TASK_PRIO					2		// 任务优先级
#define RUDDER_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Rudder_Task_Handler;					// 任务句柄
extern void RUDDER_TASK(void *p_arg);
//--- Revolver Task ---//
#define RIFLE_TASK_PRIO					2		// 任务优先级
#define RIFLE_STK_SIZE					256		// 任务堆栈大小
TaskHandle_t Rifle_Task_Handler;					// 任务句柄
extern void RIFLE_TASK(void *p_arg);
//--- Friction Task ---//
#define MODULE_TASK_PRIO						2		// 任务优先级
#define MODULE_STK_SIZE						128		// 任务堆栈大小
TaskHandle_t Module_Task_Handler;						// 任务句柄
extern void MODULE_TASK(void *p_arg);
//--- Vision Task ---//
#define VISION_TASK_PRIO					1		// 任务优先级
#define VISION_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Vision_Task_Handler;					// 任务句柄
extern void VISION_TASK(void *p_arg);
//--- Imu Task ---//
#define IMU_TASK_PRIO						1		// 任务优先级
#define IMU_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Imu_Task_Handler;						// 任务句柄
extern void IMU_TASK(void *p_arg);
//--- PID Task ---//
#define PID_TASK_PRIO						3		// 任务优先级
#define PID_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Pid_Task_Handler;						// 任务句柄
extern void PID_TASK(void *p_arg);
//--- Masterlink Task ---//
#define MASTERLINK_TASK_PRIO						1		// 任务优先级
#define MASTERLINK_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t Masterlink_Task_Handler;						// 任务句柄
extern void MASTERLINK_TASK(void *p_arg);
//--- CUSUI Task ---//
#define CUSUI_TASK_PRIO						1		// 任务优先级
#define CUSUI_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t CUSUI_Task_Handler;						// 任务句柄
extern void CUSUI_TASK(void *p_arg);
//--- ZIGBEE Task ---//
#define ZIGBEE_TASK_PRIO					1		// 任务优先级
#define ZIGBEE_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t ZIGBEE_Task_Handler;						// 任务句柄
extern void ZIGBEE_TASK(void *p_arg);
//--- TRIGGER Task ---//
#define TRIGGER_TASK_PRIO					1		// 任务优先级
#define TRIGGER_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t TRIGGER_Task_Handler;						// 任务句柄
extern void VISION_TRIGGER_TASK(void *p_arg);
//--- SONIC Task ---//
#define SONIC_TASK_PRIO					1		// 任务优先级
#define SONIC_STK_SIZE						256		// 任务堆栈大小
TaskHandle_t SONIC_Task_Handler;						// 任务句柄
extern void SONIC_TASK(void *p_arg);

void FreeRTOS_Init(void)
{
	/* 创建初始化任务 */
	xTaskCreate((TaskFunction_t		)System_Start_Task,						// 任务函数
							(const char*		)"System_Start_Task",			// 任务名称
							(uint16_t			)START_STK_SIZE,		// 任务堆栈大小
							(void*				)NULL,					// 传递给任务函数的参数
							(UBaseType_t		)START_TASK_PRIO,		// 任务优先级
							(TaskHandle_t*		)&SystemStartTask_Handler);	// 任务句柄
	vTaskStartScheduler();	// 开启任务调度
}

void System_Start_Task(void *p_arg)
{
	taskENTER_CRITICAL();	// 进入临界区
  if (Master == MASTER_UP)
  {
	/* 创建系统状态机任务 */
	xTaskCreate((TaskFunction_t		)SYSTEM_STATE_TASK,						// 任务函数
							(const char*		)"system_state_task",		// 任务名称
							(uint16_t			)SYSTEM_STATE_STK_SIZE,		// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)SYSTEM_STATE_TASK_PRIO,	// 任务优先级
							(TaskHandle_t*		)&SystemState_Task_Handler);	// 任务句柄
	/* 创建底盘任务 */
	xTaskCreate((TaskFunction_t		)CHASSIS_TASK,							// 任务函数
							(const char*		)"chassis_task",			// 任务名称
							(uint16_t			)CHASSIS_STK_SIZE,			// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)CHASSIS_TASK_PRIO,			// 任务优先级
							(TaskHandle_t*		)&Chassis_Task_Handler);		// 任务句柄
	/* 创建云台任务 */
	xTaskCreate((TaskFunction_t		)GIMBAL_TASK,							// 任务函数
							(const char*		)"gimbal_task",				// 任务名称
							(uint16_t			)GIMBAL_STK_SIZE,			// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)GIMBAL_TASK_PRIO,			// 任务优先级
							(TaskHandle_t*		)&Gimbal_Task_Handler);		// 任务句柄			
	/* 创建舵轮任务 */
	xTaskCreate((TaskFunction_t		)RUDDER_TASK,							// 任务函数
							(const char*		)"rudder_task",				// 任务名称
							(uint16_t			)RUDDER_STK_SIZE,			// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)RUDDER_TASK_PRIO,			// 任务优先级
							(TaskHandle_t*		)&Rudder_Task_Handler);		// 任务句柄		              
	/* 创建拨盘电机任务 */
	xTaskCreate((TaskFunction_t		)RIFLE_TASK,							// 任务函数
							(const char*		)"rifle_task",			// 任务名称
							(uint16_t			)RIFLE_STK_SIZE,			// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)RIFLE_TASK_PRIO,		// 任务优先级
							(TaskHandle_t*		)&Rifle_Task_Handler);	// 任务句柄							
	/* 创建常规任务 */
	xTaskCreate((TaskFunction_t		)MODULE_TASK,								// 任务函数
							(const char*		)"module_task",				// 任务名称
							(uint16_t			)MODULE_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)MODULE_TASK_PRIO,			// 任务优先级
							(TaskHandle_t*		)&Module_Task_Handler);		// 任务句柄
	/* 创建视觉任务 */
	xTaskCreate((TaskFunction_t		)VISION_TASK,							// 任务函数
							(const char*		)"vision_task",				// 任务名称
							(uint16_t			)VISION_STK_SIZE,			// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)VISION_TASK_PRIO,			// 任务优先级
							(TaskHandle_t*		)&Vision_Task_Handler);		// 任务句柄
	/* 创建IMU任务 */
	xTaskCreate((TaskFunction_t		)IMU_TASK,								// 任务函数
							(const char*		)"imu_task",				// 任务名称
							(uint16_t			)IMU_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)IMU_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&Imu_Task_Handler);
	/* 创建PID任务 */
	xTaskCreate((TaskFunction_t		)PID_TASK,								// 任务函数
							(const char*		)"pid_task",				// 任务名称
							(uint16_t			)PID_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)PID_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&Pid_Task_Handler);
	/* 创建Masterlink任务 */
	xTaskCreate((TaskFunction_t		)MASTERLINK_TASK,								// 任务函数
							(const char*		)"Masterlink_task",				// 任务名称
							(uint16_t			)MASTERLINK_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)MASTERLINK_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&Masterlink_Task_Handler);
//	/* 创建ZIGBEE任务 */
//	xTaskCreate((TaskFunction_t		)ZIGBEE_TASK,								// 任务函数
//							(const char*		)"zigbee_task",				// 任务名称
//							(uint16_t			)ZIGBEE_STK_SIZE,				// 任务堆栈大小
//							(void*				)NULL,						// 传递给任务函数的参数
//							(UBaseType_t		)ZIGBEE_TASK_PRIO,				// 任务优先级
//							(TaskHandle_t*		)&ZIGBEE_Task_Handler);
	/* 创建硬触发任务 */
	xTaskCreate((TaskFunction_t		)VISION_TRIGGER_TASK,								// 任务函数
							(const char*		)"trigger_task",				// 任务名称
							(uint16_t			)TRIGGER_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)TRIGGER_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&TRIGGER_Task_Handler);
  }
  if (Master == MASTER_DOWN)
  {
	/* 创建系统状态机任务 */
	xTaskCreate((TaskFunction_t		)SYSTEM_STATE_TASK,						// 任务函数
							(const char*		)"system_state_task",		// 任务名称
							(uint16_t			)SYSTEM_STATE_STK_SIZE,		// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)SYSTEM_STATE_TASK_PRIO,	// 任务优先级
							(TaskHandle_t*		)&SystemState_Task_Handler);	// 任务句柄
	/* 创建Masterlink任务 */
	xTaskCreate((TaskFunction_t		)MASTERLINK_TASK,								// 任务函数
							(const char*		)"Masterlink_task",				// 任务名称
							(uint16_t			)MASTERLINK_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)MASTERLINK_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&Masterlink_Task_Handler);
	/* 创建CUSUI任务 */
	xTaskCreate((TaskFunction_t		)CUSUI_TASK,								// 任务函数
							(const char*		)"CUSUI_task",				// 任务名称
							(uint16_t			)CUSUI_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)CUSUI_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&CUSUI_Task_Handler);
	/* 创建IMU任务 */
	xTaskCreate((TaskFunction_t		)IMU_TASK,								// 任务函数
							(const char*		)"imu_task",				// 任务名称
							(uint16_t			)IMU_STK_SIZE,				// 任务堆栈大小
							(void*				)NULL,						// 传递给任务函数的参数
							(UBaseType_t		)IMU_TASK_PRIO,				// 任务优先级
							(TaskHandle_t*		)&Imu_Task_Handler);
//	/* 创建ZIGBEE任务 */
//	xTaskCreate((TaskFunction_t		)ZIGBEE_TASK,								// 任务函数
//							(const char*		)"zigbee_task",				// 任务名称
//							(uint16_t			)ZIGBEE_STK_SIZE,				// 任务堆栈大小
//							(void*				)NULL,						// 传递给任务函数的参数
//							(UBaseType_t		)ZIGBEE_TASK_PRIO,				// 任务优先级
//							(TaskHandle_t*		)&ZIGBEE_Task_Handler);
//	/* 创建SONIC任务 */
//	xTaskCreate((TaskFunction_t		)SONIC_TASK,								// 任务函数
//							(const char*		)"sonic_task",				// 任务名称
//							(uint16_t			)SONIC_STK_SIZE,				// 任务堆栈大小
//							(void*				)NULL,						// 传递给任务函数的参数
//							(UBaseType_t		)SONIC_TASK_PRIO,				// 任务优先级
//							(TaskHandle_t*		)&SONIC_Task_Handler);

  }
							
	vTaskDelete(SystemStartTask_Handler);	//删除开始任务
	taskEXIT_CRITICAL();	// 退出临界区
}
			 
#endif
