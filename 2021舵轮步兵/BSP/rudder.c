/***      《 舵轮控制代码 》

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
 
#include "rudder.h"
#include "Device.h"

/* Init start */
Rudder_Info_t Rudder = {
  .ctrl_mode = Ctrl_Err,
  .pid_mode = PID_Err,
  .action_mode = ACT_Err,
  .PID_type = Turn_off,

  .Rudder_Axis[RUD_LF].MECH_Mid_Angle = LF_F_MidAngle,
  .Rudder_Axis[RUD_RF].MECH_Mid_Angle = RF_F_MidAngle,
  .Rudder_Axis[RUD_LB].MECH_Mid_Angle = LB_F_MidAngle,
  .Rudder_Axis[RUD_RB].MECH_Mid_Angle = RB_F_MidAngle,
  
};//舵轮控制信息结构体

/* Init end   */
static System_Ctrl_Mode_t R_prev_ctrl_mode = Ctrl_Err;
static PID_type_t R_prev_pid_type = Turn_off;
/* --------------------------任务层------------------------- */
/**
 * @brief 舵轮总控
 * @param 
 */
void RUDDER_Ctrl(void)
{
  if(GIM_CalOK_Report())//云台复位成功之后再启动舵轮
  switch (Rudder.ctrl_mode)
  {
    case RC_CTRL_MODE:
      RUD_RC_Ctrl();
      break;
    case KEY_CTRL_MODE: 
      RUD_KEY_Ctrl();
      break;
  }
  RUD_Output();
}

/**
 * @brief 舵轮信息获取
 * @param 
 */
void RUD_GET_Info(void)
{
  Rudder.ctrl_mode = System.ctrl_mode;
  Rudder.pid_mode = System.pid_mode;
  Rudder.action_mode = System.action_mode;
  Rudder.PID_type = GET_PID_Type();
  Axis_Move_Angle_Init();
  Axis_SPIN_Angle_Init();//旋转角初始化
}




/**
 * @brief PID种类获取
 * @note  本地同步
 */
void RUD_GET_PID_Type(PID_Info_t *str )
{
  str->PID_type = Rudder.PID_type;
}
void RUD_SetPID_type(void)
{
  RUD_GET_PID_Type(&Rudder.Rudder_Axis[RUD_LF].PID);
  
  RUD_GET_PID_Type(&Rudder.Rudder_Axis[RUD_RF].PID);
  
  RUD_GET_PID_Type(&Rudder.Rudder_Axis[RUD_LB].PID); 
  
  RUD_GET_PID_Type(&Rudder.Rudder_Axis[RUD_RB].PID);  
}

 /**
 * @brief 舵轮电机PID设置总函数
 * @param 
 */
void RUD_SetPID(void)
{
  RUD_SetPID_type();
  
  RUD_PID_Switch(&Rudder.Rudder_Axis[RUD_LF].PID);
  
  RUD_PID_Switch(&Rudder.Rudder_Axis[RUD_RF].PID);
  
  RUD_PID_Switch(&Rudder.Rudder_Axis[RUD_LB].PID);
  
  RUD_PID_Switch(&Rudder.Rudder_Axis[RUD_RB].PID);
}

 
/**
 * @brief PID切换
 * @param 
 */
void RUD_PID_Switch(PID_Info_t *str )
{
  static int length = sizeof(PID_Parameter_t);
  switch (str->PID_type)
  {   
    case Turn_off :{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_RUDDER][Turn_off],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_RUDDER][Turn_off],length);
    }break;  

    default :{
      memcpy(&(str->Speed_Loop.PID_Param),&PID_Speed_Param[GM_6020_RUDDER][RC_GYRO],length);
      memcpy(&(str->Angle_Loop.PID_Param),&PID_Angle_Param[GM_6020_RUDDER][RC_GYRO],length);
    }break; 
  }
}


/**
 * @brief 舵轮复位函数
 * @param 
 */
void RUD_Reset(void)
{
  if(R_prev_ctrl_mode != Rudder.ctrl_mode)
  {/* 第一次进入复位模式 */
    R_prev_ctrl_mode = Rudder.ctrl_mode;
    /*清空数据*/
    RUD_SetEmpty(&Rudder.Rudder_Axis[RUD_LF].PID);
    RUD_SetEmpty(&Rudder.Rudder_Axis[RUD_RF].PID);
    RUD_SetEmpty(&Rudder.Rudder_Axis[RUD_LB].PID);
    RUD_SetEmpty(&Rudder.Rudder_Axis[RUD_RB].PID);
  }
  
  if(R_prev_pid_type != Rudder.PID_type)
  {/* 第一次进入复位模式 */
    R_prev_pid_type = Rudder.PID_type;
    RUD_SetPID();//更新PID状态
  }
  
  RUD_Stop();//卸力
}



