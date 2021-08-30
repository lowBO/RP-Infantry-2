/***      《 视觉通信代码 》

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
#include "vision.h"
#include "Device.h"
#include "Function.h"

uint8_t Vision_Tx_Buffer[VISION_TX_BUFFER_LEN];
/* Init start */
Vision_Info_t Vision = {
  .action_mode = ACT_Err,
  .System_Cmd_Id = CMD_AIM_AUTO,
  .Trigger_Signal = NO,
};
/* Init end   */

System_Action_t prev_action = ACT_Err;

/* --------------------------任务层------------------------- */
/**
 * @brief 视觉总控
 * @param 
 */
void VISION_Ctrl(void)
{
  switch(Vision.action_mode)
  {
    case SYS_ACT_AUTO_AIM:
        AUTO_AIM_Ctrl();
        break;
    case SYS_ACT_BIG_BUFF:
        BIG_BUFF_Ctrl();
        break;
    case SYS_ACT_SMALL_BUFF:
        SMALL_BUFF_Ctrl();
        break;
    default :
        Vision_Reset();
        break;
  }
}

/**
 * @brief 读取系统信息
 * @param 
 */
void VISION_GET_Info(void)
{
  Vision.action_mode = System.action_mode;
}

/**
 * @brief 自瞄控制
 * @param 
 */
void Set_IF_ShootSentry(void)
{
  Vision.System_Cmd_Id = CMD_AIM_SENTRY;
}
void Reset_IF_ShootSentry(void)
{
  Vision.System_Cmd_Id = CMD_AIM_AUTO;
}
bool Report_IF_ShoorSentry(void)
{
  bool res = NO;
  if(Vision.System_Cmd_Id == CMD_AIM_SENTRY)
    res = YES;
  else 
    res = NO;
  return res;
}

void AUTO_AIM_Ctrl(void)
{
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.my_color = \
                        Judge_MyColor(DUM_Report_Robot_ID());
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.fric_speed = \
                        Report_Shoot_Speed();
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IMU_YAW = \
                        Report_prev_imu_yaw_angle(19);//Report_YawAngle();
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IMU_PIT = \
                        Report_prev_imu_pit_angle(19);//Report_PitAngle();
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IF_anti_top = \
                        Report_IF_anti_GYRO();
  switch(Vision.System_Cmd_Id)
  {
    case CMD_AIM_SENTRY://打哨兵
        VISION_SendData(CMD_AIM_SENTRY);
        break;
    case CMD_AIM_AUTO://普通自瞄
        VISION_SendData(CMD_AIM_AUTO);
        break;
    case CMD_AIM_BASE://吊射基地
        break;
  }
}
/**
 * @brief 切换打符模式
 * @param 
 */
void Set_IF_BigBuff(void)
{
  Vision.System_Cmd_Id = CMD_AIM_BIG_BUFF;
}
void Set_IF_SmallBuff(void)
{
  Vision.System_Cmd_Id = CMD_AIM_SMALL_BUFF;
}
void Reset_IF_BuffMode(void)
{
  //只有进入buff之后才会有退出buff
  if(Vision.System_Cmd_Id == CMD_AIM_BIG_BUFF ||
     Vision.System_Cmd_Id == CMD_AIM_SMALL_BUFF)
  Vision.System_Cmd_Id = CMD_AIM_AUTO;
}

/**
 * @brief 大符控制
 * @param 
 */
void BIG_BUFF_Ctrl(void)
{
  Vision.VisionRTx.BuffAim_Tx.Packet.TxData.my_color = \
                        Judge_MyColor(DUM_Report_Robot_ID());
  Vision.VisionRTx.BuffAim_Tx.Packet.TxData.fric_speed = \
                        Report_Shoot_Speed();  
  VISION_SendData(CMD_AIM_BIG_BUFF);
}
/**
 * @brief 小符控制
 * @param 
 */
