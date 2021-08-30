#include "masters_RTX.h"
#include "Function.h"
#include "Device.h"

uint8_t Rx[8];
uint8_t Tx[8];

DUM_t DUM;

//---------------------------信息层---------------------
/**
 * @brief 反馈裁判系统数据更新标志位
 * @note  频率越低越靠前 ,异步触发也靠前
 */
Data_ID_t Report_IF_DataUpdate(void)
{
  if     (DUM.TxDataInfo.REF_UD_Flag.ShootSpeed_UD) //异步触发
    return ShootSpeed_ID;
  else if(DUM.TxDataInfo.REF_UD_Flag.GameRobotStat_UD) //10hz
    return GameRobotStat_ID;
  else if(DUM.TxDataInfo.REF_UD_Flag.PowerHeatData_UD) //50hz
    return PowerHeatData_ID;
  else if(DUM.TxDataInfo.REF_UD_Flag.UI_Data_UD)
    return UI_Ctrl_ID;
  else if(DUM.TxDataInfo.REF_UD_Flag.SuperCap_UD) //200hz
    return SuperCap_tx_ID;
  else if(DUM.TxDataInfo.REF_UD_Flag.DUM_IMU_UD) //1000hz
    return IMU_DATA_ID;
  else
    return NO_DATA;
}

/**
 * @brief 下主控改变裁判系统数据更新状态
 * @note  于裁判系统串口中断中调用 
 */
void Referee_Update(Data_ID_t ID)
{
  switch(ID)
  {
    case GameRobotStat_ID:
      DUM.TxDataInfo.REF_UD_Flag.GameRobotStat_UD = YES;
      break;
    
    case PowerHeatData_ID:
      DUM.TxDataInfo.REF_UD_Flag.PowerHeatData_UD = YES;
      break;
    
    case ShootSpeed_ID:
      DUM.TxDataInfo.REF_UD_Flag.ShootSpeed_UD = YES;
      break;
    
    case IMU_DATA_ID:
      DUM.TxDataInfo.REF_UD_Flag.DUM_IMU_UD = YES;
      break;
    
    case SuperCap_tx_ID:
      DUM.TxDataInfo.REF_UD_Flag.SuperCap_UD = YES;
      break;
    
    case UI_Ctrl_ID:
      DUM.TxDataInfo.REF_UD_Flag.UI_Data_UD = YES;
      break;
  }
}



//-------------------------------下主控---------------------------
//-------------------------获取数据 装填数据-------------------------
/**
 * @brief 获取裁判系统数据
 * @param 
 */
void Get_GameRobotStat_Info(GameRobotStat_Info_t *str)
{
  str->MaxPower         = REF_Report_CHAS_MaxPower();
  str->Robot_ID         = REF_Report_robot_ID();
  str->Shoot_SpeedLimit = REF_Report_Shoot_SpeedLimit();
  str->Shooter_Output   = REF_Report_IF_shooter_output();
  str->Chassis_Output   = REF_Report_IF_chassis_output();
  str->Cooling_Limit    = REF_Report_Shoot_CoolingLimit(); 
  memcpy(Tx , str , 8);
}
void Get_PowerHeatData_Info(PowerHeatData_Info_t *str)
{
  str->CHAS_Power        = REF_Report_CHAS_Power();
  str->CHAS_PowerBuffer  = REF_Report_CHAS_PowerBuffer();
  memcpy(Tx , str , 8);
}
void Get_IMU_Info(DUM_IMU_Info_t *str)
{
  str->Yaw_Speed = Report_YawSpeed();
  str->Shooter_Heat    = REF_Report_Shooter_Heat();
  str->Shooter_Heat_01 = REF_Report_Shooter_Heat_01();
  memcpy(Tx , str , 8);
}

extern uint8_t Report_SuperCap_cmd(void);
void Get_SuperCap_Info(SuperCap_tx_Info_t *str)
{
  str->order = Report_SuperCap_cmd();
  str->CHAS_Power = DUM_Report_CHAS_Power();
  str->CHAS_PowerBuffer = DUM_Report_PowerBuffer();
  str->MaxPower = (uint8_t)DUM_Report_CHAS_MaxPower();
  memcpy(Tx , str , 8);
}

void Get_ShootSpeed_Info(ShootSpeed_Info_t *str)
{
  str->real_ShootSpeed = REF_Report_RealShootSpeed();
//  str->Shoot_CNT += 1;
  memcpy(Tx , str , 8);
}

void Get_UI_ctrl_cmd(UI_User_cmd_t *str)
{
  str->IF_AUTO = Report_IF_Auto_InitOver();
  str->IF_CLIP = Report_IF_CLIP_OPEN();
  str->IF_SPIN = CHAS_SPIN_Report();
  str->IF_Block_Much = Report_IF_SinBlocks();
  str->IF_Shootheatlimit = Report_IF_ShootHeatLimit();
  str->IF_Bullet_Warning = Report_IF_BulletWarning();
  memcpy(Tx , str , 8);
}

