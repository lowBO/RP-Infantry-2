/***      《 云台控制代码 》

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
 
#include "gimbal.h"
#include "Device.h"
#include "Function.h"

/* Init start */
GIM_Info_t Gimbal = {
  .ctrl_mode = Ctrl_Err,
  .pid_mode = PID_Err,
  .action_mode = ACT_Err,
  .PID_type = Turn_off,
  .GIM_Cal_state = NO_Cal,
  .IF_CALIBRATE_OK = NO,
  .IF_NAN = false,
  .YAW.MECH_Mid_Angle = YAW_MOTOR_MID_ANGLE,
  .PIT.MECH_Mid_Angle = PIT_MOTOR_MID_ANGLE,
};//云台信息结构体

/* Init end   */
static System_Ctrl_Mode_t G_prev_ctrl_mode = Ctrl_Err;
static PID_type_t G_prev_pid_type = Turn_off;
static PID_type_t YAW_prev_pid_type = Turn_off;
static PID_type_t PIT_prev_pid_type = Turn_off;
/* --------------------------任务层------------------------- */
/**
 * @brief 云台总控
 * @param 
 */
void GIMBAL_Ctrl(void)
{
  if(IF_IMU_InitOver())//IMU初始化完成之后再开启云台
  switch (Gimbal.ctrl_mode)
  {
    case RC_CTRL_MODE:
      GIM_RC_Ctrl();
      break;
    case KEY_CTRL_MODE: 
      GIM_KET_Ctrl();
      break;
  }
}

/**
 * @brief 云台信息获取
 * @param 
 */
void GIM_GET_Info(void)
{
  Gimbal.ctrl_mode = System.ctrl_mode;
  Gimbal.pid_mode = System.pid_mode;
  Gimbal.action_mode = System.action_mode;
  Gimbal.PID_type = GET_PID_Type();
  Gimbal.Defense_Mode = System.Defense_Mode;
  IMU_Data_Report();
  Judge_IF_TurnOver();
  SystemSet_IF_CarHalfMove();//用于判断整车斜向移动时的朝向
}

/**
 * @brief PID种类获取
 * @param 
 */
void GIM_GET_PID_Type(AXIS_Info_t *strAxis)
{
  strAxis->PID.PID_type = Gimbal.PID_type;
}
void GIM_AXIS_SetPID_type(void)
{
  GIM_GET_PID_Type(&Gimbal.YAW);
  
  GIM_GET_PID_Type(&Gimbal.PIT);    
}

/**
 * @brief 云台PID设置总函数
 * @param 
 */
void GIM_SetPID(void)
{
  GIM_AXIS_SetPID_type();
  
  GIM_PID_Switch(&Gimbal.YAW);

  GIM_PID_Switch(&Gimbal.PIT); 
}
/**
 * @brief PID切换
          yaw和pitch
 * @param 原版：Motor_Data_t *str , Motor_Info_t *strx
          新版：AXIS_Info_t
 */
void GIM_PID_Switch(AXIS_Info_t *strAxis)
{
  PID_Info_t *str = &(strAxis->PID);
  Motor_Info_t *strx = &(strAxis->motor_info);
  static int length = sizeof(PID_Parameter_t);
  switch(strx->motor_type)
  {
    case GM_6020_YAW:  
      switch (str->PID_type)
      {
        case RC_GYRO:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][RC_GYRO],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][RC_GYRO],length);
        }break;
        
        case RC_MECH:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][RC_MECH],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][RC_MECH],length);
        }break;
        
        case KEY_GYRO:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][KEY_GYRO],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][KEY_GYRO],length);
        }break;
        
        case KEY_MECH:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][KEY_MECH],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][KEY_MECH],length);
        }break;
        
        case ACT_BIG_BUFF:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][ACT_BIG_BUFF],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][ACT_BIG_BUFF],length);
        }break;
        case ACT_SMALL_BUFF:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][ACT_SMALL_BUFF],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][ACT_SMALL_BUFF],length);
        }break;
        case ACT_AUTO_AIM:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][ACT_AUTO_AIM],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][ACT_AUTO_AIM],length);
        }break;
        
        case ACT_SPEED:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][ACT_SPEED],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][ACT_SPEED],length);
        }break;
        
        case ACT_PARK:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][ACT_PARK],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][ACT_PARK],length);
        }break;
        
        default :{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_YAW][Turn_off],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_YAW][Turn_off],length);
        }break;      
      }break;
    case GM_6020_PIT:
      switch (str->PID_type)
      {
        case RC_GYRO:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][RC_GYRO],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][RC_GYRO],length);
        }break;
        
        case RC_MECH:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][RC_MECH],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][RC_MECH],length);
        }break;
        
        case KEY_GYRO:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][KEY_GYRO],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][KEY_GYRO],length);
        }break;
        
        case KEY_MECH:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][KEY_MECH],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][KEY_MECH],length);
        }break;
        
        case ACT_BIG_BUFF:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][ACT_BIG_BUFF],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][ACT_BIG_BUFF],length);
        }break;
        case ACT_SMALL_BUFF:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][ACT_SMALL_BUFF],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][ACT_SMALL_BUFF],length);
        }break;
        case ACT_AUTO_AIM:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][ACT_AUTO_AIM],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][ACT_AUTO_AIM],length);
        }break;
        
        case ACT_SPEED:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][ACT_SPEED],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][ACT_SPEED],length);
        }break;
        
        case ACT_PARK:{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][ACT_PARK],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][ACT_PARK],length);
        }break;

        default :{
          memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_PIT][Turn_off],length);
          memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_PIT][Turn_off],length);
        }break;      
      }break;
  }
}

