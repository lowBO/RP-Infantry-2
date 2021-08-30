/***      《 打弹控制代码 》

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
 
#include "rifle.h"
#include "Device.h"
#include "Function.h"
 
/* Init start */
RIFLE_Info_t Rifle = {
  .ctrl_mode = Ctrl_Err,
  .pid_mode = PID_Err,
  .action_mode = ACT_Err,
  .PID_type = Turn_off,
  .SingleAngle = SINGLE_ANGLE,
  .ContinuFreq = CONTIN_FREQ_10,
  .Barrel_ID = ID_Left,
};//拨盘信息结构体


/* Init end   */

/* --------------------------任务层------------------------- */
/**
 * @brief 拨盘打弹机制主控
 * @param 
 */
void RIFLE_Ctrl(void)
{
  switch(Rifle.ctrl_mode)
  {
    case  RC_CTRL_MODE:
      RIFLE_RC_Ctrl();
      break;
    
    case  KEY_CTRL_MODE:
      RIFLE_KET_Ctrl();
      break;
  }
  RIFLE_Handler();
}

void Normal_Ctrl(void)
{

}
 
/**
 * @brief 拨盘电机信息获取
 * @param 
 */
void RIFLE_GET_Info(void)
{
  Rifle.ctrl_mode = System.ctrl_mode;
  Rifle.pid_mode = System.pid_mode;
  Rifle.action_mode = System.action_mode;
  Rifle.PID_type = GET_PID_Type();
  FlagUpdate();
  Check_IF_FricSpeed_Normal();
  
  KEY_SCAN();
  Judge_Barrel_ID();

}

 /**
 * @brief 拨盘电机PID设置总函数
 * @param 
 */
void RIFLE_SetPID(void)
{
  RIFLE_GET_PID_Type(&Rifle.Motor[RIFLE].PID);
  RIFLE_PID_Switch(&Rifle.Motor[RIFLE].PID);
}

/**
 * @brief PID种类获取
 * @param 
 */
void RIFLE_GET_PID_Type(PID_Info_t *str)
{
  str->PID_type = Rifle.PID_type;
}
 
/**
 * @brief PID切换
 * @param 
 */
void RIFLE_PID_Switch(PID_Info_t *str )
{
  static int length = sizeof(PID_Parameter_t);
  switch (str->PID_type)
  {   
    case Turn_off :{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[M_2006][Turn_off],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[M_2006][Turn_off],length);
    }break;  

    default :{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[M_2006][RC_GYRO],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[M_2006][RC_GYRO],length);
    }break; 
  }
}



/**
 * @brief 拨盘遥控控制
 * @param 
 */
 System_Ctrl_Mode_t R_prev_ctrl_mode = Ctrl_Err;
void RIFLE_RC_Ctrl(void)
{
  if(R_prev_ctrl_mode != Rifle.ctrl_mode)
  {/* 第一次进入RC模式 */
    R_prev_ctrl_mode = Rifle.ctrl_mode;
    RIFLE_MotorAngleInit();
    M_2006_Init();
  }
//  /*PID参数调试*/
//  PID_Debug(&Rifle.Motor[RIFLE].PID);
  
  if(Rifle.flag.IF_Block)BlockReact();//卡弹操作
  else//不卡弹操作
  switch (Rifle.PID_type)
  {
    case RC_MECH:
      RIFLE_MECH_RC_Ctrl();
      break;
    case RC_GYRO:
      RIFLE_GYRO_RC_Ctrl();
      break; 
  }
  
}
/**
 * @brief 拨盘键盘控制
 * @param 
 */