void SMALL_BUFF_Ctrl(void)
{
  Vision.VisionRTx.BuffAim_Tx.Packet.TxData.my_color = \
                        Judge_MyColor(DUM_Report_Robot_ID());
  Vision.VisionRTx.BuffAim_Tx.Packet.TxData.fric_speed = \
                        Report_Shoot_Speed();
  VISION_SendData(CMD_AIM_SMALL_BUFF);
}
/**
 * @brief 复位
 * @param 
 */
void Set_IF_AutoAim(void)
{
  Vision.System_Cmd_Id = CMD_AIM_AUTO;
}
float test_sent_num = 0;
void Vision_Reset(void)
{
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.my_color = \
                        Judge_MyColor(DUM_Report_Robot_ID());
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.fric_speed = \
                        Report_Shoot_Speed();
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IMU_YAW = Report_YawAngle();
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IMU_PIT = Report_PitAngle();
  
  Vision.VisionRTx.AutoAim_Tx.Packet.TxData.IF_anti_top = \
                        Report_IF_anti_GYRO();

  VISION_SendData(NO_VISION);
}



//------------------------功能层------------------------
/**
 *	@brief	读取视觉标志位
 *  @param  Vision_Flag flag 可读取项目的枚举
 *  @note   
 */
bool VISION_GetFlagStatus(Vision_Flag flag)
{
  switch(flag)
  {
		case IF_LOCK_ARMOR:
			return Vision.VisionRTx.AutoAim_Rx.Packet.RxData.identify_target;

    case IF_FRAMEDROP:
      return Vision.VisionRTx.AutoAim_Rx.Packet.RxData.identify_framedrop;

    case IF_CHANGE_ARMOR :
      return Vision.VisionRTx.AutoAim_Rx.Packet.RxData.change_armor;
    
    case IF_CHANGE_ARMOR_BUFF:
      return Vision.VisionRTx.BuffAim_RX.Packet.RxData.change_armor;
      
    case IF_IDENTIFY_SPIN :
      return Vision.VisionRTx.AutoAim_Rx.Packet.RxData.anti_gyro;
    
    case IF_LOCK_BUFF:
      return Vision.VisionRTx.BuffAim_RX.Packet.RxData.identify_buff;
      
		default:
			return false;
	}
}

float Get_Aim_distance(void)
{
  return Vision.VisionRTx.AutoAim_Rx.Packet.RxData.distance;
}

bool IF_anti_GYRO = YES;
void Set_anti_GYRO(void)
{
  IF_anti_GYRO = YES;
}
void Reset_anti_GYRO(void)
{
  IF_anti_GYRO = NO;
}
bool Report_IF_anti_GYRO(void)
{
  return IF_anti_GYRO;
}

/**
 *	@brief 判断自身颜色
 *	@note
 */
bool IF_ShootBase = NO;
void Set_ShootBase(void)
{
  IF_ShootBase = YES;
}
void Reset_ShootBase(void)
{
  IF_ShootBase = NO;
}
bool Report_IF_ShootBase(void)
{
  return IF_ShootBase;
}
Color_t Get_RC_ctrlColor(Color_t prev)
{
  Color_t res;
  switch(Report_S2())
  {
    case 1:
      res = I_AM_BLUE;//上拨为蓝色，打红色
      break;

    case 2:
      res = I_AM_RED; //下拨为红色，打蓝色
      break;
      
    case 3:
      res = prev;//拨到中间自动识别
      break;
  }
  return res;
}

Color_t Judge_MyColor(uint8_t robotID)
{
  Color_t res = UNKNOWN;
  
  if(robotID == 3 || robotID == 4 || robotID == 5)
    res = I_AM_RED;
  if(robotID == 103 || robotID == 104 || robotID == 105)
    res = I_AM_BLUE;
  if(Report_IF_ShootBase())
    res = BASE;
  
  res = Get_RC_ctrlColor(res);//遥控器s2拨动控制自瞄识别的颜色，上打红 下打蓝
  
  return res;
}


/**
 *	@brief 键盘自瞄开启
 *	@note
 */
