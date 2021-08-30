/***      《 常规模块控制代码 》

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
 
#include "module.h"
#include "Device.h"
 
Module_Info_t Module = {
  .ctrl_mode = RC_CTRL_MODE,
  .pid_mode = GYRO,
  
  .REMOTE_or_REFER_NORMAL = false,
    
  .Friction.IF_WORK = false,  
  .Friction.IF_INIT_OK = false,  
  .Friction.friction_num = FRICTION_INIT_SPEED,
  .Friction.friciont_num_target = FRICTION_INIT_SPEED,
  .Clip.Clip_State = CLIP_CLOSE_NUM,
}; 
 


/* ——————————————————————————————————————————————控制层—————————————————————————————————————————————— */

/**
 * @brief 常规模块主控
 * @param 
 */
void MODULE_Ctrl(void)
{
//  MODULE_GetInfo();
  switch(Module.ctrl_mode)
  {
    case  RC_CTRL_MODE:
      First_GetInto_Mode();//第一次进入模式的处理
      
      RC_Friction_Ctrl();/*摩擦轮*/
      
      RC_AimLED_Ctrl();/* LED */
      
      RC_Clip_Ctrl();/* 弹舱舵机 */
      break;
    
    case  KEY_CTRL_MODE:
      First_GetInto_Mode();//第一次进入模式的处理
      
      KEY_Friction_Ctrl();/*摩擦轮*/
      
      KEY_AimLED_Ctrl();/* LED */
      
      KEY_Clip_Ctrl();/* 弹舱舵机 */
      break;
  }
}
 
/**
 * @brief 常规模块获取系统控制信息
 * @param 
 */
void MODULE_GetInfo(void)
{
  Module.ctrl_mode = System.ctrl_mode;
  Module.pid_mode = System.pid_mode;
  Friction_SetInitNum();
  Judge_Clip_state();
  if(IF_DUM_NORMAL)
  {
    Check_IF_ArmorBooster_Deinit();//判断armorbooster是否断电，断电的话需要重新初始化
//    Friction_SpeedFIX();//射速自适应
  }
}

/**
 * @brief 摩擦轮复位
 * @param 
 */
void Friction_Reset(void)
{
//  Fric_3508_stop();
//  Fric_3508_Reset();
  Fric_Speed_Slope(&Module.Friction.friction_num ,FRICTION_STOP_SPEED);//FRICTION_INIT_SPEED);
  Fric_3508_Handler();
  Module.Friction.IF_WORK = false;
}

/**
 * @brief 激光复位
 * @param 
 */
void AimLED_Reset(void)
{
  AimLED_Ctrl = 0;
  Module.AimLED.IF_WORK = false;
}

/**
 * @brief 舵机复位
 * @param 
 */
void Clip_Reset(void)
{
  Clip_turn(DISABLE);
}

/**
 * @brief 判断是否第一次进入 遥控器/键盘 模式
 * @param 
 */
static System_Ctrl_Mode_t M_prev_ctrl_mode = RC_CTRL_MODE;
void First_GetInto_Mode(void)
{
  if(M_prev_ctrl_mode != Module.ctrl_mode)
  {
    Clip_turn(ENABLE);
    M_prev_ctrl_mode = Module.ctrl_mode;
    Module.Friction.IF_WORK = false;
    Module.AimLED.IF_WORK = false;
    Module.Clip.Clip_State = CLIP_CLOSE_NUM;
  }
}

/**
 * @brief 常规任务复位
 * @param 
 */
void MODULE_Reset(void)
{
  if(M_prev_ctrl_mode != Module.ctrl_mode)
  {
    M_prev_ctrl_mode = Module.ctrl_mode;
    Module.REMOTE_or_REFER_NORMAL = false;
  }
  Friction_Reset();
  AimLED_Reset();
  Clip_Reset();
}
/**
 * @brief 摩擦轮初始化、电调校准
 * @note  小蜜蜂电机 
 */