void RIFLE_KET_Ctrl(void)
{
  if(R_prev_ctrl_mode != Rifle.ctrl_mode)
  {/* 第一次进入KEY模式 */
    R_prev_ctrl_mode = Rifle.ctrl_mode;
    RIFLE_MotorAngleInit(); 
    M_2006_Init();   
  }
  if(Rifle.flag.IF_Block)BlockReact();//卡弹操作
  else//不卡弹操作
  {
    Keyboard_Ctrl();
//    Yaw_AutoShoot();//自瞄自动打弹
//    Buff_AutoShoot();//打符自动打弹
  }
}
bool IF_SELF_CHANGE_Barrel_left  = NO;
void Set_IF_SELF_CHANGE_Barrel_left(void)
{
  IF_SELF_CHANGE_Barrel_left = YES;
}
void Reset_IF_SELF_CHANGE_Barrel_left(void)
{
  IF_SELF_CHANGE_Barrel_left = NO;
}
bool Report_IF_SELF_CHANGE_Barrel_left(void)
{
  return IF_SELF_CHANGE_Barrel_left;
}

bool IF_SELF_CHANGE_Barrel_righ  = NO;
void Set_IF_SELF_CHANGE_Barrel_righ(void)
{
  IF_SELF_CHANGE_Barrel_righ = YES;
}
void Reset_IF_SELF_CHANGE_Barrel_righ(void)
{
  IF_SELF_CHANGE_Barrel_righ = NO;
}
bool Report_IF_SELF_CHANGE_Barrel_righ(void)
{
  return IF_SELF_CHANGE_Barrel_righ;
}

void Judge_Barrel_ID(void)
{
  switch(Rifle.Barrel_ID)
  {
    case ID_Left :
      if((Report_IF_TurnRigh_allow() && Report_IF_ShootHeatLimit())||Report_IF_SELF_CHANGE_Barrel_righ())
        Rifle.Barrel_ID = ID_Righ;
        Reset_IF_SELF_CHANGE_Barrel_righ();
      break;
    
    case ID_Righ :
      if((Report_IF_TurnLeft_allow() && Report_IF_ShootHeatLimit())||Report_IF_SELF_CHANGE_Barrel_left())
        Rifle.Barrel_ID = ID_Left;
        Reset_IF_SELF_CHANGE_Barrel_left();
      break;
  } 
}

Barrel_ID_t Get_Barrel_ID(void)
{
  Barrel_ID_t res;
  res = Rifle.Barrel_ID;
  return res;
}

//右枪管偏移
float AutoAim_Barrelturn(void)
{
  float res = 0;
  if(Get_Barrel_ID() == ID_Righ)
    res = 0;
  return res;
}



/* --------------------------控制层------------------------- */

/**
 * @brief 机械模式
 * @param 
 */
static PID_type_t R_prev_pid_type = Turn_off;
void RIFLE_MECH_RC_Ctrl(void)
{
  if(R_prev_pid_type != Rifle.PID_type)
  {/* 第一次进入机械模式 */
    R_prev_pid_type = Rifle.PID_type;
    RIFLE_MotorAngleInit();
    RIFLE_SetPID();
    FlagReset();

  }
  SingleFire();
}

/**
 * @brief 陀螺仪模式
 * @param 
 */
void RIFLE_GYRO_RC_Ctrl(void)
{
  if(R_prev_pid_type != Rifle.PID_type)
  {/* 第一次进入陀螺仪模式 */
    R_prev_pid_type = Rifle.PID_type;
    RIFLE_MotorAngleInit();
    RIFLE_SetPID();
    ContinFreq_10();//连发射频恢复为最低
    FlagReset();
    
  }
  ContinFire();
}

void Keyboard_Ctrl(void)
{
  if(R_prev_pid_type != Rifle.PID_type)
  {
    R_prev_pid_type = Rifle.PID_type;
    RIFLE_SetPID();
    ContinFreq_10();//连发射频恢复为最低
    FlagReset();
  }
  SingleFire();
  ContinFire();
}

/**
 * @brief 复位函数
 * @note 
 * @param 
 */