//----------------------------主发送层----------------------------//
/**
 * @brief 发送函数
 * @param 
 */
void Master_Sent(void)
{
  SuperCap_Update();//只在上主控触发
  
  UI_Data_Update();//只在上主控触发
  
  Data_ID_t ID = Report_IF_DataUpdate();
  switch(ID)
  {
    case GameRobotStat_ID:
        Get_GameRobotStat_Info(&DUM.TxDataInfo.GameRobotStat_Info);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.GameRobotStat_UD = NO;
        break;
    case PowerHeatData_ID:
        Get_PowerHeatData_Info(&DUM.TxDataInfo.PowerHeatData_Info);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.PowerHeatData_UD = NO;
        break;
    case ShootSpeed_ID:
        Get_ShootSpeed_Info(&DUM.TxDataInfo.ShootSpeed_Info);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.ShootSpeed_UD = NO;
        break;
    case IMU_DATA_ID:
        Get_IMU_Info(&DUM.TxDataInfo.DUM_IMU_Info);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.DUM_IMU_UD = NO;
        break;
    case SuperCap_tx_ID:
        Get_SuperCap_Info(&DUM.TxDataInfo.SuperCap_tx_Info);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.SuperCap_UD = NO;          
        break;
    case UI_Ctrl_ID:
        Get_UI_ctrl_cmd(&DUM.TxDataInfo.UI_User_cmd);
        CAN1_DUM_Send(ID , Tx);
        DUM.TxDataInfo.REF_UD_Flag.UI_Data_UD = NO; 
        break;
    default :
        break;
  }
}



/**
 * @brief 下主控IMU数据更新
 * @param 
 */
void DUM_Set_IMU_DataUpdate(void)
{
  Referee_Update(IMU_DATA_ID);
}

/**
 * @brief 上主控与超电板子通信
 * @param 
 */
void Set_SuperCap_cmd_Update(void)
{
  Referee_Update(SuperCap_tx_ID);
}
int supercap_cnt = 0;
void SuperCap_Update(void)
{
  supercap_cnt++;
  if(supercap_cnt == 5){//5ms发一次
    if(Master == MASTER_UP) //只在上主控触发
      Set_SuperCap_cmd_Update();
    supercap_cnt = 0;
  }
}

void Set_UI_Data_Update(void)
{
  Referee_Update(UI_Ctrl_ID);
}
int UI_Data_cnt = 0;
void UI_Data_Update(void)
{
  UI_Data_cnt++;
  if(UI_Data_cnt == 10){//10ms触发一次
    if(Master == MASTER_UP)//只在上主控触发
      Set_UI_Data_Update();
    UI_Data_cnt = 0;
  }
}


//-------------------------------上主控---------------------------
/**
 * @brief 主控获取接收到的数据
 * @param 
 */
void DUM_Data_Receive(uint8_t *rx,Data_ID_t ID)
{
  switch(ID)
  {
    case GameRobotStat_ID:
        memcpy(&DUM.RxDataInfo.GameRobotStat_Info, rx , 8);
        break;
    case PowerHeatData_ID:
        memcpy(&DUM.RxDataInfo.PowerHeatData_Info, rx , 8);
        break;  
    case IMU_DATA_ID:
        memcpy(&DUM.RxDataInfo.DUM_IMU_Info , rx , 8);
        break;
    case ShootSpeed_ID:
        memcpy(&DUM.RxDataInfo.ShootSpeed_Info , rx , 8);
        break;
    
    case SuperCap_rx_ID:
        memcpy(&DUM.RxDataInfo.SuperCap_rx_Info , rx , 8);
        break;
    
    case UI_Ctrl_ID:
        memcpy(&DUM.RxDataInfo.UI_User_cmd , rx , 8);
        break;
  }
}


/**
 * @brief 上主控程序读取数据接口
 * @param 
 */
