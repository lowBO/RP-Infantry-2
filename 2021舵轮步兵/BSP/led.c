#include "led.h"

/**
 * @brief 总LED初始化
 * @param 
 */
void Led_Init(void)
{
  Hint_Led_Init();
    AimLED_Init();
}



/*------------------------驱动层-----------------------*/
/**
 * @brief 提示灯初始化
 * @param 
 */
void Hint_Led_Init(void)
{
	GPIO_InitTypeDef gpio;	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;	
	gpio.GPIO_Mode = GPIO_Mode_OUT;										
	gpio.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14;  		
	GPIO_Init(GPIOC,&gpio);	
	
	Green_Off;
	Red_Off;
	Blue_Off;
	Orange_Off;
}
/**
 * @brief 瞄准灯初始化
 * @param 
 */
void AimLED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
 
  GPIO_Init(GPIOD, &GPIO_InitStructure);//D9
  GPIO_ResetBits(GPIOD,GPIO_Pin_9);
    
}




/*------------------------任务层-----------------------*/
/**
 * @brief 系统正常提示灯
 * @note  绿灯跳动
 */
void System_Alive_Hint(void)
{
	static uint16_t green_twinkle_cnt = 0;
	green_twinkle_cnt++;
	if(green_twinkle_cnt > 500) {
		green_twinkle_cnt = 0;
		LED_GREEN = !LED_GREEN;
	}
	Red_Off;
	
}

/**
 * @brief 遥控丢失提示灯
 * @note  红灯跳动
 */
void RC_Lost_Hint(void)
{
  static uint16_t blue_twinkle_cnt = 0;
  blue_twinkle_cnt++;
	if(blue_twinkle_cnt > 250) {
		blue_twinkle_cnt = 0;
		LED_RED = !LED_RED;
  }
	Green_Off;
	Blue_Off;
	Orange_Off;
}

/**
 * @brief 遥控数据错误提示灯
 * @note  红灯常亮
 */
void RC_Err_Hint(void)
{
	Red_On;
	Green_Off;
	Blue_Off;
	Orange_Off;
}

/**
 * @brief NMI中断提示灯
 * @note  全部常亮
 */
void NMI_Handler_Hint(void)
{
	Red_On;
	Green_On;
	Blue_On;
	Orange_On;
}
  
void HardFault_Handler_Hint(void)
{
  static uint16_t HardFault_twinkle_cnt = 0;
  HardFault_twinkle_cnt++;
	if(HardFault_twinkle_cnt > 500) {
		HardFault_twinkle_cnt = 0;
		LED_RED = !LED_RED;
    LED_GREEN = !LED_GREEN;
    LED_BLUE = !LED_BLUE;
    LED_ORANGE = !LED_ORANGE;
  }
}

void BusFault_Handler_Hint(void)
{
  static uint16_t BusFault_twinkle_cnt = 0;
  BusFault_twinkle_cnt++;
	if(BusFault_twinkle_cnt > 2000) {
		BusFault_twinkle_cnt = 0;
		LED_RED = !LED_RED;
    LED_GREEN = !LED_GREEN;
    LED_BLUE = !LED_BLUE;
    LED_ORANGE = !LED_ORANGE;
  }
}


/**
 * @brief  遥控器s1模式转换提示灯
 * @param 
 */
void RC_Mode_Change_Hint(void)
{
  static int Hint_cout = 0;
  Hint_cout+=1; 
  if(Hint_cout == 1500)
  {
    Green_Off;
    Hint_LED = System_Alive_Hint;
    Hint_cout = 0;
  }
  else Green_On;
}

/**
 * @brief  系统状态灯
 * @note   函数指针 
 */
Hint_LED_t Hint_LED;
void System_LED_Hint(Hint_LED_t p)
{
  p();
}