void RIFLE_Reset(void)
{  
  if(R_prev_ctrl_mode != Rifle.ctrl_mode)
  {/* 第一次进入复位模式 */
    R_prev_ctrl_mode = Rifle.ctrl_mode;
    //清空数据
    memset(&Rifle.Motor[RIFLE].PID.Speed_Loop,0,sizeof(PID_Loop_t));
    memset(&Rifle.Motor[RIFLE].PID.Angle_Loop,0,sizeof(PID_Loop_t));
    FlagReset();
  }
  
  if(R_prev_pid_type != Rifle.PID_type)
  {/* 第一次进入复位模式 */
    R_prev_pid_type = Rifle.PID_type;
    RIFLE_SetPID();    
  } 
  //卸力
  RIFLE_Stop();
}
/* --------------------------功能层------------------------- */
/**
 * @brief 状态更新
 * @param 
 */
void FlagUpdate(void)
{
  Rifle.flag.IF_SinFire_Request = SinFireRequest_Report();
  Rifle.flag.IF_ConFire_Request = ConFireRequest_Report();
  Rifle.flag.IF_SingleFire_OVER = Judge_IF_SingleFire_OVER();

  //卡弹检测
  if(!Rifle.flag.IF_ConFire_Request)
    //单发检测
    Rifle.flag.IF_SinFire_Block = Judge_SinFireBlock();
  else          
    //连发检测
    Rifle.flag.IF_ConFire_Block = Judge_ConFireBlock();
  
  Rifle.flag.IF_Block = IF_FireBlock();
  Rifle.flag.IF_Stuck = Stuck_Judge();
}
void FlagReset(void)
{
  Rifle.flag.IF_ConFire_Request = false;//取消连发请求
  Rifle.flag.IF_SinFire_Request = false;//取消单发请求
//  Rifle.flag.IF_SingleFire_OVER = YES;  //单发准备就绪
}

/**
 * @brief 判断单发完成
 * @note  如果连发则返回YES 1
 * @param 
 */
bool Judge_IF_SingleFire_OVER(void)
{ 
  bool res = NO;
  Motor_Data_t *DATA = &(Rifle.Motor[RIFLE].motor_data);
  
  if(abs(DATA->PID_Angle_target-DATA->PID_Angle) <= Judge_AngleErr)   
    res = YES;
  else 
    res = NO;
  if(Rifle.flag.IF_ConFire_Request)
    res = YES;//如果连发则进入  
  return res;
}

//正转一格
void Single_Forward(void)
{
  Rifle.Motor[RIFLE].motor_data.PID_Angle_target += Rifle.SingleAngle * Get_Barrel_ID();
}
void Single_Forward_HALF(void)
{
  Rifle.Motor[RIFLE].motor_data.PID_Angle_target += Rifle.SingleAngle/2 * Get_Barrel_ID();
}
void SetAngle_stay(void)
{
  Rifle.Motor[RIFLE].motor_data.PID_Angle_target = Rifle.Motor[RIFLE].motor_data.PID_Angle;
}
//反转一格
void Single_Reverse(void)
{
  Rifle.Motor[RIFLE].motor_data.PID_Angle_target -= Rifle.SingleAngle * Get_Barrel_ID();
}
void Single_Reverse_HALF(void)
{
  Rifle.Motor[RIFLE].motor_data.PID_Angle_target -= Rifle.SingleAngle/2 * Get_Barrel_ID();
}
/**
 * @brief 单发
 * @note  如果有请求位，则判断上一发是否发射完成，若发射完成则进行下一次单发
 * @param 
 */
void SingleFire(void)
{
  if(Rifle.flag.IF_SinFire_Request)//有单发请求
  if(Report_IF_Fric3508_SetSpeed()){//当摩擦轮为3508时，3508有达到目标转速左右
  if(Judge_IF_SingleHeat_ShootAllow()){//热量限制
  if(Rifle.flag.IF_SingleFire_OVER)//上一次单发结束
  {
    Single_Forward();
    Rifle.flag.IF_SinFire_Request = false;
  }
  }else RIFLE_MotorAngleInit();
  }else RIFLE_MotorAngleInit();
}
/**
 * @brief 连发
 * @param 
 */