/*底盘功率*/
float DUM_Report_CHAS_Power(void)
{
  return DUM.RxDataInfo.PowerHeatData_Info.CHAS_Power;
}
/*底盘最大功率*/
float DUM_Report_CHAS_MaxPower(void)
{
  if(DUM.RxDataInfo.GameRobotStat_Info.MaxPower > 60)DUM.RxDataInfo.GameRobotStat_Info.MaxPower = 60;
  return DUM.RxDataInfo.GameRobotStat_Info.MaxPower;
}
/*底盘能量缓冲*/
uint16_t DUM_Report_PowerBuffer(void)
{
  return DUM.RxDataInfo.PowerHeatData_Info.CHAS_PowerBuffer;
}
/*射速限制*/
uint8_t DUM_Report_Shoot_SpeedLimit(void)
{
  return DUM.RxDataInfo.GameRobotStat_Info.Shoot_SpeedLimit;
}
/*机器人ID*/
uint8_t DUM_Report_Robot_ID(void)
{
  return DUM.RxDataInfo.GameRobotStat_Info.Robot_ID;
}
/*机器人射击热量*/
uint16_t DUM_Report_Shoot_Heat(void)
{
  if(Get_Barrel_ID() == 1)
    return DUM_Report_Shoot_Heat_00();
  else
    return DUM_Report_Shoot_Heat_01();
}
/*机器人枪管一射击热量*/
uint16_t DUM_Report_Shoot_Heat_00(void)
{
  return DUM.RxDataInfo.DUM_IMU_Info.Shooter_Heat;
}
/*机器人机动枪管射击热量*/
uint16_t DUM_Report_Shoot_Heat_01(void)
{
  return DUM.RxDataInfo.DUM_IMU_Info.Shooter_Heat_01;
}
/*机器人射击热量上限制*/
uint16_t DUM_Report_CoolingLimit(void)
{
  /*下主控没有连接*/
  if(!IF_MASTER_DOWN_CONNECT) DUM.RxDataInfo.GameRobotStat_Info.Cooling_Limit = 120;//无连接不进行热量限制
  return DUM.RxDataInfo.GameRobotStat_Info.Cooling_Limit;
}
/*底盘陀螺仪yaw速度*/
float DUM_Report_CHAS_IMU_YAW_Speed(void)
{
  return DUM.RxDataInfo.DUM_IMU_Info.Yaw_Speed;
}
///*底盘陀螺仪pit速度*/
//float DUM_Report_CHAS_IMU_PIT_Angle(void)
//{
//  return DUM.RxDataInfo.DUM_IMU_Info.PIT_Angle;
//}

/*发射机构是否断电*/
bool DUM_Report_IF_ArmorBooster_work(void)
{
  bool res;
  if(DUM.RxDataInfo.GameRobotStat_Info.Shooter_Output == 1)
    res = YES;
  else res = NO;
  if(!IF_MASTER_DOWN_CONNECT)res = YES;//无连接默认电源正常
  return res;
}
/*反馈真实射速*/
float DUM_Report_RealShootSpeed(void)
{
  return DUM.RxDataInfo.ShootSpeed_Info.real_ShootSpeed;
}
/*反馈打弹数量*/
uint32_t DUM_Report_Shoot_NUM(void)
{
//  return DUM.RxDataInfo.ShootSpeed_Info.Shoot_CNT;
  return 0;
}
/*反馈是否起飞*/
bool DUM_Report_IF_CarFly(void)
{
//  bool res = NO;
//  if(DUM.RxDataInfo.DUM_IMU_Info.IF_FLY == 1)
//    res = YES;
//  else res = NO;
//  return res;
  return 0;
}
/*反馈对位底盘移动指令*/
Sonic_mode_t DUM_Report_Park_Fmove(void)
{
//  return DUM.RxDataInfo.DUM_IMU_Info.Park_F;
  return NO_FLY;
}
Sonic_mode_t DUM_Report_Park_Rmove(void)
{
//  return DUM.RxDataInfo.DUM_IMU_Info.Park_R;
  return NO_FLY;
}
Sonic_mode_t DUM_Report_Park_Zmove(void)
{
//  return DUM.RxDataInfo.DUM_IMU_Info.Park_Z;
  return NO_FLY;
}

//**********************UI**************************//
/*UI超电容量*/
float Report_SuperCap_Vremain(void)
{
  return DUM.RxDataInfo.SuperCap_rx_Info.V_CAP;
}
/*UI是否陀螺*/
uint8_t Report_UI_User_cmd_SPIN(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_SPIN;
}
/*UI是否自瞄*/
uint8_t Report_UI_User_cmd_AUTO(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_AUTO;
}
/*UI是否开弹舱*/
uint8_t Report_UI_User_cmd_CLIP(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_CLIP;
}
/*UI是否热量限制*/
uint8_t Report_UI_User_cmd_SHOOTLIMIT(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_Shootheatlimit;
}
/*UI是否多次卡弹*/
uint8_t Report_UI_User_cmd_BlockMuch(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_Block_Much;
}
/*UI是否补弹提醒*/
uint8_t Report_UI_User_cmd_BulletWarning(void)
{
  return DUM.RxDataInfo.UI_User_cmd.IF_Bullet_Warning;
}

//如果开弹舱，则下主控读取到的射击数量清0
void ShootNum_Reset(void)
{
//  DUM.RxDataInfo.ShootSpeed_Info.Shoot_CNT = 0;
//  DUM.TxDataInfo.ShootSpeed_Info.Shoot_CNT = 0;
}