int AutoAim_start_cnt = 0;
System_Action_t Vision_Handler(void)
{
  System_Action_t res = SYS_ACT_NORMOL;
  switch(Vision.System_Cmd_Id)
  {
    case CMD_AIM_AUTO:
      AutoAim_ON();
      res = SYS_ACT_AUTO_AIM;
      break;
    
    case CMD_AIM_SENTRY:
      AutoAim_ON();
      res = SYS_ACT_AUTO_AIM;
      break;
    
    case CMD_AIM_SMALL_BUFF:
//      BuffAim_ON();//启动打符
      res = SYS_ACT_SMALL_BUFF;
      break;
    
    case CMD_AIM_BIG_BUFF:
//      BuffAim_ON();//启动打符
      res = SYS_ACT_BIG_BUFF;
      break;
    
  }
  return res;
}

void Vision_Cmd_Reset(void)
{
  //自瞄模式为初始视觉模式
  Vision.System_Cmd_Id = CMD_AIM_AUTO;
}

void Vision_OFF(void)
{
  AutoAim_start_cnt = 0;
  Vision_Cmd_Reset();//视觉模式恢复成普通自瞄模式
  
  AutoAim_OFF();//自瞄关闭
  BuffAim_OFF();//打符关闭
}
  
/**
 * @brief YAW轴当视觉反馈识别不到目标时可以自由运动
 * @param 
 */
float YawVision_nolook(void)
{
  return -Gimbal.YAW.motor_data.PID_Angle + \
          Gimbal.YAW.motor_data.PID_Angle_target + \
          Gimbal.YAW.Key_Move.AXIS_Angle_k * Mouse_X_Speed();
}
/**
 * @brief Pit轴当视觉反馈识别不到目标时可以自由运动
 * @param 
 */
float PitVision_nolook(void)
{
  return -Gimbal.PIT.motor_data.PID_Angle + \
          Gimbal.PIT.motor_data.PID_Angle_target + \
          Gimbal.PIT.Key_Move.AXIS_Angle_k * Mouse_Y_Speed();
}


float YAW_GetAutoAimErr(void)
{
  return (float)Vision.VisionRTx.AutoAim_Rx.Packet.RxData.yaw_angle;
}
float PIT_GetAutoAimErr(void)
{
  return (float)Vision.VisionRTx.AutoAim_Rx.Packet.RxData.pitch_angle;
}
float Get_VisionDistance(void)
{
  return ((float)Vision.VisionRTx.AutoAim_Rx.Packet.RxData.distance)/1000;
}
float YAW_GetBuffCoordinate(void)
{
  return (float)Vision.VisionRTx.BuffAim_RX.Packet.RxData.yaw_angle;
}
float PIT_GetBuffCoordinate(void)
{
  return (float)Vision.VisionRTx.BuffAim_RX.Packet.RxData.pitch_angle;
}