/**
 * @brief 舵轮遥控控制
 * @param 
 */
void RUD_Remote_Ctrl(void)
{
//  /*PID参数调试*/
//  PID_Debug(&Rudder.Rudder_Axis[RUD_LF].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_RF].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_LB].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_RB].PID);
  
    
    switch(Rudder.PID_type)
    {
      case RC_GYRO:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , RC_CH3 , 0.7 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , RC_CH2 , 0.7 , 660);
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*CHAS_CHASE_K , 
                 1.2 , 660 * CHAS_CHASE_K);// 1.7
//        Rudder.Vector.X_speed = RC_CH3;
//        Rudder.Vector.Y_speed = RC_CH2;
//        Rudder.Vector.Z_speed = Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*CHAS_CHASE_K;
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        
        
        break;
      
      case RC_MECH:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , RC_CH3 , 0.7 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , RC_CH2 , 0.7 , 660);
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(RC_CH0), 1.5 , 660);
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_MECH();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_MECH();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        
        
        break;
    }
}
/**
 * @brief 舵轮遥控控制
 * @param 
 */
void RUD_RC_Ctrl(void)
{
  if(R_prev_ctrl_mode != Rudder.ctrl_mode)
  {/* 第一次进入RC模式 */
    R_prev_ctrl_mode = Rudder.ctrl_mode;
    RUD_ParamInit();
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_LF]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_RF]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_LB]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_RB]);
  }
  switch (CHAS_SPIN_Report())
  {
    case false:
      RUD_Remote_Ctrl();
      break;
    case true:
      RUD_SPIN();
      break; 
  }
}
/**
 * @brief 舵轮键盘控制
 * @param 
 */
void RUD_Keyboard_Ctrl(void)
{
//  /*PID参数调试*/
//  PID_Debug(&Rudder.Rudder_Axis[RUD_LF].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_RF].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_LB].PID);
//  PID_Debug(&Rudder.Rudder_Axis[RUD_RB].PID);
  
    switch(Rudder.PID_type)
    {
      case KEY_GYRO:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , KEY_FB_Ctrl() , 1.2 , 660); //1 
        RUD_Slow(&Rudder.Vector.Y_speed , KEY_LR_Ctrl() , 0.9 , 660); //0.8
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*CHAS_CHASE_K ,
                  1.1 , 660 * CHAS_CHASE_K);//1.3
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        
        
        break;
      
      case KEY_MECH:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , KEY_FB_Ctrl() , 1 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , KEY_LR_Ctrl() , 0.8 , 660);
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(Mouse_X_Speed()), 1.5 , 660);;
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_MECH();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_MECH();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        
        
        break;
    
    case ACT_AUTO_AIM:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , KEY_FB_Ctrl() , 1.2 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , KEY_LR_Ctrl() , 0.9 , 660);
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*CHAS_CHASE_K ,
                  1.3 , 660 * CHAS_CHASE_K);
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        
        break;

    case ACT_SMALL_BUFF:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        Rudder.Vector.X_speed = 0;//KEY_FB_Ctrl();
        Rudder.Vector.Y_speed = 0;//KEY_LR_Ctrl();
        Rudder.Vector.Z_speed = 0;//Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*5;
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        break;
    
    case ACT_BIG_BUFF:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        Rudder.Vector.X_speed = 0;//KEY_FB_Ctrl();
        Rudder.Vector.Y_speed = 0;//KEY_LR_Ctrl();
        Rudder.Vector.Z_speed = 0;//Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*5;
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        break;
    
    case ACT_PARK:
        if(R_prev_pid_type != Rudder.PID_type)
        {/* 第一次进入模式 */
          R_prev_pid_type = Rudder.PID_type;
          RUD_SetPID();
        }
        RUD_Slow(&Rudder.Vector.X_speed , KEY_FB_Ctrl() , 1.7 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , KEY_LR_Ctrl() , 1.7 , 660);
        RUD_Slow(&Rudder.Vector.Z_speed , Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*CHAS_CHASE_K ,
                  1.3 , 660 * CHAS_CHASE_K);
        
        /* 得到舵轮的真实前进角度 */
        RUD_Get_F_Angle_GYRO();
        
        /* 得到舵轮的真实旋转角度 */
        RUD_Get_Z_Angle_GYRO();
        
        /* 得到舵轮XYZ平面的矢量角 */
        RUD_Angle_Fusion_XYZ();
        break;
  }
}
/**
 * @brief 舵轮键盘控制
 * @param 
 */
