/***      《 遥控器控制代码 》

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
 
#include "remote.h"
#include "Device.h"
#include "Function.h"

/*遥控器*/
#define RC_IO_CLOCK             RCC_AHB1Periph_GPIOA
#define RC_IO_RX                GPIO_Pin_3
#define RC_IO_Port              GPIOA
#define RC_IO_PinSource_RX      GPIO_PinSource3
#define Usart                   USART2//纯标识
 


volatile unsigned char sbus_rx_buffer[25];
RC_Ctl_t RC_Ctl;

/**
 * @brief 封装初始化函数 
 * @param 
 */
void Remote_Init(void)
{
  usart2_Init();
}
/**
 * @brief 初始化 
 * @param 
 */
void usart2_Init(void)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart2;
	DMA_InitTypeDef dma;
	NVIC_InitTypeDef nvic;
	
/* -------------- Enable Module Clock Source ----------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_USART2);

	gpio.GPIO_Pin = GPIO_Pin_3 ;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &gpio);
	
	USART_DeInit(USART2);
	usart2.USART_BaudRate = 100000;
	usart2.USART_WordLength = USART_WordLength_8b;
	usart2.USART_StopBits = USART_StopBits_1;
	usart2.USART_Parity = USART_Parity_Even;
	usart2.USART_Mode = USART_Mode_Rx;
	usart2.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART2,&usart2);
	USART_Cmd(USART2,ENABLE);
	
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);
/* -------------- Configure NVIC ---------------------------------------*/
	nvic.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
/* -------------- Configure DMA -----------------------------------------*/
	
	DMA_DeInit(DMA1_Stream5);
	dma.DMA_Channel = DMA_Channel_4;
	dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	dma.DMA_Memory0BaseAddr = (uint32_t)sbus_rx_buffer;
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma.DMA_BufferSize = 18;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_Mode = DMA_Mode_Circular;
	dma.DMA_Priority = DMA_Priority_VeryHigh;
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma.DMA_MemoryBurst = DMA_Mode_Normal;
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
	DMA_Init(DMA1_Stream5,&dma);
	DMA_ITConfig(DMA1_Stream5,DMA_IT_TC,ENABLE);//dma传输完成的中断
	DMA_Cmd(DMA1_Stream5,ENABLE);
}


void delay_ms1(unsigned int t)
{
	int i;
	for( i=0;i<t;i++)
	{
		int a=42000;
		while(a--);
	}
}

void Get_RC_Data(void)
{
  RC_Ctl.rc.ch0 = ((sbus_rx_buffer[0]| (sbus_rx_buffer[1] << 8)) & 0x07ff)-1024; // Channel 0
  RC_Ctl.rc.ch1 = (((sbus_rx_buffer[1] >> 3) | (sbus_rx_buffer[2] << 5)) & 0x07ff)-1024; // Channel 1
  RC_Ctl.rc.ch2 = (((sbus_rx_buffer[2] >> 6) | (sbus_rx_buffer[3] << 2) | (sbus_rx_buffer[4] << 10)) & 0x07ff)-1024;// Channel 2
  RC_Ctl.rc.ch3 = (((sbus_rx_buffer[4] >> 1) | (sbus_rx_buffer[5] << 7)) & 0x07ff)-1024; // Channel 3
  RC_Ctl.rc.s1 = ((sbus_rx_buffer[5] >> 4)& 0x000C) >> 2; // 左上角132
  RC_Ctl.rc.s2 = ((sbus_rx_buffer[5] >> 4)& 0x0003); // 右上角132
  RC_Ctl.mouse.x = sbus_rx_buffer[6] | (sbus_rx_buffer[7] << 8); // Mouse X 
  RC_Ctl.mouse.y = sbus_rx_buffer[8] | (sbus_rx_buffer[9] << 8); // Mouse Y 
  RC_Ctl.mouse.z = sbus_rx_buffer[10] | (sbus_rx_buffer[11] << 8); // Mouse Z axis
  RC_Ctl.mouse.press_l = sbus_rx_buffer[12]; // 左鼠  
  RC_Ctl.mouse.press_r = sbus_rx_buffer[13]; // 右鼠
  RC_Ctl.kb.key_code = sbus_rx_buffer[14] | (sbus_rx_buffer[15] << 8); //键盘值
  RC_Ctl.rc.sw = ((sbus_rx_buffer[16] | (sbus_rx_buffer[17] << 8)) & 0x07FF)-1024;//拨轮
}


extern uint32_t  Remote_time;
void DMA1_Stream5_IRQHandler()
{
	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
	{
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		Get_RC_Data();	
		Remote_time = micros() + 30000;
  }
}

void System_AllReset(void)
{
  CHAS_Stop();
   GIM_Stop();
        //reset CPU
				__set_FAULTMASK(1);
				NVIC_SystemReset();
}
/**
 * @brief 遥控器关闭、丢失、数据错误后的计时复位 
 * @note  打开一次遥控后关闭 等于 复位 
 */
int Reset_tim = 0;
bool IF_Allow_Reset_cnt = NO;
void Set_Reset_cnt_Allow(void)
{
  IF_Allow_Reset_cnt = YES;
}
void Reset_cnt(void)
{
  if(IF_Allow_Reset_cnt)
  {
    Reset_tim++;
  }
  if(Reset_tim > 2000)//2s
  {
    System_AllReset();
  }
}

