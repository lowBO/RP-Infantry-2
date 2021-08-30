#include "uart3.h"
#include "Device.h"
#include "Function.h"

/* Private macro -------------------------------------------------------------*/
/* TX */
#define    GPIO_TX                   GPIOB
#define    GPIO_PIN_TX               GPIO_Pin_10
#define    GPIO_PINSOURCE_TX         GPIO_PinSource10
#define    RCC_AHB1PERIPH_GPIO_TX    RCC_AHB1Periph_GPIOB

/* RX */
#define    GPIO_RX                   GPIOB
#define    GPIO_PIN_RX               GPIO_Pin_11
#define    GPIO_PINSOURCE_RX         GPIO_PinSource11
#define    RCC_AHB1PERIPH_GPIO_RX    RCC_AHB1Periph_GPIOB

/* DMA */
#define    DMA1_Stream_RX            DMA1_Stream1

/* Buffer Len */
#define    USART3_BUFFER_RX_LEN     2
/* Private variables ---------------------------------------------------------*/


uint8_t  Sonic_Buffer[ USART3_BUFFER_RX_LEN ] = {0};	//视觉发过来的数据暂存在这里

void USART3_DMA_Init(void)
{		
	DMA_InitTypeDef xCom3DMAInit;
	
	DMA_DeInit( DMA1_Stream_RX );
	xCom3DMAInit.DMA_Channel = DMA_Channel_4;
	xCom3DMAInit.DMA_PeripheralBaseAddr  = (uint32_t)&(USART3->DR);
	xCom3DMAInit.DMA_Memory0BaseAddr     = (uint32_t)Sonic_Buffer;
	xCom3DMAInit.DMA_DIR = DMA_DIR_PeripheralToMemory;//方向外设到存储器
	xCom3DMAInit.DMA_BufferSize = USART3_BUFFER_RX_LEN;
	xCom3DMAInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	xCom3DMAInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	xCom3DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	xCom3DMAInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	xCom3DMAInit.DMA_Mode = DMA_Mode_Circular;                             //DMA_Mode_Normal（只传送一次）
	xCom3DMAInit.DMA_Priority = DMA_Priority_VeryHigh;                     //中断等级
	xCom3DMAInit.DMA_FIFOMode = DMA_FIFOMode_Enable;                       //直接模式
	xCom3DMAInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;               
	xCom3DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 //存储器单次传输
	xCom3DMAInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         //外设单次传输

	DMA_Init( DMA1_Stream_RX, &xCom3DMAInit );
	DMA_Cmd( DMA1_Stream_RX, ENABLE);  // stream1
}

void USART3_Init(void)
{
	USART_InitTypeDef  xUsartInit;
	GPIO_InitTypeDef   xGpioInit;
	NVIC_InitTypeDef   xNvicInit;
  
  RCC_AHB1PeriphClockCmd( RCC_AHB1PERIPH_GPIO_TX | RCC_AHB1PERIPH_GPIO_RX, ENABLE );//GPIO时钟
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );//串口3时钟
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, ENABLE );

	GPIO_PinAFConfig( GPIO_TX, GPIO_PINSOURCE_TX, GPIO_AF_USART3 );
	GPIO_PinAFConfig( GPIO_RX, GPIO_PINSOURCE_RX, GPIO_AF_USART3 ); 

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
  
	USART_ITConfig( USART3, USART_IT_TC,  DISABLE);  
  USART_ITConfig( USART3, USART_IT_RXNE,DISABLE);  
	USART_ITConfig( USART3, USART_IT_IDLE, ENABLE); // 注意要配置成串口空闲中断 	
  
  USART3_DMA_Init( );	// 初始化usart3的DMA	
	USART_DMACmd( USART3, USART_DMAReq_Rx, ENABLE );
//	USART_DMACmd( USART3, USART_DMAReq_Tx, ENABLE );
  
	xNvicInit.NVIC_IRQChannel                    = USART3_IRQn;
	xNvicInit.NVIC_IRQChannelPreemptionPriority  = 3;//抢占
	xNvicInit.NVIC_IRQChannelSubPriority         = 0;//子
	xNvicInit.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init( &xNvicInit );	
  
	USART_Init( USART3, &xUsartInit );
	USART_Cmd( USART3, ENABLE );
  
}

/**
 *	@brief	串口3中断函数  (读取数据)
 */
void USART3_IRQHandler( void )
{
	uint8_t res;
  
	/* 空闲中断 */
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		res = res;	// 消除警告的空操作
		//根据ST官方手册,读一下SR和DR寄存器,IDLE才能再次使用,否则会一直进入中断,就会跟串口接收中断没区别
		res = USART3->SR ;
		res = USART3->DR ;//USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //清除标记位
		
		DMA_Cmd(DMA1_Stream_RX, DISABLE);
		
		res = USART3_BUFFER_RX_LEN - DMA_GetCurrDataCounter(DMA1_Stream_RX);
    
    Sonic_ReadData_FL(Sonic_Buffer); // 读取超声波数据
    
		memset(Sonic_Buffer, 0, USART3_BUFFER_RX_LEN);	// 读完之后内容清零
		DMA_Cmd(DMA1_Stream_RX, ENABLE);
    
	}
}

/**
  * @brief  串口一次发送一个字节数据
  * @param  自己打包好的要发给裁判的数据
  */
void USART3_SendChar(uint8_t cData)
{
	while (USART_GetFlagStatus( USART3, USART_FLAG_TC ) == RESET);
	
	USART_SendData( USART3, cData );   
}