/**
 * @brief 云台遥控控制
 * @param 
 */
void GIM_RC_Ctrl(void)
{
  if(G_prev_ctrl_mode != Gimbal.ctrl_mode)
  {/* 第一次进入RC模式 */
    G_prev_ctrl_mode = Gimbal.ctrl_mode;
    Start_GIM_Calibrate();
  }
  switch(Gimbal.IF_CALIBRATE_OK)
  {
    case true :
      GIM_Remote_Ctrl();
      break;
    case false :
      GIM_Calibrate();
      break;
  }
}
/**
 * @brief 云台键盘控制
 * @param 
 */
void GIM_KET_Ctrl(void)
{
  if(G_prev_ctrl_mode != Gimbal.ctrl_mode)
  {/* 第一次进入KEY模式 */
    G_prev_ctrl_mode = Gimbal.ctrl_mode;
    Start_GIM_Calibrate();
  }
  switch(Gimbal.IF_CALIBRATE_OK)
  {
    case true :
      GIM_Keyboard_Ctrl();
      break;
    case false :
      GIM_Calibrate();
      break;
  }
}

/**
 * @brief 配合系统的功能切换函数
 * @param 
 */
void GIM_Function_Switch(void)
{
  switch(Gimbal.ctrl_mode)
  {
    case RC_CTRL_MODE:
      break;
      
    case KEY_CTRL_MODE:
      break;
  }
}





/* --------------------------控制层------------------------- */
/*-----------------------分模式的控制区--------------------------*/
/**
 * @brief 限制陀螺仪模式的头部不要超前底盘太多
 * @param 
 */
void RC_GYRO_AngleLimit(AXIS_Info_t *str)
{
  if(abs(YAW_AngleErrReport()) > 1000)
   str->motor_data.PID_Angle_target -= str->RC_Move.AXIS_Angle_Inc ;
}
/**
 * @brief GIM_YAW轴的控制 遥控器遥控模式
 * @param 
 */
void Remote_Ctrl_YAW(AXIS_Info_t *str)
{
//  /*PID参数调试*/
//  PID_Debug(&str->PID);

  switch(str->PID.PID_type)
  { 
    case RC_GYRO:
      if(YAW_prev_pid_type != Gimbal.PID_type)
      {/* 第一次进入模式 */
        YAW_prev_pid_type = Gimbal.PID_type;
        
        GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
      }
      /*云台运动控制解算*/
      str->RC_Move.AXIS_Angle_Inc = str->RC_Move.AXIS_Angle_k * (-RC_CH0);
     
      /*云台期望值获取*/
      str->motor_data.PID_Angle_target += str->RC_Move.AXIS_Angle_Inc;
      
//      //不处于小陀螺模式则进行超前角度限制
//      if(!CHAS_SPIN_Report())RC_GYRO_AngleLimit(str);
    
      /*同步陀螺仪模式YAW数据*/
      GIM_YawDataSync_GYRO(&str->motor_data);
    break;
        
    case RC_MECH:
      if(YAW_prev_pid_type != Gimbal.PID_type)
      {/* 第一次进入模式 */
        YAW_prev_pid_type = Gimbal.PID_type;
        
        GIM_MECH_FirstYawAngle(&Gimbal.YAW);
      }
      
      /*云台运动控制解算*/     
      str->RC_Move.AXIS_Angle_Inc = str-> MECH_Mid_Angle ;
    
      /*云台期望值获取*/
      str->motor_data.PID_Angle_target = str->RC_Move.AXIS_Angle_Inc;
    
      /*同步陀螺仪模式YAW数据*/    
      GIM_YawDataSync_MECH(&str->motor_data);        
    break;
  }
}

/**
 * @brief GIM_PIT轴的控制 遥控器遥控模式
 * @param 
 */
void Remote_Ctrl_PIT(AXIS_Info_t *str)
{
  if(PIT_prev_pid_type != Gimbal.PID_type)
  {/* 第一次进入模式 */
    PIT_prev_pid_type = Gimbal.PID_type;
    
    GIM_FirstPitAngle(&Gimbal.PIT);
  }
  
//  /*PID参数调试*/
//  PID_Debug(&str->PID); 
  
  /*云台运动控制解算*/
  str->RC_Move.AXIS_Angle_Inc = str->RC_Move.AXIS_Angle_k * (-RC_CH1);

  /*云台期望值获取*/
  str->motor_data.PID_Angle_target = \
  constrain(str->motor_data.PID_Angle_target + str->RC_Move.AXIS_Angle_Inc,
            PIT_MOTOR_MAX_ANGLE,
            PIT_MOTOR_MIN_ANGLE); 
 
  /*同步陀螺仪模式PITCH数据*/
  GIM_PitchDataSync(&str->motor_data);
}

/**
 * @brief GIM 总控制 遥控器遥控模式
 * @param 
 */
