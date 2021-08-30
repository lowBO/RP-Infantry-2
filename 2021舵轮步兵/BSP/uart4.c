#include "uart4.h"
#include "Device.h"
#include "Function.h"

/* TX */
#define    GPIO_TX                   GPIOA
#define    GPIO_PIN_TX               GPIO_Pin_0
#define    GPIO_PINSOURCE_TX         GPIO_PinSource0
#define    RCC_AHB1PERIPH_GPIO_TX    RCC_AHB1Periph_GPIOA

/* RX */
#define    GPIO_RX                   GPIOA
#define    GPIO_PIN_RX               GPIO_Pin_1
#define    GPIO_PINSOURCE_RX         GPIO_PinSource1
#define    RCC_AHB1PERIPH_GPIO_RX    RCC_AHB1Periph_GPIOA

/* DMA */
#define    DMA1_Stream_RX            DMA1_Stream2
#define    DMA1_Stream_TX            DMA1_Stream4

/* Buffer Len */
#define    VISION_BUFFER_LEN         100

/* Private variables ---------------------------------------------------------*/
/* ## Global variables ## ----------------------------------------------------*/
uint8_t  Vision_Buffer[ VISION_BUFFER_LEN ] = {0};	//视觉发过来的数据暂存在这里
uint8_t  Vision_txBuffer[ VISION_BUFFER_LEN ] = {0}; //发送缓存
//float Yaw_error,Pitch_error;//在UART4中改变

//js看波形
float js_pitch_erro_V = 0,js_yaw_erro_V = 0;

void UART4_DMA_Init(void)
{		
	DMA_InitTypeDef xCom4DMAInit;
	
	DMA_DeInit( DMA1_Stream_RX );
	xCom4DMAInit.DMA_Channel = DMA_Channel_4;
	xCom4DMAInit.DMA_PeripheralBaseAddr  = (uint32_t)&(UART4->DR);
	xCom4DMAInit.DMA_Memory0BaseAddr     = (uint32_t)Vision_Buffer;
	xCom4DMAInit.DMA_DIR = DMA_DIR_PeripheralToMemory;//方向外设到存储器
	xCom4DMAInit.DMA_BufferSize = VISION_BUFFER_LEN;
	xCom4DMAInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	xCom4DMAInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	xCom4DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	xCom4DMAInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	xCom4DMAInit.DMA_Mode = DMA_Mode_Circular;                             //DMA_Mode_Normal（只传送一次）
	xCom4DMAInit.DMA_Priority = DMA_Priority_VeryHigh;                     //中断等级
	xCom4DMAInit.DMA_FIFOMode = DMA_FIFOMode_Enable;                       //直接模式
	xCom4DMAInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;               
	xCom4DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 //存储器单次传输
	xCom4DMAInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         //外设单次传输

	DMA_Init( DMA1_Stream_RX, &xCom4DMAInit );	
	DMA_Cmd( DMA1_Stream_RX, ENABLE);  // stream2
  
	DMA_DeInit( DMA1_Stream_TX );
	xCom4DMAInit.DMA_Channel = DMA_Channel_4;
	xCom4DMAInit.DMA_PeripheralBaseAddr  = (uint32_t)&(UART4->DR);
	xCom4DMAInit.DMA_Memory0BaseAddr     = (uint32_t)Vision_txBuffer;
	xCom4DMAInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	xCom4DMAInit.DMA_BufferSize = VISION_BUFFER_LEN;
	xCom4DMAInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	xCom4DMAInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	xCom4DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	xCom4DMAInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	xCom4DMAInit.DMA_Mode = DMA_Mode_Circular;                             //DMA_Mode_Normal（只传送一次）
	xCom4DMAInit.DMA_Priority = DMA_Priority_VeryHigh;                     //中断等级
	xCom4DMAInit.DMA_FIFOMode = DMA_FIFOMode_Enable;                       //直接模式
	xCom4DMAInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;               
	xCom4DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 //存储器单次传输
	xCom4DMAInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         //外设单次传输

	DMA_Init( DMA1_Stream_TX, &xCom4DMAInit );	
//	DMA_Cmd( DMA1_Stream_TX, ENABLE);  // stream2   发送dma不在这使能
  DMA_ITConfig(DMA1_Stream_TX , DMA_IT_TC , ENABLE);//发送完成中断
}

