#include "Sonic.h"

Sonic_Info_t  Sonic_Info = {

  .Fly_Info.time = 0,
  .Fly_Info.distance = 0,
  
  .Park_Info.FL_TIM = 0,
  .Park_Info.FL_DIS = 0,
  
  .Park_Info.FR_TIM = 0,
  .Park_Info.FR_DIS = 0,
  
  .Park_Info.R_TIM = 0,
  .Park_Info.R_DIS = 0,
  
  .Fly_Info.IF_Data_Update = YES,
  .Park_Info.IF_Data_Update_FL = YES,
  .Park_Info.IF_Data_Update_FR = YES,
  .Park_Info.IF_Data_Update_R = YES,
};

void Sonic_Ctrl(void)
{
  if(Sonic_Info.Fly_Info.IF_Data_Update)
    KS103_FLY_Explore();
  
  if(Sonic_Info.Park_Info.IF_Data_Update_FL)
    KS103_FL_Explore();
  
  if(Sonic_Info.Park_Info.IF_Data_Update_FR)
    KS103_FR_Explore();
  
  if(Sonic_Info.Park_Info.IF_Data_Update_R)
    KS103_R_Explore();
}

#define SONIC_SPEED 0.34f  			//unit mm/us
void Sonic_ReadData_Fly(uint8_t *rxBuf)
{
  int16_t sonic_time = 0;
  //获取并且处理接收到的数据 
  sonic_time = (((uint16_t)(rxBuf[0])<<8) + (uint16_t)rxBuf[1]);
  //记录时间数据
	Sonic_Info.Fly_Info.time = sonic_time;
	//由时间计算出距离
	Sonic_Info.Fly_Info.distance = SONIC_SPEED * sonic_time /2.f;
  //数据更新标志位置1
	Sonic_Info.Fly_Info.IF_Data_Update = YES;
}

void Sonic_ReadData_FL(uint8_t *rxBuf)
{
  int16_t sonic_time = 0;
  //获取并且处理接收到的数据 
  sonic_time = (((uint16_t)(rxBuf[0])<<8) + (uint16_t)rxBuf[1]);
  //记录时间数据
	Sonic_Info.Park_Info.FL_TIM = sonic_time;
	//由时间计算出距离
	Sonic_Info.Park_Info.FL_DIS = SONIC_SPEED * sonic_time /2.f;
  //数据更新标志位置1
	Sonic_Info.Park_Info.IF_Data_Update_FL = YES;
}

void Sonic_ReadData_FR(uint8_t *rxBuf)
{
  int16_t sonic_time = 0;
  //获取并且处理接收到的数据 
  sonic_time = (((uint16_t)(rxBuf[0])<<8) + (uint16_t)rxBuf[1]);
  //记录时间数据
	Sonic_Info.Park_Info.FR_TIM = sonic_time;
	//由时间计算出距离
	Sonic_Info.Park_Info.FR_DIS = SONIC_SPEED * sonic_time /2.f;
  //数据更新标志位置1
	Sonic_Info.Park_Info.IF_Data_Update_FR = YES;
}

void Sonic_ReadData_R(uint8_t *rxBuf)
{
  int16_t sonic_time = 0;
  //获取并且处理接收到的数据 
  sonic_time = (((uint16_t)(rxBuf[0])<<8) + (uint16_t)rxBuf[1]);
  //记录时间数据
	Sonic_Info.Park_Info.R_TIM = sonic_time;
	//由时间计算出距离
	Sonic_Info.Park_Info.R_DIS = SONIC_SPEED * sonic_time /2.f;
  //数据更新标志位置1
	Sonic_Info.Park_Info.IF_Data_Update_R = YES;
}


//飞坡超声波
void KS103_FLY_Explore(void)
{
	//ks103配置  
	static uint8_t TxBuf[3] = {0xd8,0x02,0x0a};
	if(Sonic_Info.Fly_Info.IF_Data_Update)
	{
		USART1_SendChar(TxBuf[0]);
		delay_us(100);
		
		USART1_SendChar(TxBuf[1]);
		delay_us(100);
		
		USART1_SendChar(TxBuf[2]);
		
		Sonic_Info.Fly_Info.IF_Data_Update = NO;
	}
}
//左前对位超声波
void KS103_FL_Explore(void)
{
	//ks103配置  
	static uint8_t TxBuf[3] = {0xe8,0x02,0x0a};
	if(Sonic_Info.Park_Info.IF_Data_Update_FL)
	{
		UART4_SendChar(TxBuf[0]);
		delay_us(100);
		
		UART4_SendChar(TxBuf[1]);
		delay_us(100);
		
		UART4_SendChar(TxBuf[2]);
		
		Sonic_Info.Park_Info.IF_Data_Update_FL = NO;
	}
}
//右前对位超声波
void KS103_FR_Explore(void)
{
	//ks103配置  
	static uint8_t TxBuf[3] = {0xe8,0x02,0x0a};
	if(Sonic_Info.Park_Info.IF_Data_Update_FR)
	{
		USART3_SendChar(TxBuf[0]);
		delay_us(100);
		
		USART3_SendChar(TxBuf[1]);
		delay_us(100);
		
		USART3_SendChar(TxBuf[2]);
		
		Sonic_Info.Park_Info.IF_Data_Update_FR = NO;
	}
}
//侧身右边对位超声波
void KS103_R_Explore(void)
{
	//ks103配置  
	static uint8_t TxBuf[3] = {0xd8,0x02,0x0a};
	if(Sonic_Info.Park_Info.IF_Data_Update_R)
	{
		USART1_SendChar(TxBuf[0]);
		delay_us(100);
		
		USART1_SendChar(TxBuf[1]);
		delay_us(100);
		
		USART1_SendChar(TxBuf[2]);
		
		Sonic_Info.Park_Info.IF_Data_Update_R = NO;
	}
}



