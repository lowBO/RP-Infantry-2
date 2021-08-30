#include "iwdg.h"
#include "Device.h"
/**
 * @note
   @看门狗主要用于任意时刻的系统卡机，不过目前没有出现过这种情况
    也用于监测接收机是否处于掉线上线两种模式重复高频切换
    之前出现过，不过发现是接收机和遥控老化的问题，但当时看门狗能够应对这个问题做出复位决断
 */

//初始化独立看门狗
//prer:分频数:0~7(只有低3位有效!)
//rlr:自动重装载值,0~0XFFF.
//分频因子=4*2^prer.但最大值只能是256!
//rlr:重装载寄存器值:低11位有效.
//时间计算(大概):Tout=((4*2^prer)*rlr)/32 (ms).
void IWDG_Init(u8 prer,u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对IWDG->PR IWDG->RLR的写
	
	IWDG_SetPrescaler(prer); //设置IWDG分频系数

	IWDG_SetReload(rlr);   //设置IWDG装载值

	IWDG_ReloadCounter(); //reload
	
	IWDG_Enable();       //使能看门狗
}

//喂独立看门狗
void IWDG_Feed(void)
{
  /* 底盘没有nan && 云台没有nan */
  if(!CHAS_Report_IF_NAN() && !GIM_Report_IF_NAN())
	IWDG_ReloadCounter();//reload
}
