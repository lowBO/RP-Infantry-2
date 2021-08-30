/***      《 底盘控制代码 》

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
 
#include "chassis.h"
#include "Device.h"

/* Init start */
CHAS_Info_t Chassis = {
  .ctrl_mode = Ctrl_Err,
  .pid_mode = PID_Err,
  .action_mode = ACT_Err,  
  .PID_type = Turn_off,
  .IF_SPIN_START = false,
  .IF_SPIN_CLOSING = false,
  .IF_NAN = false,
  .Speed_MAX = Car_SPEED,
  .Dir.mode = HEAD,
  .Dir.num = 1,//头
};//底盘信息结构体

/* Init end   */
static System_Ctrl_Mode_t C_prev_ctrl_mode = RC_CTRL_MODE;
static PID_type_t C_prev_pid_type = Turn_off;

/* --------------------------任务层------------------------- */
/**
 * @brief 底盘总控
 * @param 
 */
void CHASSIS_Ctrl(void)
{
  if(GIM_CalOK_Report())//云台复位成功之后再启动底盘
  switch (Chassis.ctrl_mode)
  {
    case RC_CTRL_MODE:
      CHAS_RC_Ctrl();
      break;
    case KEY_CTRL_MODE: 
      CHAS_KET_Ctrl();
      break;
  }
  else CHAS_Stop();
}
 
/**
* @brief 底盘信息获取
* @param 
*/
void CHAS_GET_Info(void)
{
  Chassis.ctrl_mode = System.ctrl_mode;
  Chassis.pid_mode = System.pid_mode;
  Chassis.action_mode = System.action_mode;
  Chassis.PID_type = GET_PID_Type();
  Chassis.Defense_Mode = System.Defense_Mode;
  CHAS_SetPID_type();
  CHAS_Dir_Switch();//改变底盘方向
}

/**
 * @brief PID种类获取
 * @note  本地同步
 */
void CHAS_GET_PID_Type(PID_Info_t *str )
{
  str->PID_type = Chassis.PID_type;
}
void CHAS_SetPID_type(void)
{
  CHAS_GET_PID_Type(&Chassis.CHAS_Motor[CHAS_LF].PID);
  
  CHAS_GET_PID_Type(&Chassis.CHAS_Motor[CHAS_RF].PID);
  
  CHAS_GET_PID_Type(&Chassis.CHAS_Motor[CHAS_LB].PID); 
  
  CHAS_GET_PID_Type(&Chassis.CHAS_Motor[CHAS_RB].PID);  
}


/**
* @brief 底盘PID设置总函数
* @param 
*/
void CHAS_SetPID(void)
{
  CHAS_PID_Switch(&Chassis.CHAS_Motor[CHAS_LF].PID);
  
  CHAS_PID_Switch(&Chassis.CHAS_Motor[CHAS_RF].PID);

  CHAS_PID_Switch(&Chassis.CHAS_Motor[CHAS_LB].PID);

  CHAS_PID_Switch(&Chassis.CHAS_Motor[CHAS_RB].PID);
}

/**
 * @brief PID切换
 * @param 
 */
void CHAS_PID_Switch(PID_Info_t *str)
{
  static int length = sizeof(PID_Parameter_t);
  switch (str->PID_type)
  {
    case RC_GYRO:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][RC_GYRO],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][RC_GYRO],length);
    }break;
    
    case RC_MECH:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][RC_MECH],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][RC_MECH],length);
    }break;
    
    case KEY_GYRO:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][KEY_GYRO],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][KEY_GYRO],length);
    }break;
    
    case KEY_MECH:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][KEY_MECH],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][KEY_MECH],length);
    }break;
    
    case ACT_BIG_BUFF:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][ACT_BIG_BUFF],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][ACT_BIG_BUFF],length);
    }break;
    
    case ACT_SMALL_BUFF:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][ACT_SMALL_BUFF],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][ACT_SMALL_BUFF],length);
    }break;
    
    case ACT_AUTO_AIM:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][ACT_AUTO_AIM],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][ACT_AUTO_AIM],length);
    }break;
    
    case ACT_SPEED:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][ACT_SPEED],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][ACT_SPEED],length);
    }break;
    
    case ACT_PARK:{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][ACT_PARK],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][ACT_PARK],length);
    }break;
    
    default :{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[RM_3508][Turn_off],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[RM_3508][Turn_off],length);
    }break;      
  }
}

/**
 * @brief 底盘遥控控制
 * @param 
 */
void CHAS_RC_Ctrl(void)
{
  if(C_prev_ctrl_mode != Chassis.ctrl_mode)
  {/* 第一次进入RC模式 */
    C_prev_ctrl_mode = Chassis.ctrl_mode;
    /*清空数据*/
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LB].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RB].PID); 
    RM_3508_Init();
    SPIN_Reset();//复位小陀螺
  }
  switch (Chassis.IF_SPIN_START)
  {
    case false:
      CHAS_Remote_Ctrl();
      break;
    case true:
      CHAS_SPIN();
      break; 
  }
}
/**
 * @brief 底盘键盘控制
 * @param 
 */