void GIM_Remote_Ctrl(void)
{  
  if(CHAS_SPIN_Report())
    //遥控小陀螺模式->限定云台运动为陀螺仪模式
    Gimbal.PID_type = RC_GYRO;
  
  if(G_prev_pid_type != Gimbal.PID_type)
  {/* 第一次进入模式 */
    G_prev_pid_type = Gimbal.PID_type;
    
    GIM_SetPID();
    GM_6020_Init();
    
  }  
  Remote_Ctrl_YAW(&Gimbal.YAW);
  Remote_Ctrl_PIT(&Gimbal.PIT);
  
  /*云台输出*/
  GIM_Output();
}


//普通键盘控制区-------------------
/**
 * @brief GIM_YAW轴的控制 键盘模式
 * @param 
 */
void Keyboard_Ctrl_YAW(AXIS_Info_t *str)
{
//  /*PID参数调试*/
//  PID_Debug(&str->PID);
  
  switch(str->PID.PID_type)
  { 
    case KEY_GYRO:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data);  
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * (Mouse_X_Speed()+ Report_MECH_SlowlyTurn());

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target += str->Key_Move.AXIS_Angle_Inc;
        break;
    
    case KEY_MECH:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstYawAngle(&Gimbal.YAW);
//          
//          SPIN_OFF();//关闭小陀螺
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_MECH(&str->motor_data);  
        
        /*云台运动控制解算*/      
        str->Key_Move.AXIS_Angle_Inc = str-> MECH_Mid_Angle;
      
        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = str->Key_Move.AXIS_Angle_Inc;
        break;
    
    case ACT_AUTO_AIM:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        } 
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data); 

        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = AutoAimYaw_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = str->motor_data.PID_Angle + \
                                           str->Key_Move.AXIS_Angle_Inc ;
        break;
      
    case ACT_SMALL_BUFF:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data);  
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = BuffAimYaw_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = str->motor_data.PID_Angle + \
                                           str->Key_Move.AXIS_Angle_Inc;
        break;
      
    case ACT_BIG_BUFF:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data);  
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = BuffAimYaw_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = str->motor_data.PID_Angle + \
                                           str->Key_Move.AXIS_Angle_Inc;
        break;
        
        
    case ACT_PARK:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data);  
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * (Mouse_X_Speed() + Report_MECH_SlowlyTurn());

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target += str->Key_Move.AXIS_Angle_Inc;
        
        break;

    default:
        if(YAW_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          YAW_prev_pid_type = Gimbal.PID_type;
          
          GIM_GYRO_FirstYawAngle(&Gimbal.YAW);
        }
        /*同步陀螺仪模式YAW数据*/
        GIM_YawDataSync_GYRO(&str->motor_data);  

        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * Mouse_X_Speed();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target += str->Key_Move.AXIS_Angle_Inc;
        break;
  }
}

/**
 * @brief GIM_PIT轴的控制 键盘模式
 * @param 
 */
void Keyboard_Ctrl_PIT(AXIS_Info_t *str)
{
//  /*PID参数调试*/
//  PID_Debug(&str->PID); 
  
  
  switch(str->PID.PID_type)
  {      
    case ACT_AUTO_AIM:
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstPitAngle(&Gimbal.PIT);
          
          if(Report_IF_ShoorSentry())
            Gim_Sentry_UP();//打哨兵的快速抬头
        } 
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data); 
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = AutoAimPit_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle + str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);  
        break;
 
    case ACT_SMALL_BUFF:
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstPitAngle(&Gimbal.PIT);
        } 
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data); 
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = BuffAimPit_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle - str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);  
        break;
    
    case ACT_BIG_BUFF:
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstPitAngle(&Gimbal.PIT);
        } 
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data); 
        
        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = BuffAimPit_Ctrl();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle - str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);  
        break;
    
    case KEY_GYRO:
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstPitAngle(&Gimbal.PIT);
        }
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data);

        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * Mouse_Y_Speed();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle_target + str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);
        break;
    
    case ACT_PARK:
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_MECH_FirstPitAngle(&Gimbal.PIT);
        }
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data);

        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * Mouse_Y_Speed();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle_target + str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);
        break;
    
    default ://机械模式
        if(PIT_prev_pid_type != Gimbal.PID_type)
        {/* 第一次进入模式 */
          PIT_prev_pid_type = Gimbal.PID_type;
          
          GIM_FirstPitAngle(&Gimbal.PIT);
        }
        /*同步陀螺仪模式PITCH数据*/
        GIM_PitchDataSync(&str->motor_data);

        /*云台运动控制解算*/
        str->Key_Move.AXIS_Angle_Inc = str->Key_Move.AXIS_Angle_k * Mouse_Y_Speed();

        /*云台期望值获取*/
        str->motor_data.PID_Angle_target = \
        constrain(str->motor_data.PID_Angle_target + str->Key_Move.AXIS_Angle_Inc,
                  PIT_MOTOR_MAX_ANGLE,
                  PIT_MOTOR_MIN_ANGLE);
        break;
  }
}

/**
 * @brief GIM 总控制 键盘模式
 * @param 
 */
void GIM_Keyboard_Ctrl(void)
{
  if(G_prev_pid_type != Gimbal.PID_type)
  {/* 第一次进入陀螺仪模式 */
    G_prev_pid_type = Gimbal.PID_type;
    GIM_SetPID();    
  }
  Keyboard_Ctrl_YAW(&Gimbal.YAW);
  Keyboard_Ctrl_PIT(&Gimbal.PIT);
  
  /*云台输出*/
  GIM_Output();
}