void RUD_KEY_Ctrl(void)
{
  if(R_prev_ctrl_mode != Rudder.ctrl_mode)
  {/* 第一次进入RC模式 */
    R_prev_ctrl_mode = Rudder.ctrl_mode;
    RUD_ParamInit();
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_LF]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_RF]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_LB]);
    RUD_FirstAngle(&Rudder.Rudder_Axis[RUD_RB]);
  }
  switch (CHAS_SPIN_Report())
  {
    case false:
      RUD_Keyboard_Ctrl();
      break;
    case true:
      RUD_SPIN();
      break; 
  }
}



/* --------------------------功能层------------------------- */
/**
 * @brief 对YAW电机角度进行处理
 * @note  把 机械模式、陀螺仪底盘跟随、小陀螺回正 所用到的yaw电机角度
          都进行处理
 */
float RUD_MotorAngle_Proc(Rudder_Axis_Info_t *str)
{
  int16_t Angle = str->motor_data.CAN_GetData.Motor_Angle;
  
  if(Angle <= str->MECH_Mid_Angle - Motor_180)
    Angle += Motor_360;
  if(Angle >  str->MECH_Mid_Angle + Motor_180)
    Angle -= Motor_360;   

  return (float)Angle;
}
//用于在融合XYZ速度时取xy前进角和Z旋转角的误差
float RUD_DirAngleErr_Proc(float xy,float z)
{
  if(z <= xy - Motor_180)
    z += Motor_360;
  if(z >  xy + Motor_180)
    z -= Motor_360;
  
  return (z - xy);
}
/*将atan得出来的结果由0~360转化为*/
float RUD_Z_atanAngle_Proc(float Angle)
{
  if(Angle > 4096)Angle = Angle - 8192;
  return Angle;
}
//将角度值转化为0~8192之间的值
float RUD_DirAngle_Proc(int16_t Angle)
{
  while (Angle > 8192 || Angle < 0)
  {
    if(Angle < 0)
      Angle += Motor_360;
    if(Angle > 8192)
      Angle -= Motor_360; 
  }
  return (float)Angle;
}
//在0~8192之间计算pid误差时，要取就近误差，在PID_GetAngleErr中调用
float RUD_PIDAngleTarget_Proc(Rudder_Axis_Info_t *str)
{
  float Target_Angle = str->motor_data.PID_Angle_target;
    
  if(Target_Angle <= str->motor_data.PID_Angle - Motor_180 - 40)//非半圆划分,为了在180度转的时候同向
    Target_Angle += Motor_360;
  if(Target_Angle >  str->motor_data.PID_Angle + Motor_180 - 40)//非半圆划分
    Target_Angle -= Motor_360;   
  
  str->motor_data.PID_Angle_target = Target_Angle;

  return Target_Angle;
}

void Axis_Move_Angle_Init(void)//由底盘头尾方向同步确定舵轮头尾方向
{
  if(Judge_IF_CHAS_Dir_HEAD())
  {
    Rudder.Rudder_Axis[RUD_LF].MECH_Mid_Angle = LF_F_MidAngle;
    Rudder.Rudder_Axis[RUD_RF].MECH_Mid_Angle = RF_F_MidAngle;
    Rudder.Rudder_Axis[RUD_LB].MECH_Mid_Angle = LB_F_MidAngle;
    Rudder.Rudder_Axis[RUD_RB].MECH_Mid_Angle = RB_F_MidAngle;
  }
  if(Judge_IF_CHAS_Dir_TAIL()) 
  {
    Rudder.Rudder_Axis[RUD_LF].MECH_Mid_Angle = RUD_DirAngle_Proc(LF_F_MidAngle +4096);
    Rudder.Rudder_Axis[RUD_RF].MECH_Mid_Angle = RUD_DirAngle_Proc(RF_F_MidAngle +4096);
    Rudder.Rudder_Axis[RUD_LB].MECH_Mid_Angle = RUD_DirAngle_Proc(LB_F_MidAngle +4096);
    Rudder.Rudder_Axis[RUD_RB].MECH_Mid_Angle = RUD_DirAngle_Proc(RB_F_MidAngle +4096);
  }
}