void CHAS_KET_Ctrl(void)
{
  if(C_prev_ctrl_mode != Chassis.ctrl_mode)
  {/* 第一次进入KEY模式 */
    C_prev_ctrl_mode = Chassis.ctrl_mode;
    /*清空数据*/
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LB].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RB].PID); 
    RM_3508_Init();
    SPIN_Reset();//复位小陀螺
  }
  switch (Chassis.IF_SPIN_START)
  {
    case false:
      CHAS_Keyboard_Ctrl();
      break;
    case true:
      CHAS_SPIN();
      break; 
  }
}

/**
 * @brief 配合系统切换底盘正向
 * @param 
 */
Dir_t Report_CHAS_DIR(void)
{
  return Chassis.Dir.mode;
}
bool Judge_IF_CHAS_Dir_HEAD(void)
{
  bool res = NO;
  if(Report_CHAS_DIR() == HEAD)
    res = YES;
  return res;
}
bool Judge_IF_CHAS_Dir_TAIL(void)
{
  bool res = NO;
  if(Report_CHAS_DIR() == TAIL)
    res = YES;
  return res;
}
void CHAS_Dir_HEAD(void)
{
  Chassis.Dir.mode = HEAD;
}
void CHAS_Dir_TAIL(void)
{
  Chassis.Dir.mode = TAIL;
}
void CHAS_Dir_Switch(void)
{
  Judge_DIR();//判断底盘头的方向
  switch(Chassis.Dir.mode)
  {
    case HEAD:
      Chassis.Dir.num = 1;
      break;
      
    case TAIL:
      Chassis.Dir.num = -1;
      break;
  }
}

float CHAS_Power_Speed_K(void)
{
  float res = 0;
  switch((int)DUM_Report_CHAS_MaxPower())
  {
    case 45:
        res = CHAS_45W_K;
       
        break;
    case 55:
        res = CHAS_50W_K;
       
        break;
    case 60:
        res = CHAS_60W_K;
       
        break;
    case 70:
        res = CHAS_60W_K;
       
        break;
    case 80:
        res = CHAS_80W_K;
       
        break;
    case 100:
        res = CHAS_100W_K;
        
        break;
    case 120:
        res = CHAS_120W_K;
        
        break;
    default:
        res = CHAS_50W_K;
       
        break;
  }
  
  if(Report_SuperCap_cmd())
  {
    res = CHAS_SC_K;
  }
  return res;
}


/* --------------------------控制层------------------------- */
/**
 * @brief 遥控控制
 * @param 
 */
float LF_Tar  = 0,RF_Tar  = 0,LB_Tar  = 0, RB_Tar = 0;//舵轮控制需要对每个轮子的速度进行计算矢量计算，所以暂时先用四个全局变量得到四个速度
void CHAS_Remote_Ctrl(void)
{
  
  switch(Chassis.PID_type)
  {
    case RC_MECH:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入机械模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.RC_Move.FB_Speed_k = RC_CHAS_MECH_FB_k;
          Chassis.RC_Move.LR_Speed_k = RC_CHAS_MECH_LR_k;
          Chassis.RC_Move.Z_Speed_k  = RC_CHAS_MECH_Z_k;
        }
    break;
        
    case RC_GYRO:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入陀螺仪模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.RC_Move.FB_Speed_k = RC_CHAS_GYRO_FB_k;//11*660 = 7260
          Chassis.RC_Move.LR_Speed_k = RC_CHAS_GYRO_LR_k;
          Chassis.RC_Move.Z_Speed_k  = RC_CHAS_GYRO_Z_k;   //3*700 = 2100
        }
    break;
        
  }
  /*底盘运动控制解算*/
  LF_Tar =  Chassis.RC_Move.FB_Speed_k * Report_Fusion_Speed_LF() * Chassis.Dir.num;
  RF_Tar = -Chassis.RC_Move.FB_Speed_k * Report_Fusion_Speed_RF() * Chassis.Dir.num;
  LB_Tar =  Chassis.RC_Move.FB_Speed_k * Report_Fusion_Speed_LB() * Chassis.Dir.num;
  RB_Tar = -Chassis.RC_Move.FB_Speed_k * Report_Fusion_Speed_RB() * Chassis.Dir.num;
  
//  /*PID参数调试*/
//  PID_Debug(&Chassis.PID[CHAS_LF]);
//  PID_Debug(&Chassis.PID[CHAS_RF]);
//  PID_Debug(&Chassis.PID[CHAS_LB]);
//  PID_Debug(&Chassis.PID[CHAS_RB]);
  
  /*底盘输出处理*/
  CHAS_Handler(&Chassis.RC_Move);
}