/* --------------------------控制层------------------------- */
/**                   《检测遥控器（系统）状态》
 **************************************************************
 *                                                            *
 *   .=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-.       *
 *    |                     ______                     |      *
 *    |                  .-"      "-.                  |      *
 *    |                 /            \                 |      *
 *    |     _          |              |          _     |      *
 *    |    ( \         |,  .-.  .-.  ,|         / )    |      *
 *    |     > "=._     | )(__/  \__)( |     _.=" <     |      *
 *    |    (_/"=._"=._ |/     /\     \| _.="_.="\_)    |      *
 *    |           "=._"(_     ^^     _)"_.="           |      *
 *    |               "=\__|IIIIII|__/="               |      *
 *    |              _.="| \IIIIII/ |"=._              |      *
 *    |    _     _.="_.="\          /"=._"=._     _    |      *
 *    |   ( \_.="_.="     `--------`     "=._"=._/ )   |      *
 *    |    > _.="                            "=._ <    |      *
 *    |   (_/                                    \_)   |      *
 *    |                                                |      *
 *    '-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='      *
 *                                                            *
 *           スケルトンは問題なくコードを祝福します           *
 **************************************************************
 */

/**
 * @brief 遥控器主控
 * @param 
 */
void RC_Ctrl(void)
{
  /* 遥控器数据自检，然后上报系统改变状态 */
  RC_State_Report();
  /* 遥控器控制状态更新 */
  if(System.state == SYSTEM_NORMAL) 
  {
    RC_Mode_Ctrl();
    //Set_Reset_cnt_Allow();//允许计时复位
    
    if(GIM_CalOK_Report())
    //云台复位之后，启动按键开关、波轮功能控制
    switch (System.ctrl_mode)
    {
      case RC_CTRL_MODE:
        RC_Function_Ctrl();
        break;
      case KEY_CTRL_MODE:
        KEY_Function_Ctrl();
        break;
    }
  }
}

/**
 * @brief 遥控器数据状态上报
 * @param 
 */
void RC_State_Report(void)
{
  System.state = RC_Check();
}

/**
 * @brief 遥控器总自检
 * @param 返回状态编号
 */
System_State_t RC_Check(void)
{
  static System_State_t res;
  res = SYSTEM_NORMAL;
  if(IF_RC_LOST)
    res = SYSTEM_RCLOST;
  if(IF_RC_DATAERR)
    res = SYSTEM_RCERR;
  return res;
}

/**
 * @brief 遥控器数据自检
 * @param 
 */
bool Judge_RC_DataErr(void)
{
  static bool res ;
  res = false;
  if( ((RC_S1 != 1)&&(RC_S1 != 2)&&(RC_S1 != 3)) || ((RC_S2 != 1)&&(RC_S2 != 2)&&(RC_S2 != 3)) )
    res = true;
  else if(abs(RC_CH0)>660 ||
          abs(RC_CH1)>660 || 
          abs(RC_CH2)>660 ||
          abs(RC_CH3)>660 ||
          abs(RC_SW)>660) 
    res = true;
	return res;
}
/**
 * @brief 遥控器连接自检
 * @param 
 */
bool Judge_RC_Lost(void)
{
  static bool res ;
  res = false;
  if(micros() >= Remote_time)
    res = true;
  return res;
}

/**
 * @brief 遥控器数据出错处理
 * @param 
 */
void RC_Err_Process(void)
{
  /*遥控自我保护*/
  RC_Protect();
  
  /*系统遥控错误或丢失声明*/
  SYS_Err_Public();
  
  /*更新一次遥控器的 “先前控制状态” */
  prev_sys_state_Err(SYSTEM_RCERR);
  
  /*系统提示灯*/
  Hint_LED = RC_Err_Hint;
  
  /*键盘信息清空*/
  KEY_InfoReset();

  /*计时复位*/
  Reset_cnt();
  
}

/**
 * @brief 遥控器失联处理
 * @param 
 */
void RC_Lost_Process(void)
{
  /*遥控自我保护*/
  RC_Protect();
  
  /*系统遥控错误或丢失声明*/
  SYS_Err_Public();
  
  /*更新一次系统状态的 “先前健康状态” */
  prev_sys_state_Err(SYSTEM_RCLOST);
  
  /*系统提示灯*/
  Hint_LED = RC_Lost_Hint;
  
  /*键盘信息清空*/
  KEY_InfoReset();
  
  /*计时复位*/
  Reset_cnt();
}

/**
 * @brief 系统非正常状态下 分发到各个子状态的错误通知
 * @note  即系统状态复位 
 */
void SYS_Err_Public(void)
{
  System.pid_mode = PID_Err;
  System.ctrl_mode = Ctrl_Err;
  System.action_mode = ACT_Err;
  System.Defense_Mode = Defense_SPIN;
  RC_prev_s1_Err();
  RC_prev_s2_Err();
}

/**
 * @brief 遥控器自我保护
 * @param 
 */
void RC_Protect(void)
{
  RC_CH0 = RC_CH_VALUE_OFFSET;
  RC_CH1 = RC_CH_VALUE_OFFSET;
  RC_CH2 = RC_CH_VALUE_OFFSET;
  RC_CH3 = RC_CH_VALUE_OFFSET;
  RC_S1 = RC_S_MID;
  RC_S2 = RC_S_MID;	
  RC_SW = RC_SW_VALUE_OFFSET;
  MOUSE_X_MOVE_SPEED = 0;
  MOUSE_Y_MOVE_SPEED = 0;
  MOUSE_Z_MOVE_SPEED = 0;
  MOUSE_LEFT = 0;
  MOUSE_RIGH = 0;
  KEY_ALL_CODE = 0x0000;
}