void Axis_SPIN_Angle_Init(void)//对各个轴的旋转角度做一个同步
{
  Rudder.Rudder_Axis[RUD_LF].SPIN_Mid_Angle = RUD_DirAngle_Proc(Rudder.Rudder_Axis[RUD_LF].MECH_Mid_Angle + 1024 + 0);
  Rudder.Rudder_Axis[RUD_RF].SPIN_Mid_Angle = RUD_DirAngle_Proc(Rudder.Rudder_Axis[RUD_RF].MECH_Mid_Angle + 1024 + 2048);
  Rudder.Rudder_Axis[RUD_LB].SPIN_Mid_Angle = RUD_DirAngle_Proc(Rudder.Rudder_Axis[RUD_LB].MECH_Mid_Angle + 1024 - 2048);
  Rudder.Rudder_Axis[RUD_RB].SPIN_Mid_Angle = RUD_DirAngle_Proc(Rudder.Rudder_Axis[RUD_RB].MECH_Mid_Angle - 1024 - 2048);
}


//----------------舵轮底盘速度矢量合成----------------//
//判断最后一次转舵是否为纯旋转角
bool Judge_IF_SPIN_Dir(void)
{
  bool res = NO;
  if( abs(Rudder.Rudder_Axis[RUD_LF].RUD_Move.Dir - Rudder.Rudder_Axis[RUD_LF].RUD_Move.Z_dir) < 5 &&
      abs(Rudder.Rudder_Axis[RUD_RF].RUD_Move.Dir - Rudder.Rudder_Axis[RUD_RF].RUD_Move.Z_dir) < 5 &&
      abs(Rudder.Rudder_Axis[RUD_LB].RUD_Move.Dir - Rudder.Rudder_Axis[RUD_LB].RUD_Move.Z_dir) < 5 &&
      abs(Rudder.Rudder_Axis[RUD_RB].RUD_Move.Dir - Rudder.Rudder_Axis[RUD_RB].RUD_Move.Z_dir) < 5 )
  res = YES;
  
  Rudder.Rudder_Axis[RUD_LF].RUD_Move.IF_SPIN_Dir = res;
  Rudder.Rudder_Axis[RUD_RF].RUD_Move.IF_SPIN_Dir = res;
  Rudder.Rudder_Axis[RUD_LB].RUD_Move.IF_SPIN_Dir = res;
  Rudder.Rudder_Axis[RUD_RB].RUD_Move.IF_SPIN_Dir = res;
  
  return res;
}
//在舵角方向不纯为旋转方向时，限定一个速度死区  该死区和旋转变舵死区一致
float Movedir_Z_speed_Proc(float CH0)
{
  float limitErr = SPIN_DIR_Angle;
  if(!Judge_IF_SPIN_Dir())//不纯为旋转方向
  {
    if(Rudder.Vector.XY_Fusion_speed >= FB_DIR_speed_Limit+1)//若是在移动，则需要旋转变舵比较灵敏
      limitErr = 0;
    if(abs(CH0) < limitErr)
      CH0 = 0;
  }
  return CH0;
}
/**
 * @brief 求合速度
 */
float Get_Fusion_Speed_XY(float CH3 , float CH2 )//XY速度
{
  float res;
  res = sqrt(CH3*CH3 + CH2*CH2);//平方和的开根号
  Rudder.Vector.XY_Fusion_speed = res;//记录合速度
  return res;
}
void Get_Fusion_Speed_XYZ(float XY , float Z , Rudder_Axis_Info_t *str)//最终合速度
{
  float res;
  bool SPIN;
  SPIN = Judge_IF_SPIN_Dir();
  
  if(!SPIN)
    res = sqrt(XY*XY + Z*Z);//平方和的开根号
  else 
    res = Rudder.Vector.Z_speed * (float)str->RUD_Move.Z_LR;//单纯旋转时允许反转
  
  str->RUD_Move.XYZ_Fusion_Speed = res;//记录合速度
}
/**
 * @brief 判断是否高速
 */
bool IF_RUNNING = NO;
bool Report_IF_RUNNING(void)
{
  return IF_RUNNING;
}
void Judge_IF_RUNNING( float speed)
{
  if(abs(speed) > Report_speed_Limit + 50)
    IF_RUNNING = YES;
  else
    IF_RUNNING = NO;
}
/**
 * @brief 返回矢量速度，带死区
 */