void Friction_Init(void)
{
  PWM_Friction_Init(2500-1,84-1);//2500-1,84-1
  
  TIM_SetCompare1(TIM4,FRICTION_INIT_SPEED);//左
  TIM_SetCompare2(TIM4,FRICTION_INIT_SPEED);//右
  TIM_SetCompare1(TIM3,FRICTION_INIT_SPEED);//左
  TIM_SetCompare2(TIM3,FRICTION_INIT_SPEED);//右 
}

void Friction_Init_Reset(void)//重启
{
  Module.Friction.IF_INIT_OK = NO;
}

void Friction_SetInitNum(void)
{
  if(Module.Friction.IF_INIT_OK != true)
  {
    if(IF_DUM_NORMAL)//下主控正常
    if(DUM_Report_IF_ArmorBooster_work())
    {
      #if (Fric_motor == Fric_Bee)
        Module.Friction.IF_INIT_OK = YES;
      #elif (Fric_motor == Fric_3508)
        Module.Friction.IF_INIT_OK = YES;
      #endif
    }
  }
}
/*反馈摩擦轮是否启动*/
bool Report_IF_Fric_Init_Over(void)
{
  return Module.Friction.IF_INIT_OK;
}
/**
 * @brief 弹舱舵机初始化
 * @param 
 */
void Clip_Init(void)
{
  PWM_Clip_Init(999,(84*20)-1);
}
/**
 * @brief 弹舱舵机 使能/失能开关
 * @param 
 */
void Clip_turn(FunctionalState state)
{
   TIM_Cmd(TIM1, state);  //使能TIM1
}

/**
 * @brief 常规任务设备层总初始化
 * @note  由于瞄准灯的io初始化属于驱动层，此处不加
 * @param 
 */
void MODULE_Init(void)
{
  Friction_Init();
  Clip_Init();
}

/**    《遥控器步兵控制》
 * 
 *           _____                    _____                    _____                    _____          
 *          /\    \                  /\    \                  /\    \                  /\    \         
 *         /::\____\                /::\    \                /::\    \                /::\    \        
 *        /:::/    /                \:::\    \              /::::\    \              /::::\    \       
 *       /:::/    /                  \:::\    \            /::::::\    \            /::::::\    \      
 *      /:::/    /                    \:::\    \          /:::/\:::\    \          /:::/\:::\    \     
 *     /:::/____/                      \:::\    \        /:::/__\:::\    \        /:::/__\:::\    \    
 *    /::::\    \                      /::::\    \      /::::\   \:::\    \      /::::\   \:::\    \   
 *   /::::::\    \   _____    ____    /::::::\    \    /::::::\   \:::\    \    /::::::\   \:::\    \  
 *  /:::/\:::\    \ /\    \  /\   \  /:::/\:::\    \  /:::/\:::\   \:::\____\  /:::/\:::\   \:::\    \ 
 * /:::/  \:::\    /::\____\/::\   \/:::/  \:::\____\/:::/  \:::\   \:::|    |/:::/__\:::\   \:::\____\
 * \::/    \:::\  /:::/    /\:::\  /:::/    \::/    /\::/   |::::\  /:::|____|\:::\   \:::\   \::/    /
 *  \/____/ \:::\/:::/    /  \:::\/:::/    / \/____/  \/____|:::::\/:::/    /  \:::\   \:::\   \/____/ 
 *           \::::::/    /    \::::::/    /                 |:::::::::/    /    \:::\   \:::\    \     
 *            \::::/    /      \::::/____/                  |::|\::::/    /      \:::\   \:::\____\    
 *            /:::/    /        \:::\    \                  |::| \::/____/        \:::\   \::/    /    
 *           /:::/    /          \:::\    \                 |::|  ~|               \:::\   \/____/     
 *          /:::/    /            \:::\    \                |::|   |                \:::\    \         
 *         /:::/    /              \:::\____\               \::|   |                 \:::\____\        
 *         \::/    /                \::/    /                \:|   |                  \::/    /        
 *          \/____/                  \/____/                  \|___|                   \/____/         
 */ 