/**
 * @brief 云台复位函数
 * @param 
 */
void GIM_Reset(void)
{
  if(G_prev_ctrl_mode != Gimbal.ctrl_mode)
  {/* 第一次进入复位模式 */
    G_prev_ctrl_mode = Gimbal.ctrl_mode;
    /*清空数据*/
    GIM_SetEmpty(&Gimbal.YAW.PID);
    GIM_SetEmpty(&Gimbal.PIT.PID);
  }
  
  if(G_prev_pid_type != Gimbal.PID_type)
  {/* 第一次进入复位模式 */
    G_prev_pid_type = Gimbal.PID_type;
    YAW_prev_pid_type = Gimbal.PID_type;
    PIT_prev_pid_type = Gimbal.PID_type;
    GIM_SetPID();//更新PID状态
  }
  
  GIM_Stop();//卸力
}





/* --------------------------功能层------------------------- */
//PID计算输出区---------------
/**
 * @brief 对YAW电机角度进行处理
 * @note  把 机械模式、陀螺仪底盘跟随、小陀螺回正 所用到的yaw电机角度
          都进行处理，此处理相当于就近回正
 */
float YAW_MotorAngle_Proc(int16_t Angle)
{
  if(Angle <= Gimbal.YAW.MECH_Mid_Angle - Motor_180)
    Angle += Motor_360;
  if(Angle > Gimbal.YAW.MECH_Mid_Angle + Motor_180)
    Angle -= Motor_360;   

  return (float)Angle;
}
float PIT_MotorAngle_Proc(int16_t Angle)
{
  if(Angle < Gimbal.PIT.MECH_Mid_Angle - Motor_180)
    Angle += Motor_360;
  if(Angle > Gimbal.PIT.MECH_Mid_Angle + Motor_180)
    Angle -= Motor_360;   

  return (float)Angle;
}

float YAW_IMUAngle_Proc(float Angle)
{
  if(Angle <= Gimbal.YAW.motor_data.PID_Angle_target - IMU_180)
    Angle += IMU_360;
  if(Angle > Gimbal.YAW.motor_data.PID_Angle_target + IMU_180)
    Angle -= IMU_360;   

  return (float)Angle;
}


/**
 * @brief 同步IMU得到的数据和即将参与PID计算的数据
 *        陀螺仪模式 YAW
 */
void GIM_YawDataSync_GYRO(Motor_Data_t *str)
{
  str->PID_Speed = str->IMU_GetData.IMU_Speed;
//  str->PID_Angle = str->IMU_GetData.IMU_Angle;//陀螺仪模式无反向回正
  str->PID_Angle = YAW_IMUAngle_Proc(str->IMU_GetData.IMU_Angle);
}

/**
 * @brief 同步IMU和CAN得到的数据和即将参与PID计算的数据
 *        机械模式 YAW
 */
void GIM_YawDataSync_MECH(Motor_Data_t *str)
{
  str->PID_Speed = str->IMU_GetData.IMU_Speed;
  str->PID_Angle = YAW_MotorAngle_Proc(str->CAN_GetData.Motor_Angle);
}

/**
 * @brief 同步CAN得到的数据和即将参与PID计算的数据
 *        陀螺仪模式和机械模式的PITCH
 */
void GIM_PitchDataSync(Motor_Data_t *str)
{
  str->PID_Speed = str->IMU_GetData.IMU_Speed;
  str->PID_Angle = PIT_MotorAngle_Proc(str->CAN_GetData.Motor_Angle);
}

void GIM_PitchDataSync_GYRO(Motor_Data_t *str)
{
  str->PID_Speed = str->IMU_GetData.IMU_Speed;
  str->PID_Angle = str->IMU_GetData.IMU_Angle;
}

/**
 * @brief 获取串级PID输出
 * @param 
 */
float GIM_GetOutput(AXIS_Info_t *str)
{
  PID_Info_t *PID = &(str->PID);
  Motor_Data_t *DATA = &(str->motor_data);
  float res;
 
  /*获取角度环误差输入*/
  PID->Angle_Loop.Err = GIM_Get_AngleErr(str);
  
  /*速度环期望 = 角度环PID输出*/
  DATA->PID_Speed_target = PID_Algorithm(&PID->Angle_Loop);

  /*获取速度环误差输入*/  
  PID->Speed_Loop.Err = GIM_Get_SpeedErr(str);
  
  /*返回值 = 速度环PID输出*/
  res = PID_Algorithm(&PID->Speed_Loop);
  
  return res;
}


/**
 * @brief GIM电机最终输出
 * @param 
 */
extern int16_t zigbee_GIM_Output[2];
void GIM_Output(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
	
	/* 速度环最终输出 */
  pid_out[GIM_YAW] = (int16_t)GIM_GetOutput(&Gimbal.YAW);
  pid_out[GIM_PIT] = (int16_t)GIM_GetOutput(&Gimbal.PIT);
  
  /* 判断是否出现NAN */
  Gimbal.IF_NAN = Judge_IF_NAN(Gimbal.YAW.PID.Speed_Loop.PID_Output) |
                  Judge_IF_NAN(Gimbal.PIT.PID.Speed_Loop.PID_Output);

  /* CAN发送 */
	CAN1_Send(GIM_ESC, pid_out);	
  
  /*zigbee 看数据*/
  zigbee_GIM_Output[GIM_YAW] = pid_out[GIM_YAW];
  zigbee_GIM_Output[GIM_PIT] = pid_out[GIM_PIT];
}
  