//判断是否起飞
Sonic_mode_t Judge_IF_Fly(void)
{
  Sonic_mode_t res;
  
  if(Sonic_Info.Fly_Info.distance >= FLY_DIS)
  {
    Sonic_Info.Sonic_mode = FLY;
    res = FLY;
  }
  else 
  {
    Sonic_Info.Sonic_mode = NO_FLY;
    res = NO_FLY;
  }
  
  return res;
}

/* 前后方向的对位判断 */
Sonic_mode_t Park_Judge_F(void)
{
  Sonic_mode_t res;
  res = Judge_IF_Fly();
  if(res != FLY)//没有飞坡
  {
    if(Sonic_Info.Park_Info.FL_DIS - FL_DIS_Standard >18 && 
       Sonic_Info.Park_Info.FR_DIS - FR_DIS_Standard >18  ) //距离很远
    {
      res = PARK_MoveFront;
    }
    else if(Sonic_Info.Park_Info.FL_DIS - FL_DIS_Standard < -18 && 
            Sonic_Info.Park_Info.FR_DIS - FR_DIS_Standard < -18  ) //距离很远
    {
      res = PARK_MoveBehind;
    }
    else //距离始终
    {
      res = PARK_FrontGood;
    }
  }
  return res;
}

Sonic_mode_t Park_Judge_Z(void)
{
  Sonic_mode_t res;
  res = Judge_IF_Fly();
  if(res != FLY)//没有飞坡
  {
    if(Sonic_Info.Park_Info.FL_DIS < Sonic_Info.Park_Info.FR_DIS - 25)//车头朝右
      res = PARK_TurnLeft;//左转
    else if(Sonic_Info.Park_Info.FL_DIS > Sonic_Info.Park_Info.FR_DIS + 25)//车头朝左
      res = PARK_TurnRigh;//右转
    else 
      res = PARK_TurnGood;
  }
  return res;
}


/* 左右方向的对位判断 */
Sonic_mode_t Park_Judge_R(void)
{
  Sonic_mode_t res;
  res = Judge_IF_Fly();
  if(res != FLY)//没有飞坡
  {
    if(Sonic_Info.Park_Info.R_DIS - R_DIS_Standard > 15)//右边距离太远
      res = PARK_MoveRigh;//右平移
    
    else if(Sonic_Info.Park_Info.R_DIS - R_DIS_Standard < -15)//右边距离太近
      res = PARK_MoveLeft;//左平移
    
    else 
      res = PARK_RighGood;
  }
 
  return res;
}



//-------------------------------上报底盘控制指令--------------------供chassis.c调用
//函数主体位于master_RTX.C中
extern Sonic_mode_t DUM_Report_Park_Fmove(void);
extern Sonic_mode_t DUM_Report_Park_Rmove(void);
extern Sonic_mode_t DUM_Report_Park_Zmove(void);

float CHAS_Park_FB_Speed(void)
{
  float res = 0;
  switch(DUM_Report_Park_Fmove())
  {
    case PARK_MoveFront:
        res = 110;
        break;
    
    case PARK_MoveBehind:
        res = -110;
        break;
    
    case PARK_FrontGood:
        res = 0;
        break;
    
    default:
        res = 0;
        break;
  }
  return res;
}

float CHAS_Park_LR_Speed(void)
{
  float res = 0;
  if(DUM_Report_Park_Fmove() == PARK_FrontGood)//前后对位完成
  switch(DUM_Report_Park_Rmove())
  {
    case PARK_MoveLeft:
        res = 110;
        break;
    
    case PARK_MoveRigh:
        res = -110;
        break;
    
    case PARK_RighGood:
        res = 0;
        break;
    
    default:
        res = 0;
        break;
  }
  return res;
}

float CHAS_Park_Z_Speed(void)
{
  float res = 0;
  if(DUM_Report_Park_Fmove() == PARK_FrontGood)//前后对位完成
  switch(DUM_Report_Park_Zmove())
  {
    case PARK_TurnLeft:
        res = 105;
        break;
    
    case PARK_TurnRigh:
        res = -105;
        break;
    
    case PARK_TurnGood:
        res = 0;
        break;
    
    default:
        res = 0;
        break;
  }
  return res;
}