void ContinFreq_20(void)
{
  /*20射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_20;
  if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit() - ContinHeatLimitErr - 30)
    ContinFreq_05();
}
void ContinFreq_15(void)
{
  /*15射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_15;
  if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit() - ContinHeatLimitErr - 30)
    ContinFreq_05();
}
void ContinFreq_12(void)
{
  /*12射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_12;
  if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit() - ContinHeatLimitErr - 30)
    ContinFreq_03();
}
void ContinFreq_10(void)
{
  /*10射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_10;
  if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit() - ContinHeatLimitErr - 30)
    ContinFreq_03();
}
void ContinFreq_05(void)
{
  /* 5射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_05;
}

void ContinFreq_03(void)
{
  /* 3射频*/
  Rifle.ContinuFreq = CONTIN_FREQ_03;
}

void ContinFire(void)
{
  if(Rifle.flag.IF_ConFire_Request)//有连发请求
  if(Report_IF_Fric3508_SetSpeed()){//当摩擦轮为3508时，3508有达到目标转速左右
  if(Judge_IF_Heat_ShootAllow()){//热量限制
  if(Rifle.flag.IF_SingleFire_OVER)//上一次单发结束
  {
    Rifle.flag.IF_SingleFire_OVER = YES;
    Rifle.Motor[RIFLE].motor_data.PID_Speed_target = -Rifle.ContinuFreq * MOTOR_1GPS_SPEED * Get_Barrel_ID();
  }
  }else RIFLE_MotorAngleInit();
  }else RIFLE_MotorAngleInit();
}
//----------------------------卡弹处理-------------------
//卡弹信息复位
void BlockState_Reset(void)
{
  BlockState_t *str = &Rifle.Motor[RIFLE].BlockState;
  str->IF_FireBlock = false; //卡弹标志位清0，表示已不卡弹
  str->BlockReact_Over = YES;
  str->type = NO_Block;
  str->cnt = 0;
}
//堵转检测
bool Stuck_Judge(void)
{
  Motor_Data_t *DATA = &Rifle.Motor[RIFLE].motor_data;
  PID_Info_t *PID = &Rifle.Motor[RIFLE].PID;
  if(abs(DATA->PID_Speed) <= Judge_Stuck_SpeedMAX && 
      abs(PID->Speed_Loop.PID_Output) > Judge_Stuck_OutputMIN)
    return YES;
  else return NO;
}
/**
 * @brief 卡弹电机反应
 * @note  
 */
bool First = 0;
int Block_num = 0; //卡弹的次数
void SinFireBlock_React(void)
{
  BlockState_t *str = &Rifle.Motor[RIFLE].BlockState;
  //先反转
//  if(Block_num == 1)//卡弹达到6次  防止无限卡弹
//  {
//    //返回半个目标角度，以便下次有力打弹
//    Rifle.Motor[RIFLE].motor_data.PID_Angle_target -= Rifle.SingleAngle/4;
//    str->BlockReact_Over = YES;//卡弹反应完成
//    Block_num = 0;//卡弹次数清0
//  }
//  else   //如果还没到卡6次
//  {
    if(First == 0)
    {
      Single_Reverse();//反转1格
      Single_Reverse_HALF();//反转半格
      //一共一格半
      First = 1;//只执行一次
    }
    str->cnt--;
    if(Rifle.flag.IF_SingleFire_OVER || str->cnt == 0)
    {
//      //反转结束/时间到了 就正转
//      Single_Forward();
      SetAngle_stay();//停在原地
      str->BlockReact_Over = YES; 
      Block_num++;//卡弹次数加1
            First = 0;
    }
//  }
  
  if(str->BlockReact_Over == YES)
  //卡弹反应完成后
    BlockState_Reset();
}
/**
 * @brief 提醒卡弹次数太多 
 * @note  
 */