//-----------------------------------------
  
//回正控制区----%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/**
 * @brief 启动回正
 * @param 
 */
void Start_GIM_Calibrate(void)
{
  Gimbal.IF_CALIBRATE_OK = false;
  GIM_Cal_StateSync(NO_Cal);
//  SPIN_Reset();//复位小陀螺
}

/**
 * @brief Yaw回正函数
 * @param 
 */
void GIM_YAW_Calibrate(AXIS_Info_t *str)
{
  /*回正速度控制：角度变化单元的大小*/
  str->PID.Angle_Loop.PID_Err_Max = Yaw_CalErrLimit;
  
  /*设置YAW轴角度目标为机械中值*/
  str->motor_data.PID_Angle_target = str->MECH_Mid_Angle;
  
  /*同步陀螺仪模式YAW数据*/    
  GIM_YawDataSync_MECH(&str->motor_data);
}

/**
 * @brief Pitch回正函数
 * @param 
 */
void GIM_PIT_Calibrate(AXIS_Info_t *str)
{
  /*回正速度控制：角度变化单元的大小*/
  str->PID.Angle_Loop.PID_Err_Max = Yaw_CalErrLimit;
  
  /*设置PITCH轴角度目标为机械中值*/
  str->motor_data.PID_Angle_target = str->MECH_Mid_Angle;
  
  /*同步陀螺仪模式PITCH数据*/    
  GIM_PitchDataSync(&str->motor_data);  
}

/**
 * @brief 判断回正状态
 * @param 
 */
void GIM_Calibrate_state(void)
{
  AXIS_Info_t *YAW = &(Gimbal.YAW),*PIT = &(Gimbal.PIT);
  
  //yaw判断
  if(abs(YAW_AngleErrReport()) <= Calibrate_AngleErr)
    YAW->Calibrate_Info.OK_cnt++;
  else
    YAW->Calibrate_Info.ING_cnt++;
  if(YAW->Calibrate_Info.ING_cnt > GIM_Calibrate_CNT ||
     YAW->Calibrate_Info.OK_cnt > GIM_CalibrateOK_CNT)
    //回正时间过长或回正完成
    YAW->Calibrate_Info.State = Cal_OK;
    
  //pitch判断 
  if(abs(PIT_AngleErrReport()) <= Calibrate_AngleErr)
    PIT->Calibrate_Info.OK_cnt++;
  else 
    PIT->Calibrate_Info.ING_cnt++;
  if(PIT->Calibrate_Info.ING_cnt > GIM_Calibrate_CNT ||
     PIT->Calibrate_Info.OK_cnt > GIM_CalibrateOK_CNT)
    //回正时间过长或回正完成
    PIT->Calibrate_Info.State = Cal_OK;  
  
  if(YAW->Calibrate_Info.State == Cal_OK &&
     PIT->Calibrate_Info.State == Cal_OK)
    //回正完成，设置标志位
    {  
      Gimbal.IF_CALIBRATE_OK = true;
      Gimbal.GIM_Cal_state = Cal_OK;
    }
  
  if(Gimbal.IF_CALIBRATE_OK == true)
  {
    //回正完成，电机数据改回去
    GM_6020_Init();    
  }
}

/**
 * @brief 同步云台回正状态信息
 * @param 
 */
void GIM_Cal_StateSync(Calibrate_State_t state)
{
  Gimbal.GIM_Cal_state = state;
  Gimbal.YAW.Calibrate_Info.State = Gimbal.GIM_Cal_state;
  Gimbal.PIT.Calibrate_Info.State = Gimbal.GIM_Cal_state;
}

/**
 * @brief 回正总函数
 * @param 
 */
void GIM_Calibrate(void)
{
  if(Gimbal.GIM_Cal_state == NO_Cal)
  {/* 第一次进入模式 */ /*回正采用机械模式*/
    GIM_Cal_StateSync(Calibrating);
    
    Gimbal.PID_type = RC_MECH;//机械模式
    GIM_AXIS_SetPID_type();
    GIM_SetPID();
    GM_6020_Init();
    memset(&Gimbal.YAW.Calibrate_Info,0,sizeof(Calibrate_Info_t));
    memset(&Gimbal.PIT.Calibrate_Info,0,sizeof(Calibrate_Info_t));   
//    YAW_MECH_Mid_Angle_Proc();//调用此函数可以就头尾回正    
  }    
  
  GIM_YAW_Calibrate(&Gimbal.YAW);
  GIM_PIT_Calibrate(&Gimbal.PIT);

  /*云台输出*/
  GIM_Output(); 

  //检测回正状态
  GIM_Calibrate_state();
}

bool GIM_CalOK_Report(void)
{
  return Gimbal.IF_CALIBRATE_OK;
}

//--------------------------------------

//---------------------快速转向区-------------------
/* -----------------------整车反向------------------------------ */
/**
 * @brief 角度期望改变
 * @param 
 */