/**
 * @brief 键盘控制
 * @param 
 */
void CHAS_Keyboard_Ctrl(void)
{
  float kZ_Adapt = 0;
  float Zangle;
  Zangle = YAW_AngleErrReport();
  
  switch(Chassis.PID_type)
  {
    case KEY_MECH:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入机械模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();  
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.LR_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.Z_Speed_k  = KEY_CHAS_MECH_Z_k;  
        }
    break;
        
    case KEY_GYRO:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入陀螺仪模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.LR_Speed_k = CHAS_Power_Speed_K();//5
          Chassis.Key_Move.Z_Speed_k  = KEY_CHAS_GYRO_Z_k;  //3
        }
    break;
        
    case ACT_AUTO_AIM:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入自瞄模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.LR_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.Z_Speed_k  = KEY_CHAS_GYRO_Z_k;  
        }
    break;
        
    case ACT_SMALL_BUFF:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入小符模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.LR_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.Z_Speed_k  = KEY_CHAS_GYRO_Z_k;  
        }
    break;
    
    case ACT_BIG_BUFF:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入大符模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.LR_Speed_k = CHAS_Power_Speed_K();
          Chassis.Key_Move.Z_Speed_k  = KEY_CHAS_GYRO_Z_k;  
        }
    break;
        
    case ACT_PARK:
        if(C_prev_pid_type != Chassis.PID_type)
        {/* 第一次进入大符模式 */
          C_prev_pid_type = Chassis.PID_type;
          CHAS_SetPID();
          //可改参数
          Chassis.Key_Move.FB_Speed_k = CHAS_PARK_FB_k;
          Chassis.Key_Move.LR_Speed_k = CHAS_PARK_LR_k;
          Chassis.Key_Move.Z_Speed_k  = CHAS_PARK_Z_k;  
        }
    break;
        
  }
    /*底盘运动控制解算*/
    LF_Tar =  Chassis.Key_Move.FB_Speed_k * Report_Fusion_Speed_LF() * Chassis.Dir.num;
    RF_Tar = -Chassis.Key_Move.FB_Speed_k * Report_Fusion_Speed_RF() * Chassis.Dir.num;
    LB_Tar =  Chassis.Key_Move.FB_Speed_k * Report_Fusion_Speed_LB() * Chassis.Dir.num;
    RB_Tar = -Chassis.Key_Move.FB_Speed_k * Report_Fusion_Speed_RB() * Chassis.Dir.num; 

  
  
//  /*PID参数调试*/
//  PID_Debug(&Chassis.PID[CHAS_LF]);
//  PID_Debug(&Chassis.PID[CHAS_RF]);
//  PID_Debug(&Chassis.PID[CHAS_LB]);
//  PID_Debug(&Chassis.PID[CHAS_RB]);
  
  /*底盘输出处理*/
  CHAS_Handler(&Chassis.Key_Move);
}

/**
 * @brief 底盘复位函数
 * @param 
 */
void CHAS_Reset(void)
{
  if(C_prev_ctrl_mode != Chassis.ctrl_mode)
  {/* 第一次进入复位模式 */
    C_prev_ctrl_mode = Chassis.ctrl_mode;
    /*清空数据*/
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RF].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_LB].PID);
    CHAS_SetEmpty(&Chassis.CHAS_Motor[CHAS_RB].PID); 
  }
  
  if(C_prev_pid_type != Chassis.PID_type)
  {/* 第一次进入复位模式 */
    C_prev_pid_type = Chassis.PID_type;
    CHAS_SetPID();//更新PID状态    
  }
  
  CHAS_Stop();//卸力
}


/* --------------------------功能层------------------------- */

/**
 * @brief 得到底盘四个电机的解算速度目标值
 * @note  电机的方向控制 
 */

float Speed_Adapt_RUD(float LF, float RF, float LB, float RB)
{
  float Speed_MAX = Chassis.Speed_MAX;
  float max_F,max_B,max;
  float res = 1;
  
  if(LF - RF >= 0)
    max_F = LF;
  else 
    max_F = RF;
  
  if(LB - RB >= 0)
    max_B = LB;
  else 
    max_B = RB;
  
  if(max_F - max_B >= 0)
    max = max_F;
  else
    max = max_B;
  
  if(max >= Speed_MAX)
    res = Speed_MAX/max;
  
  Judge_IF_RUNNING(max);//判断是否处于高速状态，用于限制转头
   
  return res;
}
//整车速度分配到轮子速度
void CHAS_SetChasSpeed(CHAS_Move_t *str)
{
  float Adapt_K = Speed_Adapt_RUD(abs(LF_Tar) , abs(RF_Tar) , abs(LB_Tar) , abs(RB_Tar));
  
  CHAS_Target_Slow(&str->Motor_SpeedTarget[CHAS_LF] , LF_Tar*Adapt_K , 3);
  CHAS_Target_Slow(&str->Motor_SpeedTarget[CHAS_RF] , RF_Tar*Adapt_K , 3);
  CHAS_Target_Slow(&str->Motor_SpeedTarget[CHAS_LB] , LB_Tar*Adapt_K , 3);
  CHAS_Target_Slow(&str->Motor_SpeedTarget[CHAS_RB] , RB_Tar*Adapt_K , 3);   //速度斜坡
}