/**
 * @brief 遥控器 摩擦轮开关控制
 * @note  由于是中到上跳变时开关，所以用一个总控控制开关
 */
void RC_Friction_Ctrl(void)
{
  Fric_3508_SetPID();
  switch(Module.Friction.IF_WORK)
  {
    case (false): 
      Friction_Off();
      break;
    case (true):
      Friction_On();
      break;
  }
  #if (Fric_motor == Fric_3508)
  Fric_3508_Handler();
  #endif
}


/**
 * @brief 瞄准灯 遥控器总控
 * @note  由于是中到上跳变时开关，所以用一个总控控制开关 
 */
void RC_AimLED_Ctrl(void)
{
  switch(Module.AimLED.IF_WORK)
  {
    case (true):
      AimLED_On();
      break;
    case (false): 
      AimLED_Off();
      break;
  }
}

/**
 * @brief 常规任务状态更新
 * @note  因为在遥控器s2控制中，是通过中间->上面的跳变沿太控制常规模块一开一关的
          所以用一个类似触发器的状态控制函数供s2控制函数调用，RC与KEY操作不同
 * @param 
 */
void RC_MODULE_State_Update(void)
{
  switch(IF_MODULE_WORK)
  {/* 常规任务开启关闭 */
    case  YES:
      Module.Friction.IF_WORK = false;
      Module.AimLED.IF_WORK = false;
      break;
    
    case  NO:
      if(Report_IF_Fric_Init_Over())
      {//只有初始化成功了，才能开启摩擦轮
        Module.Friction.IF_WORK = true;
        Module.AimLED.IF_WORK = true;
      }
      break;
  }
}

/**
 * @brief 遥控器弹舱开关
 * @param 
 */
void RC_Clip_Ctrl(void)
{
  switch(CLIP_STATE)
  {
    case CLIP_OPEN_NUM:
      Clip_Open();
      break;
    case CLIP_CLOSE_NUM:
      Clip_Close();
      break;
  }
}

/**
 * @brief 同常规模块状态切换，由于是遥控器跳变沿触发开关切换，需要一个类似触发器的跳变沿状态切换函数
 * @param 
 */
void RC_Clip_State_Updata(void)
{
  switch(CLIP_STATE)
  {
    case CLIP_OPEN_NUM:
      CLIP_STATE = CLIP_CLOSE_NUM;
      break;
    case CLIP_CLOSE_NUM:
      CLIP_STATE = CLIP_OPEN_NUM;
      break;
  }
}
/**
 * @brief 判断弹舱状态
 * @param 
 */
void Judge_Clip_state(void)
{
  if(CLIP_STATE == CLIP_OPEN_NUM)
    Module.Clip.IF_CLIP_OPEN = YES;
  else Module.Clip.IF_CLIP_OPEN = NO;
}
/**
 * @brief 反馈弹舱状态
 * @param 
 */
bool Report_IF_CLIP_OPEN(void)
{
  return Module.Clip.IF_CLIP_OPEN;
}


/**          《键盘步兵控制》
 * ┌───┐   ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┐
 * │Esc│   │ F1│ F2│ F3│ F4│ │ F5│ F6│ F7│ F8│ │ F9│F10│F11│F12│ │P/S│S L│P/B│  ┌┐    ┌┐    ┌┐
 * └───┘   └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┘  └┘    └┘    └┘
 * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐ ┌───┬───┬───┐ ┌───┬───┬───┬───┐
 * │~ `│! 1│@ 2│# 3│$ 4│% 5│^ 6│& 7│* 8│( 9│) 0│_ -│+ =│ BacSp │ │Ins│Hom│PUp│ │N L│ / │ * │ - │
 * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┤ ├───┼───┼───┼───┤
 * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │{ [│} ]│ | \ │ │Del│End│PDn│ │ 7 │ 8 │ 9 │   │
 * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ └───┴───┴───┘ ├───┼───┼───┤ + │
 * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │: ;│" '│ Enter  │               │ 4 │ 5 │ 6 │   │
 * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤     ┌───┐     ├───┼───┼───┼───┤
 * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │< ,│> .│? /│  Shift   │     │ ↑ │     │ 1 │ 2 │ 3 │   │
 * ├─────┬──┴─┬─┴──┬┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ┌───┼───┼───┐ ├───┴───┼───┤ E││
 * │ Ctrl│    │Alt │         Space         │ Alt│ FN │    │Ctrl│ │ ← │ ↓ │ → │ │   0   │ . │←─┘│
 * └─────┴────┴────┴───────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘ └───────┴───┴───┘
 */

