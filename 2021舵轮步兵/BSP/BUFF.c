/***      《 打符算法代码 》

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
#include "BUFF.h"
#include "Device.h"
#include "Function.h"

/* Init start */
BuffAim_t BuffAim = {
  .YAW.Mid_co = YAW_MID_CO,
  .YAW.TargerFix = YAW_TARFIX,
  .YAW.inc_k = YAW_INC_K,
  
  .PIT.Mid_co = PIT_MID_CO,
  .PIT.TargerFix = PIT_TARFIX,
  .PIT.inc_k = PIT_INC_K,
  
  .Buff_AutoShoot_cnt = 0,
  .Buff_AutoShoot_cnt_MAX = BUFF_AUTOSHOOT_CNT_MAX,
  .IF_AUTOSHOOT_ON = NO,
};//打符结构体

/* Init end   */


/**
 * @brief  打符自动打弹控制
 * @param 
 */
void Buff_AutoShoot_ON(void)
{
  BuffAim.IF_AUTOSHOOT_ON = YES;
}
void Buff_AutoShoot_OFF(void)
{
  BuffAim.IF_AUTOSHOOT_ON = NO;
}
bool Report_IF_Buff_AutoShoot_ON(void)
{
  return BuffAim.IF_AUTOSHOOT_ON;
}
void BuffAim_ON(void)
{
  Buff_AutoShoot_ON();//启动打符自动打弹
}
void BuffAim_OFF(void)
{
  //打符关闭与退出打符模式 两个功能分开
  Buff_AutoShoot_OFF();//关闭自动打弹
}

/**
 * @brief  YAW控制
 * @param 
 */
bool YawBuffAim_Data_Update = false;
float BuffAimYaw_Ctrl(void)
{
  float res;
  if(YawBuffAim_Data_Update == true)
  {
    if(VISION_GetFlagStatus(IF_LOCK_BUFF))
    {
      res = BuffAim.YAW.Mid_co - YAW_GetBuffCoordinate() ;
      BuffAim.YAW.TargerErr = res;
//      res = KalmanFilter(&BuffAim.YAW.KF , res);
      res = (res * BuffAim.YAW.inc_k + BuffAim.YAW.TargerFix);
    }
    else 
    {
      res = YawVision_nolook();
      BuffAim.YAW.TargerErr = 0;
    }
    
    YawBuffAim_Data_Update = false;
  }
  else res = YawVision_nolook();
  
  if(Judge_IF_NAN(res))res = 0;
  
  return res;
}



/**
 * @brief  PIT控制
 * @param 
 */
bool PitBuffAim_Data_Update = false;
float BuffAimPit_Ctrl(void)
{
  float res;
  if(PitBuffAim_Data_Update == true)
  {
    if(VISION_GetFlagStatus(IF_LOCK_BUFF))
    {
      res = BuffAim.PIT.Mid_co - PIT_GetBuffCoordinate() ;
      BuffAim.PIT.TargerErr = res;
//      res = KalmanFilter(&BuffAim.PIT.KF , res);
      res = (res * BuffAim.PIT.inc_k + BuffAim.PIT.TargerFix);
    }
    else 
    {
      res = -PitVision_nolook();
      BuffAim.PIT.TargerErr = 0;
    }
    
    PitBuffAim_Data_Update = false;
  }
  else res = -PitVision_nolook();
  
  if(Judge_IF_NAN(res))res = 0;
  
  return res;
}

//void ArmorSwitch(void)
//{
//  if(VISION_GetFlagStatus(IF_CHANGE_ARMOR_BUFF))
//  {
//    BuffAim.IF_ArmorSwitch = YES;
//  }
//  if(BuffAim.IF_ArmorSwitch == YES)
//  {
//    BuffAim.Buff_AutoShoot_cnt = BUFF_AUTOSHOOT_CNT_MAX - 100;
//    BuffAim.IF_ArmorSwitch = NO;
//  }
//}

//bool Judge_IF_Yaw_LockTar(void)
//{
//  bool res ;
//  if(BuffAim.YAW.TargerErr <= 3)
//    res = true;
//  else res = false;
//  
//  return res;
//}
//bool Judge_IF_Pit_LockTar(void)
//{
//  bool res ;
//  if(BuffAim.PIT.TargerErr <= 3)
//    res = true;
//  else res = false;
//  
//  return res;
//}

/**
 * @brief  自动打弹
 * @param 
 */
void Check_IF_BuffArmorSwitch(void)
{
  if(VISION_GetFlagStatus(IF_CHANGE_ARMOR_BUFF))//切换装甲板
    BuffAim.IF_ArmorSwitch = YES;//记录切换装甲板检测
}
void Buff_AutoShoot_CntReset(void)
{
  BuffAim.Buff_AutoShoot_cnt = 0;
}
void Buff_AutoShoot(void)
{
  if(YawBuffAim_Data_Update == true || PitBuffAim_Data_Update == true)
  if(VISION_GetFlagStatus(IF_LOCK_BUFF)){
  if(Report_IF_Buff_AutoShoot_ON())
  {
    if(abs(BuffAim.YAW.TargerErr) <= abs(BuffAim.YAW.TargerFix) + 200 && //28
       abs(BuffAim.PIT.TargerErr) <= abs(BuffAim.PIT.TargerFix) + 200 )//56
       BuffAim.Buff_AutoShoot_cnt++;
    
    if(BuffAim.IF_ArmorSwitch == YES)
    {
      BuffAim.IF_ArmorSwitch = NO;
      BuffAim.Buff_AutoShoot_cnt = BuffAim.Buff_AutoShoot_cnt_MAX - 25;//切换装甲板则准备好即刻打弹
    }
    
    if(BuffAim.Buff_AutoShoot_cnt > BuffAim.Buff_AutoShoot_cnt_MAX)
    {
      SingleFireRequest();//请求打弹
    }
  }
  }else Buff_AutoShoot_CntReset();

}




/**
 * @brief  按键微调
 * @param 
 */
void YAW_KeyFix_UP(void)
{
  BuffAim.YAW.TargerFix += 4;
}
void YAW_KeyFix_DOWN(void)
{
  BuffAim.YAW.TargerFix -= 4;
}
void PIT_KeyFix_UP(void)
{
  BuffAim.PIT.TargerFix += 4;
}
void PIT_KeyFix_DOWN(void)
{
  BuffAim.PIT.TargerFix -= 4;
}
//--
void YAW_KeyFix_UP_CON(void)
{
  BuffAim.YAW.TargerFix += 0.1f;
}
void YAW_KeyFix_DOWN_CON(void)
{
  BuffAim.YAW.TargerFix -= 0.1f;
}
void PIT_KeyFix_UP_CON(void)
{
  BuffAim.PIT.TargerFix += 0.1f;
}
void PIT_KeyFix_DOWN_CON(void)
{
  BuffAim.PIT.TargerFix -= 0.1f;
}