/**    《遥控器步兵控制》   
 *           _____                    _____                          
 *          /\    \                  /\    \         
 *         /::\    \                /::\    \        
 *        /::::\    \              /::::\    \       
 *       /::::::\    \            /::::::\    \      
 *      /:::/\:::\    \          /:::/\:::\    \     
 *     /:::/__\:::\    \        /:::/  \:::\    \    
 *    /::::\   \:::\    \      /:::/    \:::\    \   
 *   /::::::\   \:::\    \    /:::/    / \:::\    \  
 *  /:::/\:::\   \:::\____\  /:::/    /   \:::\    \ 
 * /:::/  \:::\   \:::|    |/:::/____/     \:::\____\
 * \::/   |::::\  /:::|____|\:::\    \      \::/    /
 *  \/____|:::::\/:::/    /  \:::\    \      \/____/ 
 *        |:::::::::/    /    \:::\    \    
 *        |::|\::::/    /      \:::\    \   
 *        |::| \::/____/        \:::\    \    
 *        |::|  ~|               \:::\    \    
 *        |::|   |                \:::\    \         
 *        \::|   |                 \:::\____\        
 *         \:|   |                  \::/    /        
 *          \|___|                   \/____/   
 */ 


/**
 * @brief 遥控器控制更新
 * @note  只判断s1，因为s1是模式控制的一边
 */
void RC_Mode_Ctrl(void)
{
  switch(RC_Judge_s1_Change())
  {
    case RC_NO_CHANGE:
      break;
      
    case RC_MID_2_UP:
      /*中->上：机械->键盘*/  //遥控模式和键盘模式的切换
      System.ctrl_mode = KEY_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = GYRO;
      System.Defense_Mode = Defense_SPIN;
      KEY_InfoInit();
      break;
    
    case RC_UP_2_MID:
      /*上->中: 键盘->机械*/  //遥控模式和键盘模式的切换
      System.ctrl_mode = RC_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = MECH;
      System.Defense_Mode = Defense_SPIN;
      KEY_InfoReset();
      break;
    
    case RC_DOWM_2_MID:
      /*下->中： 陀螺仪->机械*/
      System.ctrl_mode = RC_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = MECH;
      break;    
    
    case RC_MID_2_DOWN:
      /*中->下： 机械->陀螺仪*/
      System.ctrl_mode = RC_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = GYRO;
      break;      
//-----------------------------------------
    case RC_Err_2_UP:
      /*错误->上： 错误->键盘*/
      System.ctrl_mode = KEY_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = GYRO;      
      break;
    
    case RC_Err_2_MID: 
      /*错误->中： 错误->机械*/
      System.ctrl_mode = RC_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = MECH;      
      break;
    
    case RC_Err_2_DOWN:
      /*错误->下： 错误->陀螺仪*/
      System.ctrl_mode = RC_CTRL_MODE;
      System.action_mode = SYS_ACT_NORMOL;
      System.pid_mode = GYRO;    
      break;
  }
}

/**
 * @brief 遥控器功能切换控制（遥控器操作的各种功能）
 * @param 
 */
void RC_Function_Ctrl(void)
{
  switch(System.pid_mode)
  {
    case GYRO:{
        switch(RC_Judge_s2_Change())
        {
          case RC_NO_CHANGE:
            
            break;
          case RC_MID_2_UP:
            /*中->上*/
            RC_MODULE_State_Update();
            break;
          case RC_UP_2_MID:
            /*上->中*/
            break;
          case RC_DOWM_2_MID:
            /*下->中*/
            ContinFire_OFF();
            break;      
          case RC_MID_2_DOWN:
            /*中->下*/
            ContinFire_ON();
            break;   
          
          case RC_Err_2_UP:
            /*错误->上*/
            break;
          case RC_Err_2_MID: 
            /*错误->中*/   
            break;
          case RC_Err_2_DOWN:
            /*错误->下*/  
            break;          
        }
      }break;

    case MECH:{
        switch(RC_Judge_s2_Change())
        {
          case RC_NO_CHANGE:
            
            break;
          case RC_MID_2_UP:
            /*中->上*/
            RC_Clip_State_Updata();
            break;
          case RC_UP_2_MID:
            /*上->中:*/
            break;
          case RC_DOWM_2_MID:
            /*下->中*/
            break;      
          case RC_MID_2_DOWN:
            /*中->下*/
            SingleFireRequest();
            break;        
          
          case RC_Err_2_UP:
            /*错误->上*/
            break;
          case RC_Err_2_MID: 
            /*错误->中*/   
            break;
          case RC_Err_2_DOWN:
            /*错误->下*/  
            break;            
        }  
      }break;    
  }
  switch(RC_Judge_sw_Change())
  {
    case SW_UP:
      SPIN_ON();
      break;
    case SW_DOWM:
      SPIN_OFF();
//      if(!CHAS_SPIN_Report())//遥控状态下如果没有启动陀螺则拨轮往下为切换防御模式
//        System.Defense_Mode = SPIN_Defense_Mode_Switch(System.Defense_Mode);
      break;
    case SW_MID:
      SPIN_CLOSING_Judge();
      break;
  }
}

/**
 * @brief 遥控s1改变时反馈信息(添加了消抖)
 * @param 
 */