/**
 * @brief 将MMKNM的速度解算 斜坡分给四个电机的PID目标值
 * @param 
 */
void CHAS_SetMotorSpeed(CHAS_Move_t *str)
{
  MotorSlow(&Chassis.CHAS_Motor[CHAS_LF].motor_data.PID_Speed_target,
       str->Motor_SpeedTarget[CHAS_LF],
       str->Slow_Inc);
  
  MotorSlow(&Chassis.CHAS_Motor[CHAS_RF].motor_data.PID_Speed_target,
       str->Motor_SpeedTarget[CHAS_RF],
       str->Slow_Inc);
  
  MotorSlow(&Chassis.CHAS_Motor[CHAS_LB].motor_data.PID_Speed_target,
       str->Motor_SpeedTarget[CHAS_LB],
       str->Slow_Inc);
  
  MotorSlow(&Chassis.CHAS_Motor[CHAS_RB].motor_data.PID_Speed_target,
       str->Motor_SpeedTarget[CHAS_RB],
       str->Slow_Inc);
}

/**
 * @brief 同步CAN得到的数据和即将参与PID计算的数据
 * @param 
 */
void CHAS_DataSync(Motor_Data_t *str)
{
  str->PID_Speed = str->CAN_GetData.Motor_Speed;
  str->PID_Angle = str->CAN_GetData.Motor_Angle;
}

/**
 * @brief 得到电机输出
 * @param 
 */
float CHAS_GetOutput(CHAS_Motor_t *str)
{
  PID_Info_t *PID = &str->PID;
  Motor_Data_t *DATA = &str->motor_data;
  float res;
  
  CHAS_DataSync(DATA);//数据同步

//  PID->Angle_Loop.Err = CHAS_Get_AngleErr(DATA);
//  
//  DATA->PID_Speed_target = PID_Algorithm(&PID->Angle_Loop);//角度环
  
  PID->Speed_Loop.Err = CHAS_Get_SpeedErr(DATA);
  
  res = PID_Algorithm(&PID->Speed_Loop);//速度环
  
  return res;
}



//第一版代码中的功率限制：
short int power_lim_output[4] = {0};//限功率后电机的输出