void Reset_Block_num(void)
{
  Block_num = 0;
}
bool Judge_IF_SinBlocks(void)
{
  bool res = NO; 
  if(Block_num >= Block_WarningNUM)
    res = YES;
  else res = NO;
  return res;
}
bool Report_IF_SinBlocks(void)
{
  return Judge_IF_SinBlocks();
}
/**
 * @brief 连发卡弹检测 
 * @note  
 */
void ConFireBlock_React(void)
{
  BlockState_t *str = &Rifle.Motor[RIFLE].BlockState;
  //先反转
  Rifle.Motor[RIFLE].motor_data.PID_Speed_target = MOTOR_SPEED_05 * Get_Barrel_ID();
  str->cnt --;
//  //如果反转堵住，则加快反转计时速度
//  if(Stuck_Judge())str->cnt -= 3;
  if( str->cnt <= Judge_ConBlock_TIM - 300)//5射频，200ms一颗，回转一格
  {//时间到了 就复位
    RIFLE_MotorAngleInit();
    str->BlockReact_Over = YES;
  }
  
  if(str->BlockReact_Over == YES)
  //卡弹反应完成后
    BlockState_Reset();
}

void BlockReact(void)
{
  RIFLE_Motor_t *str = &Rifle.Motor[RIFLE];
  switch(str->BlockState.type)
  {
    case SinBlock:
      //单发卡弹
      SinFireBlock_React();
      break;
    case ConBlock:
      //连发卡弹
      ConFireBlock_React();
      break;
  }
}
/**
 * @brief 卡弹时间检测 
 * @note   单发检测
 */
bool Judge_SinFireBlock(void)
{
  BlockState_t *str = &Rifle.Motor[RIFLE].BlockState;
  
  if(str->IF_FireBlock == NO)//只有在无卡弹标志位的时候才检测
  {
    if(Judge_IF_SingleFire_OVER() == NO)
    //时间检测
    str->cnt++;
    else str->cnt = 0;
  }
  
  if(str->cnt >= Judge_SinBlock_TIM)//计时超过一定时间认为卡弹
  {
    str->IF_FireBlock = YES;//判断为卡弹
    str->type = SinBlock; //判断为单发卡弹
    str->BlockReact_Over = NO;  //卡弹反应还没做
  }
  
  return str->IF_FireBlock;//返回卡弹状态
}
/**
 * @brief 卡弹堵转检测 
 * @note   连发检测
 */
bool Judge_ConFireBlock(void)
{
  BlockState_t *str = &Rifle.Motor[RIFLE].BlockState;
  
  if(str->IF_FireBlock == NO)//只有在无卡弹标志位的时候才检测
  {
    if(Stuck_Judge())
    //堵转检测
    str->cnt++;
    else str->cnt = 0;
  }
  
  if(str->cnt >= Judge_ConBlock_TIM)//计时超过一定时间认为卡弹
  {
    str->IF_FireBlock = YES;
    str->type = ConBlock;
    str->BlockReact_Over = NO;
  }
  
  return str->IF_FireBlock;//返回卡弹状态
}
bool IF_FireBlock(void)
{
  return Rifle.Motor[RIFLE].BlockState.IF_FireBlock;
}
//-----------------------------------------------
/**
 * @brief 摩擦轮启动的打弹延迟
 * @note   如果太快允许可以打弹，
           在操作手狂按的情况下
           大概率摩擦轮被卡住不能起转
 */
int FricCheck_cnt = 0;
bool IF_FricSpeed_Normal = NO;
void Check_IF_FricSpeed_Normal(void)
{
  bool res = NO;
  
  if(IF_MODULE_WORK)
    FricCheck_cnt++;
  else FricCheck_cnt = 0;
  
  if(FricCheck_cnt >= FricNormal_Delay) {
    res = YES;
    FricCheck_cnt = FricNormal_Delay;//防溢出
  }
  IF_FricSpeed_Normal = res;
}
bool Report_IF_FricSpeed_Normal(void)
{
  return IF_FricSpeed_Normal;
}