/**
 * @brief 键盘 摩擦轮开关控制
 * @note  由于是中到上跳变时开关，所以用一个总控控制开关
 */
void KEY_Clip_Ctrl(void)
{
  switch(CLIP_STATE)
  {
    case CLIP_OPEN_NUM:
      Clip_Open();
      break;
    case CLIP_CLOSE_NUM:
      Clip_Close();
      break;
  }
}

/**
 * @brief 瞄准灯 键盘总控
 * @note  由于是中到上跳变时开关，所以用一个总控控制开关 
 */
void KEY_AimLED_Ctrl(void)
{
  switch(Module.AimLED.IF_WORK)
  {
    case (true):
      AimLED_On();
      break;
    case (false): 
      AimLED_Off();
      break;
  }  
}

/**
 * @brief 键盘开关弹舱
 * @param 
 */
void KEY_Friction_Ctrl(void)
{
  Fric_3508_SetPID();
  switch(Module.Friction.IF_WORK)
  {
    case (false): 
      Friction_Off();
      break;
    case (true):
      Friction_On();
      break;
  }
  #if (Fric_motor == Fric_3508)
  Fric_3508_Handler();
  #endif
}
/**
 * @brief 同常规模块状态切换，由于是遥控器跳变沿触发开关切换，需要一个类似触发器的跳变沿状态切换函数
 * @param 
 */
void CLIP_Set_CloseNum(void)
{
  CLIP_STATE = CLIP_CLOSE_NUM;
}
void CLIP_Set_OpenNum(void)
{
  CLIP_STATE = CLIP_OPEN_NUM;
}
void KEY_Clip_State_Updata(void)
{
  switch(CLIP_STATE)
  {
    case CLIP_OPEN_NUM:
      ShootNum_Reset();
      CLIP_Set_CloseNum();
      break;
    case CLIP_CLOSE_NUM:
      CLIP_Set_OpenNum();
      break;
  }
}

/* ——————————————————————————————————————————————功能层—————————————————————————————————————————————— */
/**
 * @brief 读取裁判系统的信息
 * @param 
 */
void Module_GetReferInfo(void)
{
  Module.Friction.speed_limit_level = DUM_Report_Shoot_SpeedLimit();
  if(Module.Friction.now_speed_level != Module.Friction.speed_limit_level)
    Module.Friction.now_speed_level = Module.Friction.speed_limit_level;
}
/**
 * @brief 检查射速
 * @note  摩擦轮变速，动态调节转速
 */