void UART4_Init(void)
{
	USART_InitTypeDef  xUsartInit;
	GPIO_InitTypeDef   xGpioInit;
	NVIC_InitTypeDef   xNvicInit;
  
  RCC_AHB1PeriphClockCmd( RCC_AHB1PERIPH_GPIO_TX | RCC_AHB1PERIPH_GPIO_RX, ENABLE );//GPIO时钟
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4, ENABLE );//串口4时钟

	GPIO_PinAFConfig( GPIO_TX, GPIO_PINSOURCE_TX, GPIO_AF_UART4 );
	GPIO_PinAFConfig( GPIO_RX, GPIO_PINSOURCE_RX, GPIO_AF_UART4 ); 

	xGpioInit.GPIO_Pin   = GPIO_PIN_TX;
	xGpioInit.GPIO_Mode  = GPIO_Mode_AF;
	xGpioInit.GPIO_OType = GPIO_OType_PP;
  
  if(Master == MASTER_DOWN)
    xGpioInit.GPIO_Speed = GPIO_Speed_50MHz;
  else 
    xGpioInit.GPIO_Speed = GPIO_Speed_100MHz;
  
	xGpioInit.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_Init( GPIO_TX, &xGpioInit );

	xGpioInit.GPIO_Pin = GPIO_PIN_RX;
	GPIO_Init( GPIO_RX, &xGpioInit );
 
  if(Master == MASTER_DOWN)
    xUsartInit.USART_BaudRate            = 9600;  
  else 
    xUsartInit.USART_BaudRate            = 115200;   
  
	xUsartInit.USART_WordLength          = USART_WordLength_8b;
	xUsartInit.USART_StopBits            = USART_StopBits_1;
	xUsartInit.USART_Parity              = USART_Parity_No;
	xUsartInit.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
	xUsartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	
	USART_Init( UART4, &xUsartInit );
	USART_Cmd( UART4, ENABLE );
	
	USART_ITConfig( UART4, USART_IT_IDLE, ENABLE  ); // 注意要配置成串口空闲中断 
  
	UART4_DMA_Init( );	// 初始化uart4的DMA
	USART_DMACmd( UART4, USART_DMAReq_Rx, ENABLE );
	USART_DMACmd( UART4, USART_DMAReq_Tx, ENABLE );
  
	xNvicInit.NVIC_IRQChannel                    = UART4_IRQn;
	xNvicInit.NVIC_IRQChannelPreemptionPriority  = 1;//抢占
	xNvicInit.NVIC_IRQChannelSubPriority         = 0;//子
	xNvicInit.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init( &xNvicInit );
  
	xNvicInit.NVIC_IRQChannel                    = DMA1_Stream4_IRQn;
	xNvicInit.NVIC_IRQChannelPreemptionPriority  = 1;//抢占
	xNvicInit.NVIC_IRQChannelSubPriority         = 1;//子
	xNvicInit.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init( &xNvicInit );
	 
}

/**
 *	@brief	串口4中断函数  (读取数据)
 */

void UART4_IRQHandler( void )
{
	uint8_t res;
	/* 空闲中断 */
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{		
		res = res;	// 消除警告的空操作
		//根据ST官方手册,读一下SR和DR寄存器,IDLE才能再次使用,否则会一直进入中断,就会跟串口接收中断没区别
		res = UART4->SR ;
		res = UART4->DR ;//USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //清除标记位
		
		DMA_Cmd(DMA1_Stream_RX, DISABLE);
		
		res = VISION_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream_RX);
		
//    /*提醒进行下一次硬触发*/
//    TriggerSignal_YES();
//    Sonic_ReadData(Vision_Buffer);
  if(Master == MASTER_DOWN)
    Sonic_ReadData_FR(Vision_Buffer);
  else 
    VISION_ReadData(Vision_Buffer);		// 读取视觉数据
    
		
    
    
		memset(Vision_Buffer, 0, VISION_BUFFER_LEN);	// 读完之后内容清零
		DMA_Cmd(DMA1_Stream_RX, ENABLE);
    
      Single_time_cal();
	}
}

//keyboard.c自瞄模式切换函数中赋值
//float fix_error_yaw = 0,fix_error_pitch = 0;//打符时：fix_error_yaw = 11,fix_error_pitch = -65;
/* 
 * 自瞄装甲板
 * 射速15 -> fix_error_pitch = 
 * 射速18 -> fix_error_pitch = 
 * 射速30 -> fix_error_pitch = 
*/

