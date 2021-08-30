#include "TIM.h"

/**
 * @brief 摩擦轮pwm
 * @note  TIM3
 */
void PWM_Friction_Init(u16 arr,u16 psc)
{		
	GPIO_InitTypeDef PWM_GPIO;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  	//TIM3时钟使能    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  	//TIM3时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能PORTA时钟	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); //使能PORTA时钟	

	
	PWM_GPIO.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; //GPIOA6                     
	PWM_GPIO.GPIO_Mode = GPIO_Mode_AF;//复用功能
	PWM_GPIO.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_Init(GPIOA,&PWM_GPIO); //初始化PA
  
	PWM_GPIO.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13; //GPIOA6                     
	PWM_GPIO.GPIO_Mode = GPIO_Mode_AF;//复用功能
	PWM_GPIO.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_Init(GPIOD,&PWM_GPIO); //初始化PD 
 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM3); //GPIOA6复用位定时器3 
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM3); //GPIOA7复用位定时器3                
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4); //GPIOD12复用位定时器4 
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4); //GPIOD13复用位定时器4                
  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;     //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //0
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);//初始化定时器3
  TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//初始化定时器3
  
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse=0;
  
  //左
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC1
  TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC1
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);//
  
  TIM_ARRPreloadConfig(TIM3,ENABLE);//ARPE使能
  TIM_CtrlPWMOutputs(TIM3,ENABLE);//主输出使能
  TIM_Cmd(TIM3, ENABLE);  //使能TIM3

  //右
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC1
  TIM_OC2Init(TIM4, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC1
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);//
  
  TIM_ARRPreloadConfig(TIM4,ENABLE);//ARPE使能 	
  TIM_CtrlPWMOutputs(TIM4,ENABLE);//主输出使能
	TIM_Cmd(TIM4, ENABLE);  //使能TIM3
} 



/**
 * @brief 舵机pwm
 * @note  TIM1
 */
void PWM_Clip_Init(u16 arr,u16 psc)
{		
	GPIO_InitTypeDef PWM_GPIO;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  	//TIM1时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //使能PORTD时钟	
	
	PWM_GPIO.GPIO_Pin = GPIO_Pin_11; //GPIOE11
	PWM_GPIO.GPIO_Mode = GPIO_Mode_AF;//复用功能
	PWM_GPIO.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_Init(GPIOE,&PWM_GPIO); //初始化PE11
 
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_TIM1); //GPIOE11复用位定时器1

	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;     //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //0
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);//初始化定时器1
 
	//初始化TIM4 Channel 1 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OCInitStructure.TIM_Pulse=0;
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 OC2

  TIM_CtrlPWMOutputs(TIM1,ENABLE);//主输出使能

	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIM1在CCR2上的预装载寄存器
 
  TIM_ARRPreloadConfig(TIM1,ENABLE);//ARPE使能 
  
	//TIM_Cmd(TIM1, ENABLE);  //使能TIM1
  
  TIM1->CCR2 = 0;
} 



void Trigger_Cnt_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM5时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//初始化TIM5
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //允许定时器5更新中断
	TIM_Cmd(TIM5,ENABLE); //使能定时器5
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //定时器5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOA,GPIOE时钟
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; // KEY对应引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0 B1
} 

//按下为低电平
int cnt_LD = 0, cnt_RD = 0,cnt_LU = 0, cnt_RU = 0;
KEY_STATE_t key_state_L = up,key_state_R = up;
void KEY_SCAN(void)
{
  if(KEY_Left == 0)
    cnt_LD++;
  if(KEY_Righ == 0)
    cnt_RD++;
  if(KEY_Left == 1)
    cnt_LU++;
  if(KEY_Righ == 1)
    cnt_RU++;
  
  if(cnt_LD >= 5)
  {
    key_state_L = down;
    cnt_LD = 0;
  }
  if(cnt_RD >= 5)
  {
    key_state_R = down;
    cnt_RD = 0;
  }
  if(cnt_LU >= 5)
  {
    key_state_L = up;
    cnt_LU = 0;
  }
  if(cnt_RU >= 5)
  {
    key_state_R = up;
    cnt_RU = 0;
  }
}

bool Report_IF_TurnLeft_allow(void)
{
  bool res = NO;
  if(key_state_L == up)
    res = YES;
  else
    res = NO;
  res = YES;//不使用开关判断则加上
  return res;
}
bool Report_IF_TurnRigh_allow(void)
{
  bool res = NO;
  if(key_state_R == up)
    res = YES;
  else
    res = NO;
  res = YES;//不使用开关判断
  return res;
}