uint8_t SpeedErr_cnt=0;
void SpeedAdapt(float real_S , float min_S, float max_S,float *fix)
{
  if(real_S < min_S && real_S > 10)
    SpeedErr_cnt++;
  else if(real_S >= min_S && real_S <= max_S )SpeedErr_cnt = 0;
  if(SpeedErr_cnt == 4)//射速偏低
  {
    SpeedErr_cnt = 0;
    *fix += 1;
  }
  if(real_S > max_S)//射速偏高
    *fix -= 2;
  #if (Fric_motor == Fric_Bee)
  if(real_S < 10)//射速小于10判断为尿弹
    Module_KEY_OFF();
  #endif
}
uint8_t prev_Shoot_num = 0;
bool IF_SpeedUpdate = NO;
void Friction_SpeedFIX(void)
{
  float realspeed = DUM_Report_RealShootSpeed();
  if(prev_Shoot_num != DUM_Report_Shoot_NUM())
  {
    //用打弹数量的更新来判断射速速度是否更新
    prev_Shoot_num = DUM_Report_Shoot_NUM();
    IF_SpeedUpdate = YES;
  }else IF_SpeedUpdate = NO;
  
  if(IF_SpeedUpdate)
  switch(Module.Friction.now_speed_level)
  {
    case SHOOT_L1_SPEED:
      SpeedAdapt(realspeed , 14 , 14.8 , &Module.Friction.Level_1_fix_num);
      break;
    
    case SHOOT_L2_SPEED:
      SpeedAdapt(realspeed , 17 , 17.8 , &Module.Friction.Level_2_fix_num);
      break;
  
    case SHOOT_L3_SPEED:
      SpeedAdapt(realspeed , 28.2 , 29.7 , &Module.Friction.Level_3_fix_num);
      break;
  }
}


/**
 * @brief 摩擦轮斜坡加速 
 * @param 
 */
void Fric_Speed_Slope(float * now_speed,float change_target_speed) 
{
  #if (Fric_motor == Fric_Bee)
  if(*now_speed!=change_target_speed)
  {
    if((*now_speed) > change_target_speed) (*now_speed)-=10;
    if((*now_speed) < change_target_speed) (*now_speed)+=10;
    if(abs((*now_speed) - change_target_speed  ) < 15) 
    {
      (*now_speed) = change_target_speed;
    }
    if(*now_speed != 0)
    {
      TIM_SetCompare1(TIM3,(int16_t)(*now_speed+1000 ));
      TIM_SetCompare2(TIM3,(int16_t)(*now_speed+1000 ));//右
    }
    else 
    {
      TIM_SetCompare1(TIM3,(int16_t)(*now_speed+1000));
      TIM_SetCompare2(TIM3,(int16_t)(*now_speed+1000));
    }
    if(*now_speed != 0)
    {
      TIM_SetCompare1(TIM4,(int16_t)(*now_speed+1000));
      TIM_SetCompare2(TIM4,(int16_t)(*now_speed+1000));//左
    }//可能两个枪管摩擦轮射速不同
    else 
    {
      TIM_SetCompare1(TIM4,(int16_t)(*now_speed+1000));
      TIM_SetCompare2(TIM4,(int16_t)(*now_speed+1000));
    }
  }
  #elif (Fric_motor == Fric_3508)
//  if(*now_speed!=change_target_speed)
//  {
    if((*now_speed) > change_target_speed) (*now_speed)-=80;
    if((*now_speed) < change_target_speed) (*now_speed)+=80;
    if(abs((*now_speed) - change_target_speed  ) < 100) 
    {
      (*now_speed) = change_target_speed;
    }
//  }
    Fric_L_Set_SpeedTarget(-(*now_speed));
    Fric_R_Set_SpeedTarget((*now_speed));
//    Fric_3508_Handler();
  #endif
}

/**
 * @brief 根据裁判系统的信息切换摩擦轮速度
 * @param 
 */