float Report_Fusion_Speed_LF(void)
{
  float res = Rudder.Rudder_Axis[RUD_LF].RUD_Move.XYZ_Fusion_Speed;
  if(abs(res) <= Report_speed_Limit && !Rudder.Rudder_Axis[RUD_LF].RUD_Move.IF_SPIN_Dir)
    res = 0;
  return res;
}
float Report_Fusion_Speed_RF(void)
{
  float res = Rudder.Rudder_Axis[RUD_RF].RUD_Move.XYZ_Fusion_Speed;
  if(abs(res) <= Report_speed_Limit && !Rudder.Rudder_Axis[RUD_LF].RUD_Move.IF_SPIN_Dir)
    res = 0;
  return res;
}
float Report_Fusion_Speed_LB(void)
{
  float res = Rudder.Rudder_Axis[RUD_LB].RUD_Move.XYZ_Fusion_Speed;
  if(abs(res) <= Report_speed_Limit && !Rudder.Rudder_Axis[RUD_LF].RUD_Move.IF_SPIN_Dir)
    res = 0;
  return res;
}
float Report_Fusion_Speed_RB(void)
{
  float res = Rudder.Rudder_Axis[RUD_RB].RUD_Move.XYZ_Fusion_Speed;
  if(abs(res) <= Report_speed_Limit && !Rudder.Rudder_Axis[RUD_LF].RUD_Move.IF_SPIN_Dir)
    res = 0;
  return res;
}




/**
 * @brief  得到XY偏航角
 */
float Get_dir_Angle_XY(void)//往左减小0~ -180   ，   往右增大0~180
{
  float CH3 = (float)Rudder.Vector.X_speed;
  float CH2 = (float)Rudder.Vector.Y_speed;
  float res = 0;
  static float prev_res = 0;
  
  //XY面
  if(Get_Fusion_Speed_XY(CH3,CH2) >= FB_DIR_speed_Limit)
  {
    if(CH2 > 0)
    {
      if(CH3 > 0)
        res = (atan2(CH2,CH3))*180/3.14;
      if(CH3 < 0)
        res = (atan2(-CH3,CH2))*180/3.14 + 90;
      if(CH3 == 0)
        res = 90;
    }
    
    if(CH2 == 0)
    {
      if(CH3 > 0)
        res = 0;
      if(CH3 < 0)
        res = -180;
    }
    
    
    if(CH2 < 0)
    {
      if(CH3 > 0)
        res = (atan2(CH2,CH3))*180/3.14;
      if(CH3 < 0)
        res = -90 - (atan2(-CH3,-CH2))*180/3.14;
      if(CH3 == 0)
        res = -90;
    }
    
    prev_res = res;
  }
  else 
  {
    res = prev_res;
  }
  
  return res*4096/180;
}

/**
 * @brief 根据yaw轴机械中值角度差，确定真实前进方向角
 * @note  实现陀螺仪模式的“快速全向”（麦轮底盘的全向）
 */
//陀螺仪模式----------------------------------------------------
/**
 * @brief 融合xy轴前进角
 */
void RUD_Get_Axis_Z_Angle_GYRO(Rudder_Axis_Info_t *str)
{ 
  Judge_Z_LR_Dir(str , Rudder.Vector.Z_speed);
  if(str->RUD_Move.Z_LR == RIGH)
    str->RUD_Move.Z_dir = RUD_DirAngle_Proc(str->SPIN_Mid_Angle );//换算为0~8192区间内的值
  else
    str->RUD_Move.Z_dir = RUD_DirAngle_Proc(str->SPIN_Mid_Angle  + 4096);//换算为0~8192区间内的值
}
void RUD_Get_Axis_F_Angle_GYRO(Rudder_Axis_Info_t *str)
{
  str->RUD_Move.F_dir = RUD_DirAngle_Proc(str->MECH_Mid_Angle + YAW_AngleErrReport());//+ YAW_AngleErrReport());//换算为0~8192区间内的值
} 
void RUD_Get_F_Angle_GYRO(void)
{
  RUD_Get_Axis_F_Angle_GYRO(&Rudder.Rudder_Axis[RUD_LF]);
  RUD_Get_Axis_F_Angle_GYRO(&Rudder.Rudder_Axis[RUD_RF]);
  RUD_Get_Axis_F_Angle_GYRO(&Rudder.Rudder_Axis[RUD_LB]);
  RUD_Get_Axis_F_Angle_GYRO(&Rudder.Rudder_Axis[RUD_RB]);
}
void RUD_Get_Z_Angle_GYRO(void)
{
  RUD_Get_Axis_Z_Angle_GYRO(&Rudder.Rudder_Axis[RUD_LF]);
  RUD_Get_Axis_Z_Angle_GYRO(&Rudder.Rudder_Axis[RUD_RF]);
  RUD_Get_Axis_Z_Angle_GYRO(&Rudder.Rudder_Axis[RUD_LB]);
  RUD_Get_Axis_Z_Angle_GYRO(&Rudder.Rudder_Axis[RUD_RB]);
}