/*计算帧率*/
float data_cnt = 0;
float time_start = 0;
float time_end = 0;
float prev_t = 0 ;
bool cnt(void)
{
  bool res = false; 
    time_start = millis();   
  
    //记录两帧时间间隔
    Vision.FrameRate_Cnt.FrameTime = time_start - prev_t;
  
    if(  data_cnt == 0)
    {
      time_end = time_start +1000;
    }
    if(time_start<time_end)
    {
        data_cnt += 1;
    }
    else 
    {
      res = true;
      Vision.FrameRate_Cnt.FrameRate =   data_cnt;
        data_cnt = 0;
//      time_end = 0;
//      time_start = 0;
    }  
    prev_t = time_start;
    return res;
}
/*返回两帧时间*/
float Get_FrameTime(void)
{
  float res;
  res = Vision.FrameRate_Cnt.FrameTime;
  if(res != 0)
    return Vision.FrameRate_Cnt.FrameTime;
  else 
    return 1;
}
//------------------------驱动层------------------------
void Vision_Init(void)
{
  Vision.VisionRTx.AutoAim_Rx.LEN.LEN_RX_DATA = sizeof(AutoAim_Rx_Data_t);
  Vision.VisionRTx.AutoAim_Rx.LEN.LEN_RX_PACKET = sizeof(AutoAim_Rx_Packet_t);
  
  Vision.VisionRTx.AutoAim_Tx.LEN.LEN_TX_DATA = sizeof(AutoAim_Tx_Data_t);
  Vision.VisionRTx.AutoAim_Tx.LEN.LEN_TX_PACKET = sizeof(AutoAim_Tx_Packet_t);
  Vision.VisionRTx.AutoAim_Tx.LEN.TX_CRC16 = sizeof(AutoAim_Tx_Packet_t) - 2;
  
  Vision.VisionRTx.BuffAim_RX.LEN.LEN_RX_DATA = sizeof(BuffAim_Rx_Data_t);
  Vision.VisionRTx.BuffAim_RX.LEN.LEN_RX_PACKET = sizeof(BuffAim_Rx_Packet_t);
  
  Vision.VisionRTx.BuffAim_Tx.LEN.LEN_TX_DATA = sizeof(BuffAim_Tx_Data_t);
  Vision.VisionRTx.BuffAim_Tx.LEN.LEN_TX_PACKET = sizeof(BuffAim_Tx_Packet_t);
  Vision.VisionRTx.BuffAim_Tx.LEN.TX_CRC16 = sizeof(BuffAim_Tx_Packet_t) - 2;
}


/**
 *	@brief	读取视觉通信数据
 *	@return false - 数据错误
 *			    true  - 数据正确	
 *	@note	uart4.c中IRQ调用
 */
bool VISION_ReadData(uint8_t *rxBuf)
{
  BuffAim_Rx_Info_t *BUFF = &Vision.VisionRTx.BuffAim_RX;
  AutoAim_Rx_Info_t *AUTO = &Vision.VisionRTx.AutoAim_Rx;
  bool res = false;
	/* 帧首字节是否为0xA5 */
	if(rxBuf[SOF_Vision] == VISION_FRAME_HEADER) 
	{	
		/* 帧头CRC8校验*/
		if(Verify_CRC8_Check_Sum( rxBuf, LEN_FRAME_HEADER ))
		{
      //开始区分打符和自瞄的视觉信息
			if(Verify_CRC16_Check_Sum( rxBuf, BUFF->LEN.LEN_RX_PACKET ))
      {    /* 数据正确则拷贝接收包 */
        switch((Vision_Cmd_Id_t)rxBuf[CMD_ID_Vision])
        {
          case CMD_AIM_AUTO:
              memcpy(&AUTO->Packet, rxBuf, AUTO->LEN.LEN_RX_PACKET);  
              YawAutoAim_Data_Update = true;//表明自瞄数据已经更新
              PitAutoAim_Data_Update = true;
              res = true;
              break;
          case CMD_AIM_SMALL_BUFF:
              memcpy(&BUFF->Packet, rxBuf, BUFF->LEN.LEN_RX_PACKET);
              Check_IF_BuffArmorSwitch();
              YawBuffAim_Data_Update = true;//表明自瞄数据已经更新
              PitBuffAim_Data_Update = true;
              res = true;
              break;
          case CMD_AIM_BIG_BUFF:
              memcpy(&BUFF->Packet, rxBuf, BUFF->LEN.LEN_RX_PACKET);
              Check_IF_BuffArmorSwitch();
              YawBuffAim_Data_Update = true;//表明自瞄数据已经更新
              PitBuffAim_Data_Update = true;
              res = true;
              break;
          case CMD_AIM_SENTRY:
              memcpy(&AUTO->Packet, rxBuf, AUTO->LEN.LEN_RX_PACKET);  
              YawAutoAim_Data_Update = true;//表明自瞄数据已经更新
              PitAutoAim_Data_Update = true;
              res = true;
              break;
          default :
            break;
        }
      }else res = false;
    }else res = false;
	}else res = false;
  
  /*计算帧率*/
  Vision.FrameRate_Cnt.IF_GET_CNT = cnt();
  
  return res;
}



