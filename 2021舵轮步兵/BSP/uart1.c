#include "uart1.h"
#include "Device.h"
#include "Function.h"


/* Private macro -------------------------------------------------------------*/
/* TX */
#define    GPIO_TX                   GPIOA
#define    GPIO_PIN_TX               GPIO_Pin_9
#define    GPIO_PINSOURCE_TX         GPIO_PinSource9
#define    RCC_AHB1PERIPH_GPIO_TX    RCC_AHB1Periph_GPIOA

/* RX */
#define    GPIO_RX                   GPIOA
#define    GPIO_PIN_RX               GPIO_Pin_10
#define    GPIO_PINSOURCE_RX         GPIO_PinSource10
#define    RCC_AHB1PERIPH_GPIO_RX    RCC_AHB1Periph_GPIOA

/* DMA */
#define    DMA2_STREAM_RX            DMA2_Stream2
#define	   DMA2_STREAM_TX			       DMA2_Stream7

/* Buffer Len */
#define    USART1_BUFFER_RX_LEN     20
#define	   USART1_BUFFER_TX_LEN		  256
/* Private variables ---------------------------------------------------------*/
uint8_t Usart1_Buffer_Rx[ USART1_BUFFER_RX_LEN ] = {0};	//发过来的数据暂存在这里
void USART1_RX_DMA_Init( void )
{
	DMA_InitTypeDef xCom1DMAInit;
	
	DMA_DeInit( DMA2_STREAM_RX );
	xCom1DMAInit.DMA_Channel = DMA_Channel_4;
	xCom1DMAInit.DMA_DIR = DMA_DIR_PeripheralToMemory;//方向外设到存储器
	xCom1DMAInit.DMA_PeripheralBaseAddr  = (uint32_t)&(USART1->DR);
	xCom1DMAInit.DMA_Memory0BaseAddr     = (uint32_t)Usart1_Buffer_Rx;
	xCom1DMAInit.DMA_BufferSize = USART1_BUFFER_RX_LEN;
	xCom1DMAInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	xCom1DMAInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	xCom1DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	xCom1DMAInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	xCom1DMAInit.DMA_Mode = DMA_Mode_Circular;
	xCom1DMAInit.DMA_Priority = DMA_Priority_VeryHigh;
	xCom1DMAInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
	xCom1DMAInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	xCom1DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	xCom1DMAInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init( DMA2_STREAM_RX, &xCom1DMAInit );	
	DMA_Cmd( DMA2_STREAM_RX, ENABLE);  // stream2	
}

/**
 *	@brief 串口1 DMA初始化
 */
void USART1_DMA_Init( void )
{		
	USART1_RX_DMA_Init();//只有发送
}/* API functions -------------------------------------------------------------*/
/**
 *	@brief	Zigbee通信串口初始化(USART1)
 */
void USART1_Init( void )
{
	USART_InitTypeDef  xUsartInit;
	GPIO_InitTypeDef   xGpioInit;
	NVIC_InitTypeDef   xNvicInit;

	RCC_AHB1PeriphClockCmd( RCC_AHB1PERIPH_GPIO_TX | RCC_AHB1PERIPH_GPIO_RX, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	GPIO_PinAFConfig( GPIO_TX, GPIO_PINSOURCE_TX, GPIO_AF_USART1 );
	GPIO_PinAFConfig( GPIO_RX, GPIO_PINSOURCE_RX, GPIO_AF_USART1 ); 

	/*----Configure GPIO----*/
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

	/*----Configure USART----*/
  if(Master == MASTER_DOWN)
    xUsartInit.USART_BaudRate            = 9600;  
  else 
    xUsartInit.USART_BaudRate            = 115200; 
  
	xUsartInit.USART_WordLength          = USART_WordLength_8b;
	xUsartInit.USART_StopBits            = USART_StopBits_1;
	xUsartInit.USART_Parity              = USART_Parity_No;
	xUsartInit.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
	xUsartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	
	USART_Init( USART1, &xUsartInit );
	USART_Cmd( USART1, ENABLE );
	
	USART_ITConfig( USART1, USART_IT_IDLE, ENABLE  ); // 注意要配置成串口空闲中断 
//	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );

	/*----Configure DMA----*/
	USART1_DMA_Init( );	// 初始化uart1的DMA
	USART_DMACmd( USART1, USART_DMAReq_Rx, ENABLE );
//	USART_DMACmd( USART1, USART_DMAReq_Tx, ENABLE );
//	

	
	/*----Configure NVIC----*/
	xNvicInit.NVIC_IRQChannel                    = USART1_IRQn;
	xNvicInit.NVIC_IRQChannelPreemptionPriority  = 2;
	xNvicInit.NVIC_IRQChannelSubPriority         = 0;
	xNvicInit.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init( &xNvicInit );
		
}

/**
 *	@brief	串口1中断函数
 */
void USART1_IRQHandler( void )
{
	uint8_t res;
	/* 空闲中断 */
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{		
		res = res;	// 消除警告的空操作
		//根据ST官方手册,读一下SR和DR寄存器,IDLE才能再次使用,否则会一直进入中断,就会跟串口接收中断没区别
		res = USART1->SR ;
		res = USART1->DR ;
		
		DMA_Cmd(DMA2_STREAM_RX, DISABLE);
		
		res = USART1_BUFFER_RX_LEN - DMA_GetCurrDataCounter(DMA2_STREAM_RX);
			
    Sonic_ReadData_Fly(Usart1_Buffer_Rx);
//    ZIGBEE_ReceiveData(Usart1_Buffer_Rx);
//		VISION_ReadData(Usart1_Buffer_Rx);		// 读取视觉数据
		
		memset(Usart1_Buffer_Rx, 0, USART1_BUFFER_RX_LEN);	// 读完之后内容清零
		DMA_Cmd(DMA2_STREAM_RX, ENABLE);
	}
}
/**
  * @brief  串口一次发送一个字节数据
  * @param  自己打包好的要发给裁判的数据
  * @retval void
  * @attention  串口移位发送
  */
void USART1_SendChar(uint8_t cData)
{
	while (USART_GetFlagStatus( USART1, USART_FLAG_TC ) == RESET);
	
	USART_SendData( USART1, cData );   
}