void TurnLeft_90(void)
{
  if(GIM_CalOK_Report())//回正完成后
  if(Gimbal.pid_mode == GYRO)//只在陀螺仪模式中
  if(Gimbal.Turn_Info.Left.IF_Turn_90 == false)
  {
    TurnRighReset();//左转则右转复位
    Gimbal.YAW.motor_data.PID_Angle_target += Turn_90;//2048; 
    Gimbal.Turn_Info.Left.IF_Turn_90 = true;
  }
}
void TurnRigh_90(void)
{
  if(GIM_CalOK_Report())//回正完成后
  if(Gimbal.pid_mode == GYRO)//只在陀螺仪模式中
  if(Gimbal.Turn_Info.Righ.IF_Turn_90 == false)
  {
    TurnLeftReset();//右转则左转复位
    Gimbal.YAW.motor_data.PID_Angle_target -= Turn_90;//2048; 
    Gimbal.Turn_Info.Righ.IF_Turn_90 = true;
  }
}
void TurnLeft_180(void)
{
  if(GIM_CalOK_Report())//回正完成后
  if(Gimbal.pid_mode == GYRO)//只在陀螺仪模式中
  if(Gimbal.Turn_Info.Left.IF_Turn_180 == false)
  {
    TurnRighReset();//左转则右转复位
    Gimbal.YAW.motor_data.PID_Angle_target += Turn_180 - 100;//4096;
    Gimbal.Turn_Info.Left.IF_Turn_180 = true;
  }
}
void TurnRigh_180(void)
{
  if(GIM_CalOK_Report())//回正完成后
  if(Gimbal.pid_mode == GYRO)//只在陀螺仪模式中
  if(Gimbal.Turn_Info.Righ.IF_Turn_180 == false)
  {
    TurnLeftReset();//右转则左转复位
    Gimbal.YAW.motor_data.PID_Angle_target -= Turn_180 - 100;//4096; 
    Gimbal.Turn_Info.Righ.IF_Turn_180 = true;
  }
}
/**
 * @brief 判断是否转向完毕
 * @param 
 */
void Judge_IF_TurnOver(void)
{
  Car_Turn_t *Left = &Gimbal.Turn_Info.Left;
  Car_Turn_t *Righ = &Gimbal.Turn_Info.Righ;
  
  if(Left->IF_Turn_90 == true ||
     Left->IF_Turn_180 == true)
  {
    Left->cnt++;
    if(abs(GIM_Get_AngleErr(&Gimbal.YAW)) <= 15 || Left->cnt >=800)
    {
      TurnLeftReset();
      Gimbal.YAW.motor_data.PID_Angle_target = Gimbal.YAW.motor_data.PID_Angle;
    }
  }
  
  if(Righ->IF_Turn_90 == true ||
     Righ->IF_Turn_180 == true)
  {
    Righ->cnt++;
    if(abs(GIM_Get_AngleErr(&Gimbal.YAW)) <= 15 || Righ->cnt >=800)
    {
      TurnRighReset();
      Gimbal.YAW.motor_data.PID_Angle_target = Gimbal.YAW.motor_data.PID_Angle;
    }
  }
}
/**
 * @brief 转向信息复位
 * @param 
 */
void TurnLeftReset(void)
{
  Gimbal.Turn_Info.Left.IF_Turn_90 = false;
  Gimbal.Turn_Info.Left.IF_Turn_180 = false;
  Gimbal.Turn_Info.Left.cnt = 0;
}
void TurnRighReset(void)
{
  Gimbal.Turn_Info.Righ.IF_Turn_90 = false;
  Gimbal.Turn_Info.Righ.IF_Turn_180 = false;
  Gimbal.Turn_Info.Righ.cnt = 0;
}
void TurnReset(void)
{
  TurnLeftReset();
  TurnRighReset();
}
/* ----------------底盘反向-------------------- */
void Judge_DIR(void)
{
  //根据云台回正角度判断底盘方向
  if(Gimbal.YAW.MECH_Mid_Angle == YAW_MOTOR_MID_ANGLE)
    CHAS_Dir_HEAD();
  else CHAS_Dir_TAIL();
}
void CHAS_Turn_180(void)
{
  if(Gimbal.YAW.MECH_Mid_Angle > 4096)
  {
    Gimbal.YAW.MECH_Mid_Angle -= 4096;
  }
  else 
  {
    Gimbal.YAW.MECH_Mid_Angle += 4096;
  }
}
void YAW_MECH_Mid_Angle_Proc(void)
{
  //就近头尾回正//调用此函数可以就头尾回正    
  int16_t Proc_Angle = 0;
  Proc_Angle = YAW_MotorAngle_Proc(Gimbal.YAW.motor_data.CAN_GetData.Motor_Angle);
  if(abs(Gimbal.YAW.MECH_Mid_Angle - Proc_Angle) > Motor_90){
    CHAS_Turn_180();
  }
}
/* ----------------头部反向-------------------- */
/**
 * @brief 角度期望改变
 * @param 
 */
void GimReverseTurn(void)
{
  if(Gimbal.Turn_Info.GIM_Reverse.IF_ReverseOver == true)
  {
    if(Gimbal.YAW.MECH_Mid_Angle >=4096)
      Gimbal.YAW.MECH_Mid_Angle -= 4096;//陀螺的变化
    else if(Gimbal.YAW.MECH_Mid_Angle < 4096)
      Gimbal.YAW.MECH_Mid_Angle += 4096;//陀螺的变化      
    Gimbal.Turn_Info.GIM_Reverse.IF_ReverseOver = false;
    Gimbal.Turn_Info.GIM_Reverse.IF_TurnReverse = true;
  }
}
/**
 * @brief 判断是否转向完毕
 * @param 
 */