//机械模式----------------------------------------------------
void RUD_Get_Axis_Z_Angle_MECH(Rudder_Axis_Info_t *str)
{
  Judge_Z_LR_Dir(str , Rudder.Vector.Z_speed);
  if(str->RUD_Move.Z_LR == RIGH)
    str->RUD_Move.Z_dir = RUD_DirAngle_Proc(str->SPIN_Mid_Angle );//换算为0~8192区间内的值
  else
    str->RUD_Move.Z_dir = RUD_DirAngle_Proc(str->SPIN_Mid_Angle + 4096);//换算为0~8192区间内的值
}
void RUD_Get_Axis_F_Angle_MECH(Rudder_Axis_Info_t *str)
{
  str->RUD_Move.F_dir = RUD_DirAngle_Proc(str->MECH_Mid_Angle);//换算为0~8192区间内的值
} 
void RUD_Get_F_Angle_MECH(void)
{
  RUD_Get_Axis_F_Angle_MECH(&Rudder.Rudder_Axis[RUD_LF]);
  RUD_Get_Axis_F_Angle_MECH(&Rudder.Rudder_Axis[RUD_RF]);
  RUD_Get_Axis_F_Angle_MECH(&Rudder.Rudder_Axis[RUD_LB]);
  RUD_Get_Axis_F_Angle_MECH(&Rudder.Rudder_Axis[RUD_RB]);
}
void RUD_Get_Z_Angle_MECH(void)
{
  RUD_Get_Axis_Z_Angle_MECH(&Rudder.Rudder_Axis[RUD_LF]);
  RUD_Get_Axis_Z_Angle_MECH(&Rudder.Rudder_Axis[RUD_RF]);
  RUD_Get_Axis_Z_Angle_MECH(&Rudder.Rudder_Axis[RUD_LB]);
  RUD_Get_Axis_Z_Angle_MECH(&Rudder.Rudder_Axis[RUD_RB]);
}
//判断左右旋转方向
void Judge_Z_LR_Dir(Rudder_Axis_Info_t *str , float CH0)
{
  float limitErr = SPIN_DIR_Angle;
  
  if(Rudder.Vector.XY_Fusion_speed >= FB_DIR_speed_Limit+1)//若是在移动，则需要旋转变舵比较灵敏
    limitErr = 0;
  
  //旋转转舵必须大于一定数值
//  if(!CHAS_SPIN_Report())//非小陀螺状态下允许转舵
//  {
    if(CH0 < -limitErr)
    {
      Rudder.Vector.Z_LR = LEFT;
      str->RUD_Move.Z_LR = LEFT;
    }
    if(CH0 > limitErr)
    {
      Rudder.Vector.Z_LR = RIGH;
      str->RUD_Move.Z_LR = RIGH;
    }
//  }
}

/**----------------------------------------------------
 * @brief Z轴融合算法
 * @param Rudder_Axis_Info_t 为舵轮舵电机信息结构体，一共4个舵轮电机
 */
float Get_dir_Angle_Z(Rudder_Axis_Info_t *str)
{
  float res;
  float XY_Z_Angle;
  float b,a;// atan(b/a)
  
  XY_Z_Angle = RUD_DirAngleErr_Proc(str->RUD_Move.XY_dir , str->RUD_Move.Z_dir);//XY合成角与Z旋转角之间的夹角
  
  /* 具体算法分析见doc文件图示 */
  b = (abs(Rudder.Vector.Z_speed) * arm_sin_f32(XY_Z_Angle *2*3.1415926f/8192.f));//tan 对边
  a = (Rudder.Vector.XY_Fusion_speed + (abs(Rudder.Vector.Z_speed) * arm_cos_f32(XY_Z_Angle *2*3.1415926f/8192.f))); //tan邻边

  res =  atan2( b , a )*8192/3.1415926f /2;

  if(Rudder.Vector.XY_Fusion_speed != 0 || Rudder.Vector.Z_speed != 0 )
    str->RUD_Move.prev_Z_angle = res;
  if(Rudder.Vector.XY_Fusion_speed == 0 && Rudder.Vector.Z_speed == 0 )
    res = str->RUD_Move.prev_Z_angle;//ab都为0，则停止运动，使用上一次旋转舵角  

  str->RUD_Move.XY_Z_Angle  = XY_Z_Angle;//用于调试

  Get_Fusion_Speed_XYZ(a,b,str);
  
  return res;
}

/**----------------------------------------------------
 * @brief 合成矢量角误差，用于得到舵轮矢量角
 */