static uint8_t prev_s1 = RC_S_MID ;
RC_S_Change_t RC_Judge_s1_Change(void)
{
  static int mode_change_cnt = 0;
  static RC_S_Change_t res;
  if(prev_s1 != RC_S1)
  {
    mode_change_cnt += 1;
    if(mode_change_cnt > RC_S_TIM)//10ms
    {
      if(prev_s1 == RC_S_MID && RC_S1 == RC_S_DOWN)
        res = RC_MID_2_DOWN;
      if(prev_s1 == RC_S_MID && RC_S1 == RC_S_UP)
        res = RC_MID_2_UP;
      if(prev_s1 == RC_S_UP && RC_S1 == RC_S_MID)
        res = RC_UP_2_MID;
      if(prev_s1 == RC_S_DOWN && RC_S1 == RC_S_MID)
        res = RC_DOWM_2_MID;
      
      if(prev_s1 == RC_S_Err && RC_S1 == RC_S_UP)
        res = RC_Err_2_UP;
      if(prev_s1 == RC_S_Err && RC_S1 == RC_S_MID)
        res = RC_Err_2_MID;            
      if(prev_s1 == RC_S_Err && RC_S1 == RC_S_DOWN)
        res = RC_Err_2_DOWN;            
      
      /* 更新上一次遥控s1信息 */
      prev_s1 = RC_S1;  
      mode_change_cnt = 0;
      Hint_LED = RC_Mode_Change_Hint;
    }
  }
  else /* 模式没有发生变化 */
  {
    mode_change_cnt = 0;
    res = RC_NO_CHANGE;
  }
  return res;
}

/**
 * @brief 先前状态的改变 为了判断第一次进入，在RC错误处理中调用
 * @param 
 */
void RC_prev_s1_Err(void)
{
  prev_s1 = RC_S_Err;
}

/**
 * @brief 遥控器s2改变时反馈信息(添加了消抖)
 * @param 
 */
static uint8_t prev_s2 = RC_S_DOWN;
RC_S_Change_t RC_Judge_s2_Change(void)
{
  static int func_change_cnt = 0;
  static RC_S_Change_t res;
  if(prev_s2 != RC_S2)
  {
    func_change_cnt += 1;
    if(func_change_cnt > RC_S_TIM)//10ms
    {
      if(prev_s2 == RC_S_MID && RC_S2 == RC_S_DOWN)
        res = RC_MID_2_DOWN;
      if(prev_s2 == RC_S_MID && RC_S2 == RC_S_UP)
        res = RC_MID_2_UP;
      if(prev_s2 == RC_S_UP && RC_S2 == RC_S_MID)
        res = RC_UP_2_MID;
      if(prev_s2 == RC_S_DOWN && RC_S2 == RC_S_MID)
        res = RC_DOWM_2_MID;
      
      if(prev_s2 == RC_S_Err && RC_S2 == RC_S_UP)
        res = RC_Err_2_UP;
      if(prev_s2 == RC_S_Err && RC_S2 == RC_S_MID)
        res = RC_Err_2_MID;            
      if(prev_s2 == RC_S_Err && RC_S2 == RC_S_DOWN)
        res = RC_Err_2_DOWN;        
      
      /* 更新上一次遥控s2信息 */
      prev_s2 = RC_S2;  
      func_change_cnt = 0;
      Hint_LED = RC_Mode_Change_Hint;
    }
  }
  else /* 模式没有发生变化 */
  {
    func_change_cnt = 0;
    res = RC_NO_CHANGE;
  }
  return res;
}

/**
 * @brief 先前状态的改变 为了判断第一次进入，在RC错误处理中调用
 * @param 
 */
void RC_prev_s2_Err(void)
{
  prev_s2 = RC_S_Err;
}

/**
 * @brief 拨轮的判断
 * @param 
 */
RC_SW_Change_t RC_Judge_sw_Change(void)
{
  static int func_change_cnt = 0;
  RC_SW_Change_t res;
  switch(RC_SW)
  {
    case RC_SW_VALUE_MIN:
      func_change_cnt++;
      if(func_change_cnt > RC_S_TIM)
        res = SW_UP;
      break;
    case RC_SW_VALUE_MAX:
      func_change_cnt++;      
      if(func_change_cnt > RC_S_TIM)
        res = SW_DOWM;  
      break;
    default:
      func_change_cnt = 0;
      res = SW_MID;
      break;
  }
  return res;
}