void Judge_IF_ReverseOver(void)
{
  Gim_Reverse_t *str = &Gimbal.Turn_Info.GIM_Reverse;
  if(str->IF_ReverseOver == false)
  {
    if(GIM_Get_AngleErr(&Gimbal.YAW)<50)
      str->Over_cnt++;
    str->cnt++;
    if(str->Over_cnt >= 60 || str->cnt > 2500)
    {
      str->IF_ReverseOver = true;
      str->IF_TurnReverse = false;
    }
  }
}
/**
 * @brief 判断GIM是否出现了转向
 * @note  为了反馈给系统
 */
bool prev_IF_TurnReverse = false;
bool Report_IF_Reverse(void)
{
  bool res = false;
  Gim_Reverse_t *str = &Gimbal.Turn_Info.GIM_Reverse;
  if(prev_IF_TurnReverse != str->IF_TurnReverse)
  {
    if(prev_IF_TurnReverse == true && str->IF_TurnReverse == false)
    //采集下降沿信号
      res = true;
    prev_IF_TurnReverse = str->IF_TurnReverse;
  }
  return res;
}

//------------------------------平移时左右转------------------
bool W_move = NO,S_move = NO,A_move = NO,D_move = NO;
void UserSet_W_move(bool a)
{
  W_move = a;
}
void UserSet_S_move(bool a)
{
  S_move = a;
}
void UserSet_A_move(bool a)
{
  A_move = a;
}
void UserSet_D_move(bool a)
{
  D_move = a;
}
void HalfMove_KeyReset(void)
{
  W_move = NO;
  S_move = NO;
  A_move = NO;
  D_move = NO;
}
bool IF_CarHalfMove = NO , IF_MECH_StopHalfMove = NO , IF_SPIN_StopHalfMove = NO;
void Judge_HalfMove_type(void)
{
  Gimbal.HalfMove_type = Normal_yaw;
  if(!IF_MECH_StopHalfMove)
  if(!IF_SPIN_StopHalfMove)
  if(IF_CarHalfMove){//开启了斜向才进行判断
    
    if(A_move)
    {
      Gimbal.HalfMove_type = Normal_yaw;
      if(W_move)Gimbal.HalfMove_type = LF_yaw;
      if(S_move)Gimbal.HalfMove_type = RF_yaw;
    }
    if(D_move)
    {
      Gimbal.HalfMove_type = Normal_yaw;
      if(W_move)Gimbal.HalfMove_type = RF_yaw;
      if(S_move)Gimbal.HalfMove_type = LF_yaw;
    }
    if((A_move && D_move) || (!A_move && !D_move))//AD全按下或者全没按下都判断为普通模式
      Gimbal.HalfMove_type = Normal_yaw;
  
  }
}
void Reset_IF_MECH_StopHalfMove(void)
{//退出机械模式开启斜向
  IF_MECH_StopHalfMove = NO;
}
void Set_IF_MECH_StopHalfMove(void)
{//进入机械模式关闭斜向
  IF_MECH_StopHalfMove = YES;
}
void Reset_IF_SPIN_StopHalfMove(void)
{//退出陀螺模式开启斜向
  IF_SPIN_StopHalfMove = NO;
}
void Set_IF_SPIN_StopHalfMove(void)
{//进入陀螺模式关闭斜向
  IF_SPIN_StopHalfMove = YES;
}
void Reset_IF_CarHalfMove(void)
{//关闭斜向总开关
  IF_CarHalfMove = NO;
}
void Set_IF_CarHalfMove(void)
{//开启斜向总开关
  IF_CarHalfMove = YES;
}
void Reset_CarHalfMove(void)
{//整体复位，初始为关闭状态
  Reset_IF_SPIN_StopHalfMove();
  Reset_IF_MECH_StopHalfMove();
  Reset_IF_CarHalfMove();
  HalfMove_KeyReset();//复位斜向模式的按键反馈，此时按键显示全部抬起
}
void IF_CarHalfMove_Switch(void)
{//跳变开关
  switch(IF_CarHalfMove)
  {
    case NO:
      Set_IF_CarHalfMove();
      break;
    case YES:
      Reset_IF_CarHalfMove();
      break;
  }
}
void SystemSet_IF_CarHalfMove(void)
{//判断斜向方向
  if(CHAS_SPIN_Report())//小陀螺
    Set_IF_SPIN_StopHalfMove();//阻止斜向
  else 
    Reset_IF_SPIN_StopHalfMove();//开启斜向
  
//  if(IF_MECH_StopHalfMove || IF_SPIN_StopHalfMove) //机械模式 或 小陀螺
//    Reset_IF_CarHalfMove();//关闭斜向
//  else 
//    Set_IF_CarHalfMove();
  
  Judge_HalfMove_type();
}
float Get_CarHalfMove_Angle(void)
{//获取斜向角度
  float res = 0;
  switch(Gimbal.HalfMove_type)
  {
    case Normal_yaw:
        res = 0;
        break;
        
    case L_yaw:
        res = -Turn_HalfMove;
        break;
    
    case R_yaw:
        res = Turn_HalfMove;
        break;
    
    case LF_yaw:
        res = -Turn_HalfMove;
        break;
    
    case RF_yaw:
        res = Turn_HalfMove;
        break;
  }
  return res;
}