/**
 * @brief 请求单发
 * @note 
 * @param 
 */
void SingleFireRequest(void)
{
  if(IF_MODULE_WORK)
  //确保常规模块开启后才能打弹
  if(Report_IF_FricSpeed_Normal())
  //确保摩擦轮速度正常
  if(!ConFireRequest_Report())
  //无连发
  if(!IF_FireBlock())//不卡弹
  {
    Rifle.flag.IF_SinFire_Request = true;
    Buff_AutoShoot_CntReset();//触发打弹时打符自动打弹的计数重置归0
  }
}

void ContinFire_ON(void)
{
  if(IF_MODULE_WORK)
  //确保常规模块开启后才能打弹
  if(Report_IF_FricSpeed_Normal())
  //确保摩擦轮速度正常
  if(!SinFireRequest_Report())
  //无单发
  if(!IF_FireBlock())//不卡弹
    Rifle.flag.IF_ConFire_Request = true;
}

void ContinFire_OFF(void)
{
  if(Rifle.flag.IF_ConFire_Request == true)
  {
    RIFLE_MotorAngleInit();
    Rifle.flag.IF_ConFire_Request = false;
  }
}

bool SinFireRequest_Report(void)
{
  return Rifle.flag.IF_SinFire_Request;
}

bool ConFireRequest_Report(void)
{
  return Rifle.flag.IF_ConFire_Request;
}


//----------------------驱动层---------------------
/**
 * @brief 得到电机Err
 * @param 
 */
float RIFLE_Get_AngleErr(Motor_Data_t *str)
{
  float res;
  res = str->PID_Angle_target - str->PID_Angle;
  return res;
}
/**
 * @brief 得到电机Err
 * @param 
 */
float RIFLE_Get_SpeedErr(Motor_Data_t *str)
{
  float res;
  res = str->PID_Speed_target - str->PID_Speed;
  return res;
}

/**
 * @brief 对电机返回的角度值进行一个越界处理
 * @param 
 */
int16_t prev_Angle = 4096;
int ROUND = 0;
float RIFLE_MotorAngle_Proc(int16_t Angle)
{
  float res;
  if(Angle - prev_Angle >= 7200)
    ROUND--;
  if(Angle - prev_Angle <=-7200)
    ROUND++;
  prev_Angle = Angle;
  res = ROUND * 8192 + Angle;
  return (float)res;
}
/**
 * @brief 电机角度和计算值初始化/复位
 * @param 
 */
void RIFLE_MotorAngleInit(void)
{
  Rifle.flag.IF_SinFire_Request = false;//取消一直狂按导致的单发，但连发不取消，一直按着能打就打
  
  Motor_Data_t *str = &(Rifle.Motor[RIFLE].motor_data);
  str->PID_Angle_target = str->CAN_GetData.Motor_Angle;
  str->PID_Speed_target = 0;
  prev_Angle = 4096;
  ROUND = 0;
}
/**
 * @brief 数据同步
 * @param 
 */
void RIFLE_DataSync(Motor_Data_t *str)
{
  str->PID_Angle = RIFLE_MotorAngle_Proc(str->CAN_GetData.Motor_Angle);
  str->PID_Speed = str->CAN_GetData.Motor_Speed;
}
/**
 * @brief 得到电机输出
 * @param 
 */
float RIFLE_GetOutput(RIFLE_Motor_t *str)
{
  PID_Info_t *PID = &str->PID;
  Motor_Data_t *DATA = &str->motor_data;
  int16_t res;
  
  RIFLE_DataSync(DATA);//数据同步

  PID->Angle_Loop.Err = RIFLE_Get_AngleErr(DATA);
  
  if(!Rifle.flag.IF_ConFire_Request)
  //非连发则启用角度环，连发则关闭
    DATA->PID_Speed_target = PID_Algorithm(&PID->Angle_Loop);//角度环
  
  PID->Speed_Loop.Err = RIFLE_Get_SpeedErr(DATA);
  
  res = (int16_t)PID_Algorithm(&PID->Speed_Loop);//速度环
  
  return res;
}