uint8_t Report_S2(void)
{
  return RC_S2;
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
 * │ Ctrl│    │Alt │         Space         │ Alt│    │    │Ctrl│ │ ← │ ↓ │ → │ │   0   │ . │←─┘│
 * └─────┴────┴────┴───────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘ └───────┴───┴───┘
 */

/* Init start */
Keyboard_Info_t Keyboard ;//按键信息结构体
/* Init end   */


/**
 * @brief 按键信息结构体初始化
 * @note  全部置0
 * @param 
 */
void KEY_InfoInit(void)
{//进入键盘
  memset(&Keyboard,0,sizeof(Keyboard_Info_t));
  Function_turnOFF();
  Reset_CarHalfMove();//复位斜向模式，初始状态为关闭
  KEY_Move_Init();
}
void KEY_InfoReset(void)
{//退出键盘
  memset(&Keyboard,0,sizeof(Keyboard_Info_t));
  SuperCap_KEY_OFF();//电容指令设置为关，因为电容信息会一直发
  Reset_CarHalfMove();//复位斜向模式，初始状态为关闭
  KEY_Move_Init();
}

void Key_W(void)
{
  KEY_State_Judge(&Keyboard.W ,KEY_W , CHANGE_TIM , LONG_CHANGE_TIM_W);
  switch(Keyboard.W.State)
  {
    case UP:
      break;
    case PRESS:
      UserSet_W_move(YES);
      if(Report_IF_Buff_AutoShoot_ON())PIT_KeyFix_UP();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      CLIP_Set_CloseNum();
      //if(Report_IF_Buff_AutoShoot_ON())PIT_KeyFix_UP_CON();
      break;
    case RELAX:
      UserSet_W_move(NO);
      break;
  }
}

void Key_S(void)
{
  KEY_State_Judge(&Keyboard.S ,KEY_S, CHANGE_TIM , LONG_CHANGE_TIM_S);
  switch(Keyboard.S.State)
  {
    case UP:
      break;
    case PRESS:
      UserSet_S_move(YES);
      if(Report_IF_Buff_AutoShoot_ON())PIT_KeyFix_DOWN();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      CLIP_Set_CloseNum();
      //if(Report_IF_Buff_AutoShoot_ON())PIT_KeyFix_DOWN_CON();
      break;
    case RELAX:
      UserSet_S_move(NO);
      break;
  }
}

void Key_D(void)
{
  KEY_State_Judge(&Keyboard.D ,KEY_D, CHANGE_TIM , LONG_CHANGE_TIM_D);
  switch(Keyboard.D.State)
  {
    case UP:
      break;
    case PRESS:
      UserSet_D_move(YES);
      if(Report_IF_Buff_AutoShoot_ON())YAW_KeyFix_DOWN();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      //if(Report_IF_Buff_AutoShoot_ON())YAW_KeyFix_DOWN_CON();
      break;
    case RELAX:
      UserSet_D_move(NO);
      break;
  }
}

void Key_A(void)
{
  KEY_State_Judge(&Keyboard.A ,KEY_A, CHANGE_TIM , LONG_CHANGE_TIM_A);
  switch(Keyboard.A.State)
  {
    case UP:
      break;
    case PRESS:
      UserSet_A_move(YES);
      if(Report_IF_Buff_AutoShoot_ON())YAW_KeyFix_UP();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      //if(Report_IF_Buff_AutoShoot_ON())YAW_KeyFix_UP_CON();
      break;
    case RELAX:
      UserSet_A_move(NO);
      break;
  }
}

float MECH_SlowlyTurn = 0;
void MECH_SlowlyTurnLeft(void)
{
  MECH_SlowlyTurn = 20;
}
void MECH_SlowlyTurnRigh(void)
{
  MECH_SlowlyTurn = -20;
}
void MECH_SlowlyTurnReset(void)
{
  MECH_SlowlyTurn = 0;
}
float Report_MECH_SlowlyTurn(void)
{
  return MECH_SlowlyTurn;
}

void Key_Q(void)
{
  KEY_State_Judge(&Keyboard.Q ,KEY_Q, CHANGE_TIM , LONG_CHANGE_TIM_Q);
  switch(Keyboard.Q.State)
  {
    case UP: //抬起状态
      break;
    case PRESS: //按下瞬间
      if(KEY_CTRL || KEY_B || Report_IF_RUNNING())
        MECH_SlowlyTurnLeft();
      else if(!MOUSE_RIGH)
        TurnLeft_90();
      
      if(MOUSE_RIGH)
        PIT_AUTO_ADD();
      break;
    case SHORT_DOWN: //短按
      break;
    case DOWN: //长按
      break;
    case RELAX: //松开瞬间
      MECH_SlowlyTurnReset();
      break;
  }
}

void Key_E(void)
{
  KEY_State_Judge(&Keyboard.E ,KEY_E, CHANGE_TIM , LONG_CHANGE_TIM_E);
  switch(Keyboard.E.State)
  {
    case UP:
      break;
    case PRESS:
      if(KEY_CTRL || KEY_B || Report_IF_RUNNING())
        MECH_SlowlyTurnRigh();
      else if(!MOUSE_RIGH)
        TurnRigh_90();
      
      if(MOUSE_RIGH)
        PIT_AUTO_DEC();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      break;
    case RELAX:
      MECH_SlowlyTurnReset();
      break;
  }
}
void Key_R(void)
{
  KEY_State_Judge(&Keyboard.R ,KEY_R, CHANGE_TIM , LONG_CHANGE_TIM_R);
  switch(Keyboard.R.State)
  {
    case UP:
      break;
    case PRESS:
      ContinFire_ON();
//      if(MOUSE_LEFT)
//      {
//        ContinFreq_20();
//        Brust_ON();//热量解禁爆发
//      }
//      else 
      ContinFreq_15();//12射频
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      ContinFreq_15();
      break;
    case RELAX:
      ContinFreq_12();
      ContinFire_OFF();
//      Brust_OFF();//关闭热量解禁
      break;
  }
}
void Key_F(void)
{
  KEY_State_Judge(&Keyboard.F ,KEY_F, CHANGE_TIM , LONG_CHANGE_TIM_F);
  switch(Keyboard.F.State)
  {
    case UP:
      SPIN_CLOSING_Judge();
      break;
    case PRESS:
//      SPIN_KEY_Switch();
      SPIN_ON();
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      break;
    case RELAX:
      break;
  }
}
void Key_G(void)
{
  KEY_State_Judge(&Keyboard.G ,KEY_G, CHANGE_TIM , LONG_CHANGE_TIM_G);
  switch(Keyboard.G.State)
  {
    case UP:
      break;
    case PRESS:
      Reset_anti_GYRO();//启动预测
      Gim_Sentry_UP();//打哨兵的快速抬头,在GIMBAL.c中已经有了设置打哨兵的抬头角度，但是此处加入是为了在右键点下的时候也能抬头
      Set_IF_ShootSentry();
      System.action_mode = Vision_Handler();//视觉开启(按下开启)
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      break;
    case RELAX:
      Set_anti_GYRO();//关预测
      System.action_mode = SYS_ACT_NORMOL;//恢复普通模式
      AutoAim_OFF();//自瞄关闭
      Reset_IF_ShootSentry();
      break;
  }
}
void Key_Z(void)
{
  KEY_State_Judge(&Keyboard.Z ,KEY_Z, CHANGE_TIM , LONG_CHANGE_TIM_Z);
  switch(Keyboard.Z.State)
  {
    case UP:
      break;
    case PRESS:
//        Set_IF_BigBuff();
//        System.action_mode = Vision_Handler();//视觉开启
      Set_IF_SELF_CHANGE_Barrel_left();
      break;
    case SHORT_DOWN:
      break;    
    case DOWN:
      break;
    case RELAX:
//        Reset_IF_BuffMode();
//        System.action_mode = SYS_ACT_NORMOL;//恢复普通模式
      break;
  }
}

void Key_X(void)
{
  KEY_State_Judge(&Keyboard.X ,KEY_X, CHANGE_TIM , LONG_CHANGE_TIM_X);
  switch(Keyboard.X.State)
  {
    case UP:
      break;
    case PRESS:
//        Set_IF_SmallBuff();//小符
//        System.action_mode = Vision_Handler();//视觉开启
      Set_IF_SELF_CHANGE_Barrel_righ();
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      break;
    case RELAX:
//        Reset_IF_BuffMode();
//        System.action_mode = SYS_ACT_NORMOL;//恢复普通模式
      break;
  }
}
void Key_C(void)
{
  KEY_State_Judge(&Keyboard.C ,KEY_C, CHANGE_TIM , LONG_CHANGE_TIM_C);
  switch(Keyboard.C.State)
  {
    case UP:
      break;
    case PRESS:
      if(!Report_IF_RUNNING() && !MOUSE_RIGH && !KEY_CTRL)
      {
        if(KEY_A)
          TurnLeft_180();
        else TurnRigh_180();
      }
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      break;
    case RELAX:
      break;
  }
}

void Key_V(void)
{
  KEY_State_Judge(&Keyboard.V ,KEY_V, CHANGE_TIM , LONG_CHANGE_TIM_V);
  switch(Keyboard.V.State)
  {
    case UP:
      break;
    case PRESS:
        BuffAim_ON();//启动自动打符
        Reset_anti_GYRO();
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      break;
    case RELAX:
        BuffAim_OFF();//自动打符关闭
        Set_anti_GYRO();
      break;
  }
}

int cnt_B = 0;
void Key_B(void)
{
  KEY_State_Judge(&Keyboard.B ,KEY_B, CHANGE_TIM , LONG_CHANGE_TIM_B);
  switch(Keyboard.B.State)
  {
    case UP:
      if(cnt_B != 0)cnt_B--;
      break;
    case PRESS:
      System.action_mode = SYS_ACT_PARK;//park由对位改为缓慢模式，缓慢对位，缓慢上下坡
      if(cnt_B == 0)cnt_B = 1000;
      else 
      {
//        KEY_Clip_State_Updata();//双击打开弹舱
        CLIP_Set_OpenNum();
        cnt_B = 0;
      }
      
      if(KEY_Z && KEY_X)//这段代码用于通知视觉小电脑重启
      {
        Set_IF_SmallBuff();//小符
        System.action_mode = Vision_Handler();//视觉开启
      }
      break;
    case SHORT_DOWN:
      break;
    case DOWN:
      break;
    case RELAX:
      Reset_IF_BuffMode();
      System.action_mode = SYS_ACT_NORMOL;
      break;
  }
}

void Function_turnOFF(void)
{
  System.action_mode = SYS_ACT_NORMOL;//恢复普通模式
  
  SPIN_OFF();//陀螺关闭
  Vision_OFF();//视觉关闭
  SuperCap_KEY_OFF();//极速关闭
  //对位关闭
}
void Key_CTRL(void)
{
  KEY_State_Judge(&Keyboard.CTRL ,KEY_CTRL, CHANGE_TIM , LONG_CHANGE_TIM_CTRL);
  switch(Keyboard.CTRL.State)
  {
    case UP:
      break;
    case PRESS: 
//      if(KEY_SHIFT)
//        IF_CarHalfMove_Switch();//斜向模式的跳变开关
      if(KEY_B)Module_KEY_OFF();//摩擦轮激光关闭
      
      CLIP_Set_CloseNum();//弹舱关闭
      
      if(CHAS_SPIN_Close_Report() == NO && CHAS_SPIN_Report() )
        SPIN_OFF();//陀螺关闭
//      Set_IF_MECH_StopHalfMove();//机械模式下取消整车斜向
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
        System.pid_mode = MECH;
      break;
    case RELAX:
//      Reset_IF_MECH_StopHalfMove();//退出机械模式开启整车斜向
      System.pid_mode = GYRO;
      break;
  }
}
void Key_SHIFT(void)
{
  KEY_State_Judge(&Keyboard.SHIFT ,KEY_SHIFT, CHANGE_TIM , LONG_CHANGE_TIM_SHIFT);
  switch(Keyboard.SHIFT.State)
  {
    case UP:
      SuperCap_KEY_OFF();
      break;
    case PRESS:
      if(KEY_CTRL)
        IF_CarHalfMove_Switch();//斜向模式的跳变开关
      SuperCap_KEY_ON();
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      break;
    case RELAX:
      break;
  }
}

void Key_Mouse_L(void)
{
  KEY_State_Judge(&Keyboard.Mouse_L ,MOUSE_LEFT, CHANGE_TIM , LONG_CHANGE_TIM_MOUSE_L); 
  switch(Keyboard.Mouse_L.State)
  {
    case UP:
      break;
    case PRESS:
      if(!Report_IF_CLIP_OPEN())
        SingleFireRequest();  //单发请求
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      if(!Report_IF_CLIP_OPEN())
        ContinFire_ON();    //连发请求
      break;
    case RELAX:
      if(!KEY_R)
        ContinFire_OFF();   //连发关闭
      Module_KEY_ON();    //常规模块开启
      CLIP_Set_CloseNum();
      break;
  }
}

void Key_Mouse_R(void)
{
  KEY_State_Judge(&Keyboard.Mouse_R ,MOUSE_RIGH, CHANGE_TIM , LONG_CHANGE_TIM_MOUSE_R); 
  switch(Keyboard.Mouse_R.State)
  {
    case UP:
      //TriggerSignal_NO();//右键没按下就一直是关闭硬触发
      break;
    case PRESS:
      Set_IF_AutoAim();
      System.action_mode = Vision_Handler();//视觉开启
      //TriggerSignal_YES();//第一次硬触发
      break;
    case SHORT_DOWN:
      break;  
    case DOWN:
      break;
    case RELAX:
      System.action_mode = SYS_ACT_NORMOL;//恢复普通模式
      AutoAim_OFF();//自瞄关闭
      break;
  }
}

void multiKeyCtrl(void)
{
  
  
  
}

//键盘模式下的拨轮：用于复位
void KeySW_Func(void)
{
  static int Func_cnt = 0;
  switch(RC_Judge_sw_Change())
  {
    case SW_UP:
      Func_cnt = 0;
      break;
    case SW_DOWM:
      Func_cnt++;
      if(Func_cnt > 500)
      {
        Func_cnt = 0;
        System_AllReset();
      }
      break;
    case SW_MID:
      Func_cnt = 0;
      break;
  }
}



/**
 * @brief 键盘功能切换控制（键盘操作的各种功能）
 * @note 
 * @param 
 */
void KEY_Function_Ctrl(void)
{
  Key_W();
  Key_S();  
  Key_D();
  Key_A();
  Key_Q();
  Key_E();
  Key_R();
  Key_F();
  Key_G();
  Key_Z();
  Key_X();
  Key_C();
  Key_V();
  Key_B();
  Key_SHIFT();
  Key_CTRL();
  Key_Mouse_L();
  Key_Mouse_R();
  KeySW_Func();
}

/**
 * @brief 判断按键是否按下
 * @note 
 * @param 
 */
void FirstGetInto_KEY_PRESS(KEY_Info_t *str)
{
  if(str->prev_KEY_PRESS != str->KEY_PRESS)
  {
    str->state_cnt = 0;
    str->prev_KEY_PRESS = str->KEY_PRESS;
  }
}
void KEY_State_Judge(KEY_Info_t *str , uint8_t KEY_PRESS , int change_tim ,int long_change_tim)
{
  str->KEY_PRESS = KEY_PRESS;
  FirstGetInto_KEY_PRESS(str);
  switch(KEY_PRESS)
  {
    case KEY_UP:  {
      if(str->prev_State != UP) 
      {
        str->state_cnt++;
        if(str->state_cnt >= change_tim)  
        {
          str->State = RELAX;
          str->prev_State = RELAX;
          if(str->state_cnt >= change_tim + 1)  //抬起不分长短抬
          {
            str->State = UP;
            str->prev_State = UP;
          }
        }
      }else{str->state_cnt = 0;}
    }break;
    
    case KEY_DOWN:    {
      if(str->prev_State != DOWN) 
      {
        str->state_cnt++;
        if(str->state_cnt >= change_tim)  
        {
          str->State = PRESS;
          str->prev_State = PRESS;
          if(str->state_cnt >= change_tim + 1)
          {
            str->State = SHORT_DOWN;
            str->prev_State = SHORT_DOWN;
            if(str->state_cnt >= long_change_tim)  
            {
              str->State = DOWN;
              str->prev_State = DOWN;
            }
          }
        }
      }else{str->state_cnt = 0;}
    }break;
  }
}

/**
 * @brief 步兵特殊运动模式控制（普通、视觉、对位、极速）
 * @note 键盘模式中写
 * @param 
 */
void Infantry_Action_Ctrl(void)
{

}

/**
 * @brief WSAD键盘斜坡
 * @note 
 * @param 
 */
float KeySlope_FB(bool J)
{
  float res = 0;
  if(!J)
    switch((int)DUM_Report_CHAS_MaxPower())
    {
      case 60: res = KeySlope_FB_60W_NO;break;     
      case 80: res = KeySlope_FB_80W_NO  ;break;      
      case 100:res = KeySlope_FB_100W_NO  ;break;    
      case 120:res = KeySlope_FB_120W_NO  ;break;   
      default: res = KeySlope_FB_50W_NO;break;
    }
  else
    switch((int)DUM_Report_CHAS_MaxPower())
    {
      case 60: res = KeySlope_FB_60W_YES ;break;     
      case 80: res = KeySlope_FB_80W_YES ;break;      
      case 100:res = KeySlope_FB_100W_YES ;break;    
      case 120:res = KeySlope_FB_120W_YES;break;   
      default: res = KeySlope_FB_50W_YES ;break;
    }
  return res;
}
float KeySlope_LR(bool J)
{
  float res = 0;
  if(!J)
    switch((int)DUM_Report_CHAS_MaxPower())
    {
      case 60: res = KeySlope_LR_60W_NO ;break;     
      case 80: res = KeySlope_LR_80W_NO   ;break;      
      case 100:res = KeySlope_LR_100W_NO   ;break;    
      case 120:res = KeySlope_LR_120W_NO  ;break;   
      default: res = KeySlope_LR_50W_NO ;break;
    }
  else 
    switch((int)DUM_Report_CHAS_MaxPower())
    {
      case 60: res = KeySlope_LR_60W_YES   ;break;
      case 80: res = KeySlope_LR_80W_YES   ;break;
      case 100:res = KeySlope_LR_100W_YES  ;break;
      case 120:res = KeySlope_LR_120W_YES  ;break;
      default: res = KeySlope_LR_50W_YES   ;break;
    }
  return res;
}
float KEY_FB_Ctrl(void)
{
  float res;
  
  /* 将斜坡分段整 */
  if(abs(Keyboard.MoveInfo.FB_dir.MoveData) < 170)
    Keyboard.MoveInfo.FB_dir.Slope = KeySlope_FB(NO);//2
  else     Keyboard.MoveInfo.FB_dir.Slope = KeySlope_FB(YES);//15
  
  Slow(&Keyboard.MoveInfo.FB_dir.MoveData,
              Keyboard.MoveInfo.FB_dir.K * (KEY_W-KEY_S),
              Keyboard.MoveInfo.FB_dir.Slope);
  res = Keyboard.MoveInfo.FB_dir.MoveData;
  return res;
}
float KEY_LR_Ctrl(void)
{
  float res;
  
  /* 将斜坡分段整 */
  if(abs(Keyboard.MoveInfo.LR_dir.MoveData) < 150)
    Keyboard.MoveInfo.LR_dir.Slope = KeySlope_LR(NO);//2
  else     Keyboard.MoveInfo.LR_dir.Slope = KeySlope_LR(YES);//15
  
  Slow(&Keyboard.MoveInfo.LR_dir.MoveData,
              Keyboard.MoveInfo.LR_dir.K * (KEY_D-KEY_A),
              Keyboard.MoveInfo.LR_dir.Slope);
  res = Keyboard.MoveInfo.LR_dir.MoveData;
  return res;
}

/**
 * @brief 鼠标卡尔曼
 * @note  已加滑动滤波
 * @param 
 */
float SFX[SF_LENGTH]={0},SFY[SF_LENGTH]={0};
float SF(float t,float *slopeFilter,float res)
{
  for(int i = SF_LENGTH-1;i>0;i--)
  {
    slopeFilter[i] = slopeFilter[i-1];
  }slopeFilter[0] = t;
  for(int i = 0;i<SF_LENGTH;i++)
  {
    res += slopeFilter[i];
  }return (res/SF_LENGTH);
}
extKalman_t KF_Mouse_X_Speed,KF_Mouse_Y_Speed;
float Mouse_X_Speed(void)
{
  int16_t res;
  if(abs(MOUSE_X_MOVE_SPEED > Xmax))res = 0;
  else res = -SF(KalmanFilter(&KF_Mouse_X_Speed,(float)MOUSE_X_MOVE_SPEED),
                    Keyboard.MouseSF.SFX,0);
  return (float)res;
}
float Mouse_Y_Speed(void)
{
  int16_t res;
  if(abs(MOUSE_Y_MOVE_SPEED > Ymax))res = 0;
  else res = SF(KalmanFilter(&KF_Mouse_Y_Speed,(float)MOUSE_Y_MOVE_SPEED),
                    Keyboard.MouseSF.SFY,0);
  return (float)res;
}


/**
 * @brief 键盘移动控制的斜坡初始化
 * @note 
 * @param 
 */
void KEY_Move_Init (void)
{
  Keyboard.MoveInfo.FB_dir.K = 660 ;//按键最大值
  Keyboard.MoveInfo.FB_dir.Slope = 1.5;//按键灵敏度 4
  
  Keyboard.MoveInfo.LR_dir.K = 660 ;//按键最大值
  Keyboard.MoveInfo.LR_dir.Slope = 0.75;//按键灵敏度 4
}


/* -------------------------舵轮四轴方向控制死区---------------------------- */
int16_t RUD_RC_CH3_proc(void)
{
  int16_t res = 0;
  
  if(abs(RC_CH3) < RUD_RC_DEAD)
    res = 0;
  else res = RC_CH3;
  
  return res;
}
int16_t RUD_RC_CH2_proc(void)
{
  int16_t res = 0;
  
  if(abs(RC_CH2) < RUD_RC_DEAD)
    res = 0;
  else res = RC_CH2;
  
  return res;
}
int16_t RUD_RC_CH1_proc(void)
{
  int16_t res = 0;
  
  if(abs(RC_CH1) < RUD_RC_DEAD)
    res = 0;
  else res = RC_CH1;
  
  return res;
}
int16_t RUD_RC_CH0_proc(void)
{
  int16_t res = 0;
  
  if(abs(RC_CH0) < RUD_RC_DEAD)
    res = 0;
  else res = RC_CH0;
  
  return res;
}
/*-------------------舵轮键盘死区-----------------------*/
int16_t RUD_KEY_FB_proc(void)
{
  int16_t res = 0;
  
  if(abs(KEY_FB_Ctrl()) < RUD_KEY_DEAD)
    res = 0;
  else res = KEY_FB_Ctrl();
  
  return res;
}
int16_t RUD_KEY_LR_proc(void)
{
  int16_t res = 0;
  
  if(abs(KEY_LR_Ctrl()) < RUD_KEY_DEAD)
    res = 0;
  else res = KEY_LR_Ctrl();
  
  return res;
}