//------------------------------------------------------------------

/**
 * @brief 获取IMU数据
 * @param 
 */
void IMU_YawData_Report(Motor_Data_t *str)
{
  str->IMU_GetData.IMU_Speed = Report_YawSpeed();
  str->IMU_GetData.IMU_Angle = Report_YawAngle();
}
void IMU_PitData_Report(Motor_Data_t *str)
{
  str->IMU_GetData.IMU_Speed = Report_PitSpeed();
  str->IMU_GetData.IMU_Angle = Report_PitAngle();
}
void IMU_Data_Report(void)
{
  IMU_YawData_Report(&Gimbal.YAW.motor_data);
  IMU_PitData_Report(&Gimbal.PIT.motor_data);
}


/**
 * @brief 底盘PID数据全部清0函数
 * @param 
 */
void GIM_SetEmpty(PID_Info_t *str)
{
  memset(&str->Speed_Loop,0,sizeof(PID_Loop_t));
  memset(&str->Angle_Loop,0,sizeof(PID_Loop_t));
}
 
/**
 * @brief 卸力函数
 * @param 
 */
void GIM_Stop(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
	
	/* 速度环最终输出 */
  pid_out[CHAS_LF] = 0;
  pid_out[CHAS_RF] = 0;
  
	CAN1_Send(GIM_ESC, pid_out);	  
}


/**
 * @brief 初始化陀螺仪模式YAW轴角度
 * @param 
 */
void GIM_GYRO_FirstYawAngle(AXIS_Info_t *str)
{
  str->motor_data.PID_Angle_target = str->motor_data.IMU_GetData.IMU_Angle;
}
/**
 * @brief 初始化机械模式YAW轴角度
 * @param 
 */
void GIM_MECH_FirstYawAngle(AXIS_Info_t *str)
{
  str->motor_data.PID_Angle_target = str->MECH_Mid_Angle;
}
/**
 * @brief 初始化PIT轴角度
 * @param 
 */
void GIM_FirstPitAngle(AXIS_Info_t *str)
{
  //str->motor_data.PID_Angle_target = str->motor_data.CAN_GetData.Motor_Angle;
  str->motor_data.PID_Angle_target = str->MECH_Mid_Angle;
}
void GIM_GYRO_FirstPitAngle(AXIS_Info_t *str)
{
  str->motor_data.PID_Angle_target = str->motor_data.IMU_GetData.IMU_Angle;
}
void GIM_MECH_FirstPitAngle(AXIS_Info_t *str)
{
  str->motor_data.PID_Angle_target = PIT_MotorAngle_Proc(str->motor_data.CAN_GetData.Motor_Angle);
}

//-----------获取误差-----------
/**
 * @brief 获取对应轴电机当前角度环的Err
 * @note  加入卡尔曼滤波 
 */
float GIM_Get_AngleErr(AXIS_Info_t *str)
{
  float res;
  res = str->motor_data.PID_Angle_target - str->motor_data.PID_Angle;
  res = KalmanFilter(&str->KF_Angle[str->PID.PID_type],res);//卡尔曼滤波
  return res;
}

/**
 * @brief 获取对应轴电机当前速度环的Err
 * @note  没加卡尔曼滤波，加了之后效果不好 
 */
float GIM_Get_SpeedErr(AXIS_Info_t *str)
{
  float res;
  res = str->motor_data.PID_Speed_target - str->motor_data.PID_Speed;
  return res;
}

/**
 * @brief YAW电机与机械中值角度误差
 * @note  使用电机的角度
 */
float YAW_AngleErrReport(void)
{
  float res;
  AXIS_Info_t *str = &(Gimbal.YAW);
  Motor_Data_t *DATA = &(str->motor_data);
  
  res = str->MECH_Mid_Angle - //下面的yaw电机加入角度处理函数
         YAW_MotorAngle_Proc((float)DATA->CAN_GetData.Motor_Angle)  ;
  return res;
}

/**
 * @brief PIT电机与机械中值角度误差 
 * @note  使用电机的角度
 */
float PIT_AngleErrReport(void)
{
  float res;
  AXIS_Info_t *str = &(Gimbal.PIT);
  Motor_Data_t *DATA = &(str->motor_data);
  
  res = str->MECH_Mid_Angle - 
         PIT_MotorAngle_Proc((float)DATA->CAN_GetData.Motor_Angle)  ;  
  return res;
}

/**
 * @brief 得到yaw轴陀螺仪角度值
 */
float YAW_GetGyroAngle(void)
{
  return Gimbal.YAW.motor_data.PID_Angle;
}

/**
 * @brief 得到pitch轴陀螺仪角度值
 */
float PIT_GetGyroAngle(void)
{
  return Gimbal.PIT.motor_data.PID_Angle;
}

/**
 * @brief 得到pitch电机机械中值
 */
float PIT_GetMechAngle(void)
{
  return (float)Gimbal.PIT.motor_data.CAN_GetData.Motor_Angle;
}

/**
 * @brief 击打哨兵快速抬头
 */
void Gim_Sentry_UP(void)
{
  Gimbal.PIT.motor_data.PID_Angle_target = PIT_SENTRY;//PIT_MOTOR_MIN_ANGLE + 200;
}

/**
 * @brief 上报是否出现NAN
 */
bool GIM_Report_IF_NAN(void)
{
  return Gimbal.IF_NAN;
}