float test_FricTarget = 0;//4530;
void Fric_Speed_Change(void)
{
#if (IF_REFER_CONNECT == 1)
  if(IF_DUM_NORMAL && IF_RC_NORMAL)
  {
    Module.REMOTE_or_REFER_NORMAL = true;
    Module_GetReferInfo();
    switch(Module.Friction.now_speed_level)
    {
      case  SHOOT_L1_SPEED:
          Module.Friction.friciont_num_target = FRICTION_L1_SPEED + Module.Friction.Level_1_fix_num;
          break;
      case  SHOOT_L2_SPEED:
          Module.Friction.friciont_num_target = FRICTION_L2_SPEED + Module.Friction.Level_2_fix_num;
          break;        
      case  SHOOT_L3_SPEED:
          Module.Friction.friciont_num_target = FRICTION_L3_SPEED + Module.Friction.Level_3_fix_num;
          break;  
      
      default:
          Module.Friction.friciont_num_target = FRICTION_L1_SPEED;//FRICTION_INIT_SPEED;
          break;          
    }
  }
  else if(!IF_RC_NORMAL)//遥控器关闭
  {
    Module.Friction.friciont_num_target = FRICTION_STOP_SPEED;//FRICTION_INIT_SPEED;
    Module.REMOTE_or_REFER_NORMAL = false;
  }
  else if(!IF_DUM_NORMAL && IF_RC_NORMAL)
  {
  //考虑加上手动调速
    Module.Friction.friciont_num_target = FRICTION_L1_SPEED ;
  }
      
   Fric_Speed_Slope(&Module.Friction.friction_num , Module.Friction.friciont_num_target);
  
#elif (IF_REFER_CONNECT == 0)
  //用于调试
   Fric_Speed_Slope(&Module.Friction.friction_num , test_FricTarget);//FRICTION_L1_SPEED);
#endif
}

uint8_t Report_Shoot_Speed(void)
{
  return Module.Friction.now_speed_level;
}

/**
 * @brief 摩擦轮 遥控开启
 * @param 
 */
void Friction_On(void)
{
  Fric_Speed_Change();
}

/**
 * @brief 摩擦轮 遥控关闭
 * @param 
 */
void Friction_Off(void)
{
  Fric_Speed_Slope(&Module.Friction.friction_num ,FRICTION_STOP_SPEED);//INITxxxx
}

/**
 * @brief 瞄准灯 开启
 * @param 
 */
void AimLED_On(void)
{
  AimLED_Ctrl = 1;
}

/**
 * @brief 瞄准灯 关闭
 * @param 
 */
void AimLED_Off(void)
{
  AimLED_Ctrl = 0;
}

/**
 * @brief 弹舱开
 * @param 
 */
void Clip_Open(void)
{
  TIM_SetCompare2(TIM1,(uint32_t)CLIP_OPEN_NUM);//打开
}

/**
 * @brief 弹舱关
 * @param 
 */
uint32_t test_Clip_num = 150;
void Clip_Close(void)
{
  TIM_SetCompare2(TIM1,(uint32_t)CLIP_CLOSE_NUM);//关闭
}


/**
 * @brief 键盘打开
 * @param 
 */
void Module_KEY_ON(void)
{
//  if(!IF_MODULE_WORK)Single_Reverse_HALF();//拨盘反转一格
  if(Module.Friction.IF_INIT_OK)
  {//只有初始化成功了，才能开启摩擦轮
    Module.Friction.IF_WORK = true;
    Module.AimLED.IF_WORK = true;
  }
//  CLIP_Set_CloseNum();
}

/**
 * @brief 用于当摩擦轮不转时初始化，使之重转
 * @param 
 */
void Module_KEY_OFF(void)
{
  Module.Friction.IF_WORK = false;
  Module.AimLED.IF_WORK = false;
//  CLIP_Set_CloseNum();
}

/**
 * @brief 当armorbooster断电的时候复位摩擦轮
 * @param 
 */
void Check_IF_ArmorBooster_Deinit(void)
{
  if(!DUM_Report_IF_ArmorBooster_work())
  { 
    Module.Friction.IF_WORK = false;
    Module.AimLED.IF_WORK = false;
    Module.Friction.IF_INIT_OK = NO;
  }
}

//--------------------------------------------摩擦轮3508------------------------------------------//

/**
 * @brief 左右摩擦轮设置速度
 * @param 
 */
void Fric_L_Set_SpeedTarget(float TargetSpeed)
{
  Motor_Data_t *str = &(Module.Friction.Fric[left].motor_Data);
  str->PID_Speed_target = TargetSpeed;
}
void Fric_R_Set_SpeedTarget(float TargetSpeed)
{
  Motor_Data_t *str = &(Module.Friction.Fric[righ].motor_Data);
  str->PID_Speed_target = TargetSpeed;
}
/**
 * @brief 摩擦轮3508电机数据同步
 * @param 
 */