/**
 *	@brief	发送视觉通信数据
 *	@note	uart4发送
 */
void VISION_SendData(Vision_Cmd_Id_t cmd_id)
{
  BuffAim_Tx_Info_t *BUFF = &Vision.VisionRTx.BuffAim_Tx;
  AutoAim_Tx_Info_t *AUTO = &Vision.VisionRTx.AutoAim_Tx;
  int len = 0;
  switch(cmd_id)
  {
    case CMD_AIM_AUTO:
        len = AUTO->LEN.LEN_TX_PACKET;
      	/* 写入帧头 */
        Vision_Tx_Buffer[SOF_Vision] = VISION_FRAME_HEADER;
        Vision_Tx_Buffer[CMD_ID_Vision] = cmd_id;
        /* 写入帧头CRC8 */
        Append_CRC8_Check_Sum( Vision_Tx_Buffer, LEN_FRAME_HEADER);
        /* 数据段填充 */
        memcpy( &Vision_Tx_Buffer[DATA_Vision], 
                &AUTO->Packet.TxData, 
                 AUTO->LEN.LEN_TX_DATA );
        /* 写入帧尾CRC16 */
        Append_CRC16_Check_Sum( Vision_Tx_Buffer, len);
        
        /* 数据同步 */
        memcpy(&AUTO->Packet , Vision_Tx_Buffer , len);
      break;
    
    case CMD_AIM_SENTRY:
        len = AUTO->LEN.LEN_TX_PACKET;
      	/* 写入帧头 */
        Vision_Tx_Buffer[SOF_Vision] = VISION_FRAME_HEADER;
        Vision_Tx_Buffer[CMD_ID_Vision] = cmd_id;
        /* 写入帧头CRC8 */
        Append_CRC8_Check_Sum( Vision_Tx_Buffer, LEN_FRAME_HEADER);
        /* 数据段填充 */
        memcpy( &Vision_Tx_Buffer[DATA_Vision], 
                &AUTO->Packet.TxData, 
                 AUTO->LEN.LEN_TX_DATA );
        /* 写入帧尾CRC16 */
        Append_CRC16_Check_Sum( Vision_Tx_Buffer, len);
        
        /* 数据同步 */
        memcpy(&AUTO->Packet , Vision_Tx_Buffer , len);
      break;
    
    case CMD_AIM_SMALL_BUFF:
        len = BUFF->LEN.LEN_TX_PACKET;
      	/* 写入帧头 */
        Vision_Tx_Buffer[SOF_Vision] = VISION_FRAME_HEADER;
        Vision_Tx_Buffer[CMD_ID_Vision] = cmd_id;
        /* 写入帧头CRC8 */
        Append_CRC8_Check_Sum( Vision_Tx_Buffer, LEN_FRAME_HEADER);
        /* 数据段填充 */
        memcpy( &Vision_Tx_Buffer[DATA_Vision], 
                &BUFF->Packet.TxData, 
                 BUFF->LEN.LEN_TX_DATA );
        /* 写入帧尾CRC16 */
        Append_CRC16_Check_Sum( Vision_Tx_Buffer, len);
        
        /* 数据同步 */
        memcpy(&BUFF->Packet , Vision_Tx_Buffer , len);      
      break;
    
    case CMD_AIM_BIG_BUFF: 
        len = BUFF->LEN.LEN_TX_PACKET;
      	/* 写入帧头 */
        Vision_Tx_Buffer[SOF_Vision] = VISION_FRAME_HEADER;
        Vision_Tx_Buffer[CMD_ID_Vision] = cmd_id;
        /* 写入帧头CRC8 */
        Append_CRC8_Check_Sum( Vision_Tx_Buffer, LEN_FRAME_HEADER);
        /* 数据段填充 */
        memcpy( &Vision_Tx_Buffer[DATA_Vision], 
                &BUFF->Packet.TxData, 
                 BUFF->LEN.LEN_TX_DATA );
        /* 写入帧尾CRC16 */
        Append_CRC16_Check_Sum( Vision_Tx_Buffer, len);
        
        /* 数据同步 */
        memcpy(&BUFF->Packet , Vision_Tx_Buffer , len);      
      break;
    
    case CMD_AIM_BASE:
      break;
    
    default :
        len = AUTO->LEN.LEN_TX_PACKET;
      	/* 写入帧头 */
        Vision_Tx_Buffer[SOF_Vision] = VISION_FRAME_HEADER;
        Vision_Tx_Buffer[CMD_ID_Vision] = cmd_id;
        /* 写入帧头CRC8 */
        Append_CRC8_Check_Sum( Vision_Tx_Buffer, LEN_FRAME_HEADER);
        /* 数据段填充 */
        memcpy( &Vision_Tx_Buffer[DATA_Vision], 
                &AUTO->Packet.TxData, 
                 AUTO->LEN.LEN_TX_DATA );
        /* 写入帧尾CRC16 */
        Append_CRC16_Check_Sum( Vision_Tx_Buffer, len);
        
        /* 数据同步 */
        memcpy(&AUTO->Packet , Vision_Tx_Buffer , len);
      break;
  }
  /* 发送数据 */
  /* DMA发送  */
  UART4_DMA_SentData(Vision_Tx_Buffer , len);
  
	/* 发送数据包清零 */
	memset(Vision_Tx_Buffer, 0, VISION_TX_BUFFER_LEN);
}