void RUD_Axis_Angle_Fusion_XYZ(Rudder_Axis_Info_t *str)
{
  str->RUD_Move.XY_dir = RUD_DirAngle_Proc(str->RUD_Move.F_dir + Get_dir_Angle_XY());//换算为0~8192区间内的值 得到了XY方向前进角
  str->RUD_Move.Dir = RUD_DirAngle_Proc(str->RUD_Move.XY_dir + Get_dir_Angle_Z(str));//融合了Z方向之后得到的全向角
  str->motor_data.PID_Angle_target = str->RUD_Move.Dir;//得到PID_AngleTarget
}
void RUD_Angle_Fusion_XYZ(void)
{
  RUD_Axis_Angle_Fusion_XYZ(&Rudder.Rudder_Axis[RUD_LF]);
  RUD_Axis_Angle_Fusion_XYZ(&Rudder.Rudder_Axis[RUD_RF]);
  RUD_Axis_Angle_Fusion_XYZ(&Rudder.Rudder_Axis[RUD_LB]);
  RUD_Axis_Angle_Fusion_XYZ(&Rudder.Rudder_Axis[RUD_RB]);
}

//PID计算输出区---------------
/**
 * @brief 同步CAN得到的数据和即将参与PID计算的数据
 */
void RUD_DataSync(Rudder_Axis_Info_t *str)
{
  str->motor_data.PID_Speed = str->motor_data.CAN_GetData.Motor_Speed;
//  str->motor_data.PID_Angle = RUD_MotorAngle_Proc(str);
  str->motor_data.PID_Angle = str->motor_data.CAN_GetData.Motor_Angle;
}
//-----------获取误差-----------
/**
 * @brief 获取对应轴电机当前角度环的Err
 * @note  加入卡尔曼滤波 
 */
float RUD_Get_AngleErr(Rudder_Axis_Info_t *str)
{
  float res;
  res = RUD_PIDAngleTarget_Proc(str) - str->motor_data.PID_Angle;
//  res = KalmanFilter(&str->KF_Angle[RC_GYRO],res);//卡尔曼滤波
  return res;
}

/**
 * @brief 获取对应轴电机当前速度环的Err
 * @note  没加卡尔曼滤波，加了之后效果不好 
 */
float RUD_Get_SpeedErr(Rudder_Axis_Info_t *str)
{
  float res;
  res = str->motor_data.PID_Speed_target - str->motor_data.PID_Speed;
  return res;
}

/**
 * @brief 获取串级PID输出
 * @param 
 */
float RUD_GetOutput(Rudder_Axis_Info_t *str)
{
  PID_Info_t *PID = &(str->PID);
  Motor_Data_t *DATA = &(str->motor_data);
  float res;
  
  /*同步电机数据*/
  RUD_DataSync(str);
 
  /*获取角度环误差输入*/
  PID->Angle_Loop.Err = RUD_Get_AngleErr(str);
  
  /*速度环期望 = 角度环PID输出*/
  DATA->PID_Speed_target = PID_Algorithm(&PID->Angle_Loop);

  /*获取速度环误差输入*/  
  PID->Speed_Loop.Err = RUD_Get_SpeedErr(str);
  
  /*返回值 = 速度环PID输出*/
  res = PID_Algorithm(&PID->Speed_Loop);
  
  return res;
}

/**
 * @brief 舵轮PID数据全部清0函数
 * @param 
 */
void RUD_SetEmpty(PID_Info_t *str)
{
  memset(&str->Speed_Loop,0,sizeof(PID_Loop_t));
  memset(&str->Angle_Loop,0,sizeof(PID_Loop_t));
}
 
/**
 * @brief 卸力函数
 * @param 
 */
void RUD_Stop(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
	
	/* 速度环最终输出 */
  pid_out[RUD_LF] = 0;
  pid_out[RUD_RF] = 0;
  pid_out[RUD_LB] = 0;
  pid_out[RUD_RB] = 0;
  
	CAN2_Send(RUD_ESC, pid_out);	  
}

/**
 * @brief 舵轮电机最终输出
 * @param 
 */
void RUD_Output(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
	
	/* 速度环最终输出 */
  pid_out[RUD_LF] = (int16_t)RUD_GetOutput(&Rudder.Rudder_Axis[RUD_LF]);
  pid_out[RUD_RF] = (int16_t)RUD_GetOutput(&Rudder.Rudder_Axis[RUD_RF]);
  pid_out[RUD_LB] = (int16_t)RUD_GetOutput(&Rudder.Rudder_Axis[RUD_LB]);
  pid_out[RUD_RB] = (int16_t)RUD_GetOutput(&Rudder.Rudder_Axis[RUD_RB]);

  /* CAN发送 */
	CAN2_Send(RUD_ESC, pid_out);	
}


/**
 * @brief 初始化舵轮角度
 * @param 
 */