float power_error = 0;
float power_total_error = 0;
float power_P = 0.3,power_I = 0;
float power_P_output = 0,power_I_output = 0,power_PID_output = 0;
float power_limit_K = 1;
void power_limit(void)
{//我的功率算法
  switch(IF_DUM_NORMAL)
  {
    case YES:
      power_error = 60 - DUM_Report_PowerBuffer();
      power_P = power_error * 0.055f;//乘系数
      power_P = power_P * power_P;//求平方
      power_total_error = power_total_error + power_error;
      power_P_output = power_error*power_P;
      //.power_I_output = power_total_error * power_I;
      power_PID_output = power_P_output ;
      power_limit_K = 1 + power_PID_output;
    break;
    
    case NO:
      power_limit_K = 1;
    break;
  }
}
void CHAS_PowerLimit(CHAS_PowerLimit_t *str , int16_t *pid_out)
{//祖传功率算法
  PID_Loop_t *LF = &Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop;
  PID_Loop_t *RF = &Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop;
  PID_Loop_t *LB = &Chassis.CHAS_Motor[CHAS_LB].PID.Speed_Loop;
  PID_Loop_t *RB = &Chassis.CHAS_Motor[CHAS_RB].PID.Speed_Loop;
  
  switch(IF_DUM_NORMAL)
  {
    case YES:
        //裁判系统在线
        str->Max_PowerBuffer = CHAS_LIMIT_POWERBUFFER;
        str->Real_PowerBuffer = DUM_Report_PowerBuffer();
        if(str->Real_PowerBuffer > 60)str->Real_PowerBuffer = 60;//防止飞坡之后缓冲250J变为正增益系数
        str->Limit_k = str->Real_PowerBuffer / str->Max_PowerBuffer;
        if(str->Real_PowerBuffer < 25)
          str->Limit_k = str->Limit_k * str->Limit_k;// * str->Limit_k; //3方
        else
          str->Limit_k = str->Limit_k;// * str->Limit_k; //平方
        
        if(str->Real_PowerBuffer < str->Max_PowerBuffer)
        str->CHAS_LimitOutput = str->Limit_k * CHAS_TOTAL_OUTPUT_MAX;
        else 
        str->CHAS_LimitOutput = CHAS_TOTAL_OUTPUT_MAX;    
        break;
    
    case NO:
        //裁判系统离线，强制限为原来的1/8
        str->CHAS_LimitOutput = CHAS_TOTAL_OUTPUT_MAX  ;
        break; 
  }
  str->CHAS_TotalOutput = abs(LF->PID_Output) +
                          abs(RF->PID_Output) +
                          abs(LB->PID_Output) +
                          abs(RB->PID_Output) ;
  if(str->CHAS_TotalOutput >= str->CHAS_LimitOutput)
  {
    pid_out[CHAS_LF] = (int16_t)(LF->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
    pid_out[CHAS_RF] = (int16_t)(RF->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
    pid_out[CHAS_LB] = (int16_t)(LB->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
    pid_out[CHAS_RB] = (int16_t)(RB->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
  }
  
//  /* 飞坡分配 */
//  if( Report_SuperCap_cmd() &&  //开了超电 
//      DUM_Report_IF_CarFly() && //前轮飞出
//      KEY_W  &&
//      Chassis.CHAS_Motor[CHAS_LB].motor_data.PID_Speed > 2000 && 
//      Chassis.CHAS_Motor[CHAS_RB].motor_data.PID_Speed > 2000)   
//  {
//    //由于前两轮卸力，加入这一步使得所有的功率全部用在后轮，让后轮pid输出更大
//    //如果后轮速度在上坡是有所减慢，理论上pid输出变大后会往目标速度靠近，让后轮更加有力
//    pid_out[CHAS_LF] = (int16_t)(LF->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
//    pid_out[CHAS_RF] = (int16_t)(RF->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
//    pid_out[CHAS_LB] = (int16_t)(LB->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
//    pid_out[CHAS_RB] = (int16_t)(RB->PID_Output / str->CHAS_TotalOutput * str->CHAS_LimitOutput);
//  }
}

//车子起飞时，前两轮卸力
bool prev_fly = NO,wait_calm = NO;
float wait_calm_cnt = 0;
void CarFly_React(int16_t *pid_out)
{
  if(DUM_Report_IF_CarFly())
  {
    //飞坡处理：前两轮pid输出置0
    Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop.PID_Output = 0;
    Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop.PID_Output = 0;
    pid_out[CHAS_LF] = 0;
    pid_out[CHAS_RF] = 0;
    
    prev_fly = YES;//记录飞坡
  }
}
//车子落地时，把输出限小幅，避免超功率扣血
void CarFly_Over_React(void)
{
  if(!DUM_Report_IF_CarFly() && prev_fly == YES)
  {
    //进入飞坡结束处理，为了防止掉血，将pid输出拉低 等待一段时间再拉高
    wait_calm = YES;
    prev_fly = NO;
  }
  if(wait_calm == YES)
  {
    //飞坡处理：pid输出限幅拉低
    Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop.PID_Output_Max = 8000;
    Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop.PID_Output_Max = 8000;
    Chassis.CHAS_Motor[CHAS_LB].PID.Speed_Loop.PID_Output_Max = 8000;
    Chassis.CHAS_Motor[CHAS_RB].PID.Speed_Loop.PID_Output_Max = 8000;
    
    wait_calm_cnt ++;
    if(wait_calm_cnt >= 500)//1s之后
    {
      //处理结束，限幅恢复
      Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop.PID_Output_Max = 15000;
      Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop.PID_Output_Max = 15000;
      Chassis.CHAS_Motor[CHAS_LB].PID.Speed_Loop.PID_Output_Max = 15000;
      Chassis.CHAS_Motor[CHAS_RB].PID.Speed_Loop.PID_Output_Max = 15000;
      
      wait_calm_cnt = 0;//计数初始化
      wait_calm = NO;//冷静标志位置no，等待下一次飞坡结束处理
    }
  }
}

/**
 * @brief CHAS电机输出
 * @note  功率限制之后
 */
int16_t LFs_js = 0,RFs_js = 0,LBs_js = 0,RBs_js = 0,LFo_js = 0,RFo_js = 0,LBo_js = 0,RBo_js = 0;
extern int16_t zigbee_CHAS_Output[4];
void CHAS_Output(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
  switch(ANCESTRAL_ALGORITHM)
  {
    case NO://我自己的草率算法
        /*功率限制*/
        power_limit(); 
        /* 速度环最终输出 */
        pid_out[CHAS_LF] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_LF]) / power_limit_K);
        pid_out[CHAS_RF] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_RF]) / power_limit_K);
        pid_out[CHAS_LB] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_LB]) / power_limit_K);
        pid_out[CHAS_RB] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_RB]) / power_limit_K);
        break;
    
    case YES://祖传算法  

//        /*飞坡落地处理*/
//        CarFly_Over_React();
    
        /* 速度环最终输出 */
        pid_out[CHAS_LF] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_LF]));
        pid_out[CHAS_RF] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_RF]));
        pid_out[CHAS_LB] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_LB]));
        pid_out[CHAS_RB] = (int16_t)(CHAS_GetOutput(&Chassis.CHAS_Motor[CHAS_RB]));
    