//---------------------------------------------------硬触发---------------------------------------------------------------
/**
 * @brief 硬触发IO初始化
 * @param 
 */
void VisionTrigger_Init(void)
{
	GPIO_InitTypeDef gpio;	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;	
	gpio.GPIO_Mode = GPIO_Mode_OUT;										
	gpio.GPIO_Pin = GPIO_Pin_13;  		
	GPIO_Init(GPIOD,&gpio);	
}

void Trigger_ON(void)
{
  Trigger_IO_UP;
}
void Trigger_OFF(void)
{
  Trigger_IO_DOWN;
}
bool Report_TriggerSignal(void)
{
  return Vision.Trigger_Signal;
}
void TriggerSignal_YES(void)
{
  Vision.Trigger_Signal = YES;
}
void TriggerSignal_NO(void)
{
  Vision.Trigger_Signal = NO;
}

void Trigger(void)
{ 
  switch(Report_TriggerSignal())
  {
    case YES:
        Trigger_ON();
        TriggerSignal_NO();
        switch(Vision.action_mode)
        {
          case SYS_ACT_AUTO_AIM:
//              AutoAim_Trigger();
              break;
          case SYS_ACT_BIG_BUFF:
              break;
          case SYS_ACT_SMALL_BUFF:
              break;
          
          default :
              break;
        }
        break;
    case NO:
        Trigger_OFF();
        TriggerSignal_YES();
        break;
    default :
        break;      
  }

}

//奇数触发，为了达到5ms触发
int trigger_cnt = 0;
void Trigger_5ms(void)
{ 
  trigger_cnt++;
  if(trigger_cnt == 2)
  {
    TriggerSignal_NO();
  }
  if(trigger_cnt == 5)
  {
    TriggerSignal_YES();
    trigger_cnt = 0;
  }
  switch(Report_TriggerSignal())
  {
    case YES:
        Trigger_ON();
        //TriggerSignal_NO();
        switch(Vision.action_mode)
        {
          case SYS_ACT_AUTO_AIM:
//              AutoAim_Trigger();
              break;
          case SYS_ACT_BIG_BUFF:
              break;
          case SYS_ACT_SMALL_BUFF:
              break;
          
          default :
              break;
        }
        break;
    case NO:
        Trigger_OFF();
        //TriggerSignal_YES();
        break;
    default :
        break;      
  }

}



