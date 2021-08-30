/***      《 PID算法 》

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

#include "PID.h"

/* Init start */
PID_Core_Ctrl_t PID_Core = {
  .ctrl_mode = RC_CTRL_MODE,
  .pid_mode = MECH,
  .action_mode = SYS_ACT_NORMOL,  
  .PID_type = Turn_off,
};

/* Init end   */

/* 用于调试PID参数的函数 */
PID_Parameter_t PID_SpeedDebug = {
  .P = 0,
  .I = 0,
  .D = 0,
};
PID_Parameter_t PID_AngleDebug = {
  .P = 0,
  .I = 0,
  .D = 0,
};
void PID_Debug(PID_Info_t *str)
{
  str->Speed_Loop.PID_Param.P = PID_SpeedDebug.P;
  str->Speed_Loop.PID_Param.I = PID_SpeedDebug.I;
  str->Speed_Loop.PID_Param.D = PID_SpeedDebug.D;
  
  str->Angle_Loop.PID_Param.P = PID_AngleDebug.P;
  str->Angle_Loop.PID_Param.I = PID_AngleDebug.I;
  str->Angle_Loop.PID_Param.D = PID_AngleDebug.D;
}
/*-------------------------*/



/**
 * @brief PID(速度/角度)环控制器
 * @param 
 */
float PID_Algorithm(PID_Loop_t *str)
{ 
  /*误差限幅*/
  str->Err = constrain( str->Err,
                        str->PID_Err_Max , 
                       -str->PID_Err_Max) ; 
  /*误差死区*/
  str->Err = anti_constrain( str->Err,
                             str->PID_Err_Dead , 
                            -str->PID_Err_Dead) ;
  
  /* P *//* P积分限幅 */ 
  str->PID_P_Out =  constrain( str->PID_Param.P * str->Err ,
                               str->PID_P_Out_Max ,
                              -str->PID_P_Out_Max ); 
  
  /* I *//* I积分限幅 */  
  str->I_Accu_Err = constrain( str->I_Accu_Err + str->Err,
                               str->I_Accu_Err_Max , 
                              -str->I_Accu_Err_Max );
         /* I输出限幅 */  
  str->PID_I_Out  = constrain( str->PID_Param.I * str->I_Accu_Err,
                               str->PID_I_Out_Max ,
                              -str->PID_I_Out_Max );
  /* D */  
  str->PID_D_Out = str->PID_Param.D *(str->Err - str->Last_Err);

  str->Last_Err = str->Err;

  /* 总 *//* 总输出限幅 */  
  str->PID_Output = constrain( str->PID_P_Out + str->PID_I_Out + str->PID_D_Out,
                               str->PID_Output_Max ,
                              -str->PID_Output_Max);
  return str->PID_Output;
}



/**
 * @brief PID主控函数
 * @param 
 */
void PID_Ctrl(void)
{
  Judge_PID_Type();
}

/**
 * @brief PID获取系统信息函数 
 * @param 
 */
void PID_GET_Info(void)
{
  PID_Core.ctrl_mode = System.ctrl_mode;
  PID_Core.pid_mode = System.pid_mode;
  PID_Core.action_mode = System.action_mode; 
}

/**
 * @brief PID判断控制模式类型函数
 * @param 
 */
void Judge_PID_Type(void)
{
  switch(PID_Core.action_mode)
  {
    case SYS_ACT_NORMOL:
      switch(PID_Core.ctrl_mode)
      {
        case RC_CTRL_MODE:
          switch(PID_Core.pid_mode)
          {
            case MECH:
              PID_Core.PID_type = RC_MECH;
              break;
            
            case GYRO:
              PID_Core.PID_type = RC_GYRO;
              break;
          }break;
          
        case KEY_CTRL_MODE:
          switch(PID_Core.pid_mode)
          {
            case MECH:
              PID_Core.PID_type = KEY_MECH;
              break;
            
            case GYRO:
              PID_Core.PID_type = KEY_GYRO;
              break;
          }break;
      }break;
    
    case SYS_ACT_AUTO_AIM:
        PID_Core.PID_type = ACT_AUTO_AIM;
      break;
      
    case SYS_ACT_SMALL_BUFF:
        PID_Core.PID_type = ACT_SMALL_BUFF;
      break;
      
    case SYS_ACT_BIG_BUFF:
        PID_Core.PID_type = ACT_BIG_BUFF;
      break;
    
    case SYS_ACT_SPEED:
        PID_Core.PID_type = ACT_SPEED;
      break;

    case SYS_ACT_PARK:
        PID_Core.PID_type = ACT_PARK;
      break;    
  }
}

/**
 * @brief PID复位函数
 * @param 
 */
void PID_Reset(void)
{
  PID_Core.PID_type = Turn_off;
}

/**
 * @brief PID状态引出接口
 * @param 
 */
PID_type_t GET_PID_Type(void)
{
  return PID_Core.PID_type;
}