//        /*飞坡处理*/
//        CarFly_React(pid_out);
    
        /*功率限制*/
        CHAS_PowerLimit(&Chassis.CHAS_PowerLimit , pid_out);
    
        /*Zigbee调试看的数据*/
        zigbee_CHAS_Output[CHAS_LF] = pid_out[CHAS_LF];
        zigbee_CHAS_Output[CHAS_RF] = pid_out[CHAS_RF];
        zigbee_CHAS_Output[CHAS_LB] = pid_out[CHAS_LB];
        zigbee_CHAS_Output[CHAS_RB] = pid_out[CHAS_RB];
    
        //数据观察
        LFo_js = pid_out[CHAS_LF];
        RFo_js = pid_out[CHAS_RF];
        LBo_js = pid_out[CHAS_LB];
        RBo_js = pid_out[CHAS_RB];
        LFs_js = Chassis.CHAS_Motor[CHAS_LF].motor_data.PID_Speed;
        RFs_js = Chassis.CHAS_Motor[CHAS_RF].motor_data.PID_Speed;
        LBs_js = Chassis.CHAS_Motor[CHAS_LB].motor_data.PID_Speed;
        RBs_js = Chassis.CHAS_Motor[CHAS_RB].motor_data.PID_Speed;
        break;
  }
  
  /* 判断是否出现NAN */
  Chassis.IF_NAN = Judge_IF_NAN(Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop.PID_Output) |
                   Judge_IF_NAN(Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop.PID_Output) |
                   Judge_IF_NAN(Chassis.CHAS_Motor[CHAS_LB].PID.Speed_Loop.PID_Output) |
                   Judge_IF_NAN(Chassis.CHAS_Motor[CHAS_RB].PID.Speed_Loop.PID_Output) ;
  
  /* CAN发送  */
	CAN1_Send(CHAS_ESC, pid_out);	
} 

/**
 * @brief CHAS输出调用
 * @param 
 */
void CHAS_Handler(CHAS_Move_t *str)
{
  /*得到底盘速度期望*/
  CHAS_SetChasSpeed(str);
  
  /*得到电机速度期望*/
  CHAS_SetMotorSpeed(str);
  
  /*电机输出*/
  CHAS_Output();
}

/**
 * @brief 底盘PID数据全部清0函数
 * @param 
 */
void CHAS_SetEmpty(PID_Info_t *str)
{
  memset(&str->Speed_Loop,0,sizeof(PID_Loop_t));
  memset(&str->Angle_Loop,0,sizeof(PID_Loop_t));
}
 
/**
 * @brief 卸力函数
 * @param 
 */
void CHAS_Stop(void)
{
	static int16_t pid_out[CHAS_MOTOR_CNT] = {0, 0, 0, 0};
	
	/* 速度环最终输出 */
  pid_out[CHAS_LF] = 0;
  pid_out[CHAS_RF] = 0;
  pid_out[CHAS_LB] = 0;
  pid_out[CHAS_RB] = 0;
  
	CAN1_Send(CHAS_ESC, pid_out);	  
}

/**
 * @brief 获取速度误差和角度误差
 * @note  待加入卡尔曼滤波
 */
float CHAS_Get_AngleErr(Motor_Data_t *str)
{
  float res;
  res = str->PID_Angle_target - str->PID_Angle;
  return res;
}

float CHAS_Get_SpeedErr(Motor_Data_t *str)
{
  float res;
  res = str->PID_Speed_target - str->PID_Speed;
  return res;
}
//现在弃用
float CHAS_Get_Zerror(CHAS_Move_t *str)
{
  float res;
  //卡尔曼滤波
  res = //KalmanFilter(&str->GYRO_Move.KF_GYRO_Angle,
        YAW_AngleErrReport();
  
  //PID计算
  //此处加PID计算是因为考虑后来是否可以加入积分项，因为误差用卡尔曼滤波了，可能会更好调一些
  //如果这里用PID，可以将Z_speed_k调为1
  str->GYRO_Move.GYRO_Ctrl.Angle_Loop.Err = res;
  res = PID_Algorithm(&str->GYRO_Move.GYRO_Ctrl.Angle_Loop);
  
  return res;
}
float CHAS_Get_Z_Speed(CHAS_Move_t *MOVE)
{
  float res = 0;
  float AngleErr = 0;//角度误差
  float Speed_Target = 0;//速度期望
  CHAS_GYRO_t *str = &MOVE->GYRO_Move;
  PID_Info_t *PID = &str->GYRO_Ctrl;
  
  /*得到卡尔曼滤波后的角度误差*/
  AngleErr = KalmanFilter(&str->KF_GYRO_Angle,
              YAW_AngleErrReport() + Get_CarHalfMove_Angle());//此项用于整车斜向移动
  
  /*获取角度环误差输入*/
  PID->Angle_Loop.Err = AngleErr;
  
  /*速度环期望 = 角度环PID输出*/
  Speed_Target = PID_Algorithm(&PID->Angle_Loop);
  
  /*获取速度环误差输入*/  //底盘速度值符号相反，用“+”
  PID->Speed_Loop.Err = Speed_Target + DUM_Report_CHAS_IMU_YAW_Speed();
  
  /*返回值 = 速度环PID输出*/
  res = PID_Algorithm(&PID->Speed_Loop);
  
  return res;
}