void RUD_FirstAngle(Rudder_Axis_Info_t *str)
{
  str->RUD_Move.Dir = str->MECH_Mid_Angle;
}



float SPIN_Speed(void)
{
  float res = 0;
  switch((int)DUM_Report_CHAS_MaxPower())
  {
    case 45:
        res = SPIN_SPEED_50W-20;
        break;
    case 55:
        res = SPIN_SPEED_50W+20;
        break;
    case 60:
        res = SPIN_SPEED_60W;
        break;
    case 70:
        res = SPIN_SPEED_60W;
        break;
    case 80:
        res = SPIN_SPEED_80W;
        break;
    case 100:
        res = SPIN_SPEED_100W;
        break;
    case 120:
        res = SPIN_SPEED_120W;
        break;
    default:
        res = SPIN_SPEED_50W;
        break;
  }
  
  if(Report_SuperCap_cmd())
  {
    res = SPIN_SPEED_SUPER;
  }
  
  return res;
}
float Move_SPIN_Speed(void)
{
  float res = 0;
  switch((int)DUM_Report_CHAS_MaxPower())
  {
    case 45:
        res = Move_SPIN_SPEED_50W-20;
        break;
    case 55:
        res = Move_SPIN_SPEED_50W+20;
        break;
    case 60:
        res = Move_SPIN_SPEED_60W;
        break;
    case 70:
        res = Move_SPIN_SPEED_60W;
        break;
    case 80:
        res = Move_SPIN_SPEED_80W;
        break;
    case 100:
        res = Move_SPIN_SPEED_100W;
        break;
    case 120:
        res = Move_SPIN_SPEED_120W;
        break;
    default:
        res = Move_SPIN_SPEED_50W;
        break;
  }
  
  if(Report_SuperCap_cmd())
  {
    res = Move_SPIN_SPEED_SUPER;
  }
  
  return res;
}

//舵轮小陀螺
void RUD_SPIN(void)
{
//  float Zangle = YAW_AngleErrReport();
  switch(Rudder.ctrl_mode)
  {
    case RC_CTRL_MODE:
        /*底盘运动控制解算*/
        RUD_Slow(&Rudder.Vector.X_speed , RC_CH3 , 1.5 , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , RC_CH2 , 1.5 , 660);
    
        if(!CHAS_SPIN_Close_Report()) {
          //陀螺控制不处于回正状态
          if(Rudder.Vector.X_speed ==0 && Rudder.Vector.Y_speed == 0)
            global_slow(&Rudder.Vector.Z_speed , SPIN_Speed() * Rudder.Vector.Z_LR , 0.9);
          else 
            global_slow(&Rudder.Vector.Z_speed , Move_SPIN_Speed() * Rudder.Vector.Z_LR , 1.4);//为了区分最后一次旋转舵角的旋转方向和旋转速度
        }
        else {
          //处于回正状态
          global_slow(&Rudder.Vector.Z_speed , 160 * Rudder.Vector.Z_LR , 1.8);//慢速回正
//          Rudder.Vector.Z_speed = Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*4;
        }
        break;
    
    case KEY_CTRL_MODE:
        /*底盘运动控制解算*/
        RUD_Slow(&Rudder.Vector.X_speed , KEY_FB_Ctrl()  , 1.15f , 660);
        RUD_Slow(&Rudder.Vector.Y_speed , KEY_LR_Ctrl()  , 1.15f , 660);
    
        if(!CHAS_SPIN_Close_Report()) {
          //陀螺控制不处于回正状态
          if(Rudder.Vector.X_speed ==0 && Rudder.Vector.Y_speed == 0)
            global_slow(&Rudder.Vector.Z_speed , SPIN_Speed() * Rudder.Vector.Z_LR , 1);
          else 
            global_slow(&Rudder.Vector.Z_speed , Move_SPIN_Speed() * Rudder.Vector.Z_LR , 1.5);//为了区分最后一次旋转舵角的旋转方向和旋转速度
        }
        else {
          //处于回正状态
          global_slow(&Rudder.Vector.Z_speed , 150 * Rudder.Vector.Z_LR , 1.8);//慢速回正
//          Rudder.Vector.Z_speed = Movedir_Z_speed_Proc(YAW_AngleErrReport()*660 /8192)*4;
        }
        break;
  } 
  /*输出处理*/
  
    /* 得到舵轮的真实前进角度 */
    RUD_Get_F_Angle_GYRO();
    
    /* 得到舵轮的真实旋转角度 */
    RUD_Get_Z_Angle_GYRO();
    
    /* 得到舵轮XYZ平面的矢量角 */
    RUD_Angle_Fusion_XYZ();
}