/**
 * @brief RIFLE电机输出
 * @note 
 */
void RIFLE_Output(void)
{
	static int16_t pid_out[4] = {0,0,0,0};
	/* 速度环最终输出 */
  pid_out[RIFLE] = (int16_t)RIFLE_GetOutput(&Rifle.Motor[RIFLE]);

	CAN2_Send(RIFLE_ESC, pid_out);	
} 

void RIFLE_Stop(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
	CAN2_Send(RIFLE_ESC, pid_out);  
}

void RIFLE_Handler(void)
{
    /*电机输出*/  
    RIFLE_Output();
}


bool ShootAllow = YES,SingleAllow = YES;
bool IF_Brust = NO;//解禁爆发
bool Judge_IF_SingleHeat_ShootAllow(void)
{
  /*判断是否超热量*/
  if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit()-SingleHeatLimitErr )
    SingleAllow = NO;
  
  /*超热量之后等待热量降下来再允许开火*/
  if(SingleAllow == NO)
    if(DUM_Report_Shoot_Heat() < DUM_Report_CoolingLimit()-SingleHeatLimitErr )
      SingleAllow = YES;
    
  /*底盘主控通讯异常*/
  if(!IF_DUM_NORMAL)SingleAllow = YES;
    
  /*热量解禁，换血模式*/
  if(IF_Brust == YES)SingleAllow = YES;
    
  return SingleAllow;
}
bool Judge_IF_Heat_ShootAllow(void)
{
//  if(Get_Barrel_ID() == ID_Left)
//  {
      /*判断是否超热量*/
      if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit()-ContinHeatLimitErr  )
        ShootAllow = NO;
      
      /*超热量之后等待热量降下来再允许开火*/
      if(ShootAllow == NO)
        if(DUM_Report_Shoot_Heat() < DUM_Report_CoolingLimit()-ContinHeatLimitErr )
          ShootAllow = YES;
      
      /*底盘主控通讯异常*/
      if(!IF_DUM_NORMAL)ShootAllow = YES;
        
      /*热量解禁，换血模式*/
      if(IF_Brust == YES)ShootAllow = YES;
//  }
//  if(Get_Barrel_ID() == ID_Righ)
//  {
//      /*判断是否超热量*/
//      if(DUM_Report_Shoot_Heat() >= DUM_Report_CoolingLimit()-ContinHeatLimitErr  )
//        ShootAllow = NO;
//      
//      /*超热量之后等待热量降下来再允许开火*/
//      if(ShootAllow == NO)
//        if(DUM_Report_Shoot_Heat() < DUM_Report_CoolingLimit()-ContinHeatLimitErr )
//          ShootAllow = YES;
//      
//      /*底盘主控通讯异常*/
//      if(!IF_DUM_NORMAL)ShootAllow = YES;
//        
//      /*热量解禁，换血模式*/
//      if(IF_Brust == YES)ShootAllow = YES;
//  }
    
    
  return ShootAllow;
}
/**
 * @brief 反馈是否热量限制
 * @note  1--热量限制  0--没被限制
 */
bool Report_IF_ShootHeatLimit(void)
{
  return !Judge_IF_Heat_ShootAllow();//打弹不允许 即 热量限制
}

void Brust_ON(void)
{
  IF_Brust = YES;
}
void Brust_OFF(void)
{
  IF_Brust = NO;
}

/**
 * @brief 提醒是否需要补弹
 * @note  
 */
bool BulletWarning = NO;
bool Judge_IF_BulletWarning(void)
{
  bool res;
  if(DUM_Report_Shoot_NUM() >= ShootWarningNum )
    res = YES;
  else res = NO;
  return res;
}
bool Report_IF_BulletWarning(void)
{
  return Judge_IF_BulletWarning();
}
void Reset_BulletWarning(void)
{
  BulletWarning = NO;
}