float CHAS_Report_Output_LF(void)
{
  return Chassis.CHAS_Motor[CHAS_LF].PID.Speed_Loop.PID_Output;
}
float CHAS_Report_Output_RF(void)
{
  return Chassis.CHAS_Motor[CHAS_RF].PID.Speed_Loop.PID_Output;
}
float CHAS_Report_Output_LB(void)
{
  return Chassis.CHAS_Motor[CHAS_LB].PID.Speed_Loop.PID_Output;
}
float CHAS_Report_Output_RB(void)
{
  return Chassis.CHAS_Motor[CHAS_RB].PID.Speed_Loop.PID_Output;
}
float CHAS_Report_Speed_LF(void)
{
  return Chassis.CHAS_Motor[CHAS_LF].motor_data.PID_Speed;
}
float CHAS_Report_Speed_RF(void)
{
  return Chassis.CHAS_Motor[CHAS_RF].motor_data.PID_Speed;
}
float CHAS_Report_Speed_LB(void)
{
  return Chassis.CHAS_Motor[CHAS_LB].motor_data.PID_Speed;
}
float CHAS_Report_Speed_RB(void)
{
  return Chassis.CHAS_Motor[CHAS_RB].motor_data.PID_Speed;
}

/*/-------------------------------------------------------

自主防御区
1、小陀螺
2、大扭腰
3、45度小幅度扭腰
4、。。。

--------------------------------------------------------*/
//小陀螺区------------------------------

float SPIN_GetSpeed_LR(float Angle , float FB_ctrl , float LR_ctrl)
{
  float res;
//  if(Angle < 0)Angle+=8192;
  res = arm_cos_f32(Angle*2*3.1415926f/8192.f) * LR_ctrl - arm_sin_f32(Angle*2*3.14f/8192.f) * FB_ctrl;
//  res = costable[(int)Angle] * LR_ctrl - sintable[(int)Angle] * FB_ctrl;
  return res;
}
float SPIN_GetSpeed_FB(float Angle , float FB_ctrl , float LR_ctrl)
{
  float res;
//  if(Angle < 0)Angle+=8192;  
  res = arm_sin_f32(Angle*2*3.1415926f/8192.f) * LR_ctrl + arm_cos_f32(Angle*2*3.14f/8192.f) * FB_ctrl;
//  res = sintable[(int)Angle] * LR_ctrl + costable[(int)Angle] * FB_ctrl;
  return res;  
}
////测试小陀螺的速度
//float spin_speed_test = SPIN_SPEED;
//void SPIN_Speed_up20(void)
//{
//  spin_speed_test += 20;
//}
//void SPIN_Speed_down20(void)
//{
//  spin_speed_test -= 20;
//}
//float Get_test_spin_speed(void)
//{
//  return spin_speed_test;
//}
/**
 * @brief 陀螺速度
 * @note  根据功率调速度
 */
//bool SPIN_SuperCap = NO;
//bool IF_SPIN_SuperCap(void)
//{
//  
//}
//float SuperCap_speed(void)
//{
//  switch()


//}
void SPIN_FB_Change(void)
{
  switch((int)DUM_Report_CHAS_MaxPower())
  {
    case 45:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_50W;
       
        break;
    case 55:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_50W;
       
        break;
    case 60:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_60W;
       
        break;
    case 70:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_60W;
       
        break;
    case 80:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_80W;
       
        break;
    case 100:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_100W;
        
        break;
    case 120:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_120W;
        
        break;
    default:
        Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_50W;
       
        break;
  }
  
  if(Report_SuperCap_cmd())
  {
    Chassis.SPIN_Move.FB_Speed_k = SPIN_FBk_SUPER;
  }
}
/**
 * @brief 陀螺总函数
 * @note  包含大小扭腰 （陀螺为个体防御战术的统称）
 */