void Fric_3508_DataSync(Motor_Data_t *str)
{
  str->PID_Speed = str->CAN_GetData.Motor_Speed;
  str->PID_Angle = str->CAN_GetData.Motor_Angle;
}
/**
 * @brief 得到速度误差
 * @param 
 */
float Fric_3508_Get_SpeedErr(Motor_Data_t *str)
{
  float res;
  res = str->PID_Speed_target - str->PID_Speed;
  return res;
}
/**
 * @brief PID算法获取输出
 * @note  速度环
 */
float Fric_3508_GetOutput(Fric_3508_Info_t *str)
{
  PID_Info_t *PID = &str->PID;
  Motor_Data_t *DATA = &str->motor_Data;
  float res;
  
  Fric_3508_DataSync(DATA);//数据同步

//  PID->Angle_Loop.Err = CHAS_Get_AngleErr(DATA);
//  
//  DATA->PID_Speed_target = PID_Algorithm(&PID->Angle_Loop);//角度环
  
  PID->Speed_Loop.Err = Fric_3508_Get_SpeedErr(DATA);
  
  res = PID_Algorithm(&PID->Speed_Loop);//速度环
  
  return res;
}
/**
 * @brief 摩擦轮3508输出调用
 * @param 
 */
float js_fric_1_speed = 0 , js_fric_2_speed = 0;//js看电机速度
void Fric_3508_Handler(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
  
  js_fric_1_speed = Module.Friction.Fric[righ].motor_Data.PID_Speed;
  js_fric_2_speed = Module.Friction.Fric[left].motor_Data.PID_Speed;
  
  //PID算法输出
  pid_out[left] = (int16_t)(Fric_3508_GetOutput(&Module.Friction.Fric[left]));
  pid_out[righ] = (int16_t)(Fric_3508_GetOutput(&Module.Friction.Fric[righ]));
  
  CAN2_Send(Fric_3508_ESC, pid_out);
}

void Fric_3508_stop(void)
{
	static int16_t pid_out[4] = {0, 0, 0, 0};
  
	CAN2_Send(Fric_3508_ESC, pid_out);	
}

void Fric_3508_Reset(void)
{
  memset(&Module.Friction.Fric[left],0,sizeof(Fric_3508_Info_t));
  memset(&Module.Friction.Fric[righ],0,sizeof(Fric_3508_Info_t));
}
float test_spl = 13 , test_spr = 13, test_sil = 0.28  , test_sir = 0.28;
void Fric_3508_SetPID(void)
{
  Module.Friction.Fric[left].PID.Speed_Loop.PID_Param.P = test_spl;//20;
  Module.Friction.Fric[left].PID.Speed_Loop.PID_Param.I = test_sil;//2;
  Module.Friction.Fric[left].PID.Speed_Loop.PID_Param.D = 0;
  Module.Friction.Fric[righ].PID.Speed_Loop.PID_Param.P = test_spr;//20;
  Module.Friction.Fric[righ].PID.Speed_Loop.PID_Param.I = test_sir;//2;
  Module.Friction.Fric[righ].PID.Speed_Loop.PID_Param.D = 0;
  Fric_3508_Init();
}

//反馈3508电机是否达到目标转速
bool Report_IF_Fric3508_SetSpeed(void)
{
  bool res = YES;
  #if (Fric_motor == Fric_3508)
  if(abs(Module.Friction.Fric[left].motor_Data.PID_Speed - Module.Friction.Fric[left].motor_Data.PID_Speed_target) < 2000 && 
     abs(Module.Friction.Fric[righ].motor_Data.PID_Speed - Module.Friction.Fric[righ].motor_Data.PID_Speed_target) < 2000)
    res = YES;
  else 
    res = NO;
  #endif
  return res;
}