/**
 *  @fixerrorpitch 为弹道抬头量
  射速关系：
  摩擦轮659  -->  射速15  -->  fix_error_pitch为弹道抬头量 = -141,fix_error_yaw = 8 -->  子弹飞行时间0.75秒
  摩擦轮678  -->  射速18  -->  fix_error_pitch为弹道抬头量 = -65 ,fix_error_yaw = 8 -->  子弹飞行时间0.4秒
  摩擦轮705  -->  射速22  -->  fix_error_pitch为弹道抬头量 =     ,fix_error_yaw =    -->  子弹飞行时间0.25秒
  摩擦轮800  -->  射速30  -->  fix_error_pitch为弹道抬头量 = 30  ,fix_error_yaw = 8 -->  子弹飞行时间0.2秒
 */

//void Vision_Data_process(void)
//{
//  /* 打符和打装甲板 视觉反馈回来的信息种类不一样 buff返回坐标，armor返回误差 */    
//  if(VISION_GetFlagStatus(VISION_FLAG_LOCK_BUFF) == true)
//  {/* 打符mode */
//    if(AUTO_AIM_BUFF_FLAG == true)
//    {
//      Yaw_error = (Vision.RxPacket.RxData.yaw_angle - 640.f)*vision_error_inc;
//      Pitch_error = (Vision.RxPacket.RxData.pitch_angle - 360.f)*vision_error_inc ;
//    }
//  }
//  if(VISION_GetFlagStatus(VISION_FLAG_LOCK_ARMOR) == true)
//  {/* 打装甲板mode */
//    if(AUTO_AIM_ARMOR_FLAG == true)
//    {
//      Yaw_error = (Vision.RxPacket.RxData.yaw_angle )*vision_error_inc;
//      Pitch_error = (Vision.RxPacket.RxData.pitch_angle )*vision_error_inc ;
//      js_yaw_erro_V = Yaw_error;
//      js_pitch_erro_V = Pitch_error;
//    }
//  }
//  else if(VISION_GetFlagStatus(VISION_FLAG_LOCK_BUFF) != true && VISION_GetFlagStatus(VISION_FLAG_LOCK_ARMOR) != true)
//  {
//    Yaw_error = 0;
//    Pitch_error = 0;
//  }
//}
bool IF_Uart4_DMA_Busy = NO;
void DMA1_Stream4_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA1_Stream4 , DMA_IT_TCIF4) != RESET)
  {
    /* DMA发送完成 */
    DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_TCIF4);
//    DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_HTIF4);
//    DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_TEIF4);
//    DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_FEIF4);
//    DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_DMEIF4);
    DMA_Cmd(DMA1_Stream4 , DISABLE);
    
    IF_Uart4_DMA_Busy = NO;//DMA空闲
    /* 清空发送缓存 */
    memset(Vision_txBuffer , 0 , VISION_BUFFER_LEN);
  }
  if(DMA_GetITStatus(DMA1_Stream4 , DMA_IT_HTIF4) != RESET)
  {
  }
  if(DMA_GetITStatus(DMA1_Stream4 , DMA_IT_TEIF4) != RESET)
  {
  }
  if(DMA_GetITStatus(DMA1_Stream4 , DMA_IT_DMEIF4) != RESET)
  {
  }
  if(DMA_GetITStatus(DMA1_Stream4 , DMA_IT_FEIF4) != RESET)
  {
  }
}

void UART4_DMA_SentData(uint8_t *cData , int size)
{
  if(IF_Uart4_DMA_Busy == NO)
  {
    /* DMA没在发送 */
    for(int a = 0;a < size;a++)
    {
      Vision_txBuffer[a] = cData[a];
    }
    /* 修改DMA发送长度 */
    DMA_SetCurrDataCounter(DMA1_Stream4,size);
    /* DMA忙线 */
    IF_Uart4_DMA_Busy = YES;
    /* DMA发送 */
    DMA_Cmd(DMA1_Stream4 , ENABLE);
  }
} 


/**
  * @brief  串口一次发送一个字节数据
  * @param  自己打包好的要发给裁判的数据
  */
void UART4_SendChar(uint8_t cData)
{
	while (USART_GetFlagStatus( UART4, USART_FLAG_TC ) == RESET);
	
	USART_SendData( UART4, cData );   
}