float Js_Zangle = 0;
void CHAS_SPIN(void)
{
  float Zangle;
  Zangle = YAW_AngleErrReport();
  Js_Zangle = Zangle;
  SPIN_FB_Change();
  switch(Chassis.ctrl_mode)
  {
    case RC_CTRL_MODE:
        /*底盘运动控制解算*/
        LF_Tar = Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_LF() * Chassis.Dir.num;
        RF_Tar =-Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_RF() * Chassis.Dir.num;
        LB_Tar = Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_LB() * Chassis.Dir.num;
        RB_Tar =-Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_RB() * Chassis.Dir.num;
        break;
    
    case KEY_CTRL_MODE:
        /*底盘运动控制解算*/
        LF_Tar = Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_LF() * Chassis.Dir.num;
        RF_Tar =-Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_RF() * Chassis.Dir.num;
        LB_Tar = Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_LB() * Chassis.Dir.num;
        RB_Tar =-Chassis.SPIN_Move.FB_Speed_k * Report_Fusion_Speed_RB() * Chassis.Dir.num;
        break;
  } 
  /*底盘输出处理*/
  CHAS_Handler(&Chassis.SPIN_Move);
}

bool CHAS_SPIN_Report(void)
{ 
  return Chassis.IF_SPIN_START;
}

bool CHAS_SPIN_Close_Report(void)
{
  return Chassis.IF_SPIN_CLOSING;
}


/**
 * @brief 启动小陀螺
 * @param 
 */
void SPIN_ON(void)
{
  if(GIM_CalOK_Report())//初始回正完成了才能小陀螺
  {
    SPIN_Reset();//小陀螺回正时能重新开小陀螺
    Chassis.IF_SPIN_START = true;
  }
}

/**
 * @brief 关闭小陀螺
 * @param 
 */
void SPIN_OFF(void)
{
//    YAW_MECH_Mid_Angle_Proc();//调用此函数可以就头尾回正
  Chassis.IF_SPIN_CLOSING = true;
}

/**
 * @brief 判断小陀螺是否回正完成
 * @param 
 */
int closing_cnt = 0;   //回正总计时：超时代表卡住
int Angleback_cnt = 0;   //回正有效计时：达到时间代表稳定回正
/* 全部标志位复位 */
void SPIN_Reset(void)
{
  /*计算部分的参数复位*/
//  Chassis.SPIN_Move.GYRO_Move.GYRO_Ctrl.Angle_Loop.PID_Err_Dead = 0;
//  Chassis.SPIN_Move.Z_Speed_k = 1.5;
  C_prev_pid_type = Turn_off;  //为第一次进入普通控制做准备
  closing_cnt = 0;             //判断计时清0
  Angleback_cnt = 0;           //判断计时清0
  Chassis.IF_SPIN_START = false;//小陀螺未开启
  Chassis.IF_SPIN_CLOSING = false;//小陀螺未进行关闭
}
/* 判断 */
void SPIN_CLOSING_Judge(void)
{
  if(Chassis.IF_SPIN_CLOSING == true)
  {   
    Chassis.SPIN_Move.GYRO_Move.GYRO_Ctrl.Angle_Loop.PID_Err_Max = 18000;//2000
    if(abs(YAW_AngleErrReport()) <= 200)//到位时间
      Angleback_cnt++;
    closing_cnt++;//“卡住”时间
    if(Angleback_cnt > 30 || closing_cnt > 3000)
      //小陀螺回正角度差很小的计时 //小陀螺回正进行的时间（有限时间）
      SPIN_Reset();
  }
}

/**
 * @brief 键盘小陀螺开关
 * @param 
 */
void SPIN_KEY_Switch(void)
{
  switch(Chassis.IF_SPIN_START)
  {
    case true :
      SPIN_OFF();
      break;
    case false :
      SPIN_ON();
      break;
  }
}

/**
 * @brief 切换陀螺防御模式
 * @param 
 */
void SPIN_Defense_Mode_Switch(void)
{
  if(CHAS_SPIN_Report())
  switch(System.Defense_Mode)
  { 
    case Defense_SPIN:
      System.Defense_Mode = Defense_BIG_DODGE;
      break;
    case Defense_BIG_DODGE:
      System.Defense_Mode = Defense_SPIN;//Defense_SMALL_DODGE;
      break;
    case Defense_SMALL_DODGE:
      System.Defense_Mode = Defense_SPIN;
      break;
  }
}

void SPIN_Defense_Mode_Reset(void)
{
  System.Defense_Mode = Defense_SPIN;
}
  

//----------------------------超级电容--------------------------------//
/**
 * @brief 上报超电命令
 * @param 
 */
uint8_t SuperCap_cmd = 0;
uint8_t Report_SuperCap_cmd(void)
{
  return SuperCap_cmd;
}

/**
 * @brief 开启超电
 * @param 
 */
void SuperCap_KEY_ON(void)
{
  SuperCap_cmd = 1;
}

/**
 * @brief 关闭超电
 * @param 
 */
void SuperCap_KEY_OFF(void)
{
  SuperCap_cmd = 0;
}

/**
 * @brief 上报是否出现NAN
 * @param 
 */
bool CHAS_Report_IF_NAN(void)
{
  return Chassis.IF_NAN;
}
