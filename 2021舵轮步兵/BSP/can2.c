#include "can2.h"
#include "Device.h"
/*----CAN2_TX-----PB13----*/
/*----CAN2_RX-----PB12----*/



extern void Data_Decode(CanRxMsg *Rx , CAN_GET_DATA_t *str);

void CAN2_Init(void)
{
    CAN_InitTypeDef        can;
    CAN_FilterInitTypeDef  can_filter;
    GPIO_InitTypeDef       gpio;
    NVIC_InitTypeDef       nvic;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
//		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

    GPIO_PinAFConfig(GPIOB, CAN2_IO_PinSource_L, GPIO_AF_CAN2);
    GPIO_PinAFConfig(GPIOB, CAN2_IO_PinSource_H, GPIO_AF_CAN2); 

    gpio.GPIO_Pin = CAN2_IO_L | CAN2_IO_H ;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &gpio);

    nvic.NVIC_IRQChannel = CAN2_RX0_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
		
		nvic.NVIC_IRQChannel = CAN2_TX_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 3;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic); 

    CAN_DeInit(CAN2);
    CAN_StructInit(&can);

    can.CAN_TTCM = DISABLE;
    can.CAN_ABOM = ENABLE;    
    can.CAN_AWUM = ENABLE;    
    can.CAN_NART = DISABLE;    
    can.CAN_RFLM = DISABLE;    
    can.CAN_TXFP = ENABLE;     
    can.CAN_Mode = CAN_Mode_Normal; 
    can.CAN_SJW  = CAN_SJW_1tq;
    can.CAN_BS1 = CAN_BS1_9tq;
    can.CAN_BS2 = CAN_BS2_4tq;
    can.CAN_Prescaler = 3;   //CAN BaudRate 42/(1+9+4)/3=1Mbps
    CAN_Init(CAN2, &can);
    
    can_filter.CAN_FilterNumber=14;
    can_filter.CAN_FilterMode=CAN_FilterMode_IdMask;
    can_filter.CAN_FilterScale=CAN_FilterScale_32bit;
    can_filter.CAN_FilterIdHigh=0x0000;
    can_filter.CAN_FilterIdLow=0x0000;
    can_filter.CAN_FilterMaskIdHigh=0x0000;
    can_filter.CAN_FilterMaskIdLow=0x0000;
    can_filter.CAN_FilterFIFOAssignment=0;//the message which pass the filter save in fifo0 CAN_Filter_FIFO0
    can_filter.CAN_FilterActivation=ENABLE;
    CAN_FilterInit(&can_filter);
    
    CAN_ITConfig(CAN2,CAN_IT_FMP0,ENABLE);    ////FIFO0消息挂号中断允许
		CAN_ITConfig(CAN2,CAN_IT_TME,ENABLE);
} 

void CAN2_RX0_IRQHandler(void)
{
  CanRxMsg RxMessage;
  if (CAN_GetITStatus(CAN2,CAN_IT_FMP0)!= RESET) 
  {				 
      CAN_ClearITPendingBit(CAN2, CAN_IT_FMP0);
      CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
      CAN2_time = micros() + 30000;
  }
  switch(RxMessage.StdId)
  {
    case RIFLE_ID:
      Data_Decode(&RxMessage,&Rifle.Motor[RIFLE].motor_data.CAN_GetData);
      break;
  
//    case Fric_L_ID:
//      Data_Decode(&RxMessage,&Module.Friction.Fric[left].motor_Data.CAN_GetData);
//      break;
//    
//    case Fric_R_ID:
//      Data_Decode(&RxMessage,&Module.Friction.Fric[righ].motor_Data.CAN_GetData);
//      break;
    
    case RUD_LF_ID:
      Data_Decode(&RxMessage,&Rudder.Rudder_Axis[RUD_LF].motor_data.CAN_GetData);
      break;

    case RUD_RF_ID:
      Data_Decode(&RxMessage,&Rudder.Rudder_Axis[RUD_RF].motor_data.CAN_GetData);
      break;
    
    case RUD_LB_ID:
      Data_Decode(&RxMessage,&Rudder.Rudder_Axis[RUD_LB].motor_data.CAN_GetData);
      break;
    
    case RUD_RB_ID:
      Data_Decode(&RxMessage,&Rudder.Rudder_Axis[RUD_RB].motor_data.CAN_GetData);
      break;
    
    default:
      break;
  }
}

void CAN2_TX_IRQHandler(void)
{
	if (CAN_GetITStatus(CAN2,CAN_IT_TME)!= RESET) 
	{
		CAN_ClearITPendingBit(CAN2,CAN_IT_TME);
	}
}

CanTxMsg Tx_CAN2;
void CAN2_Send(uint32_t Equipment_ID,int16_t *Data)
{
	Tx_CAN2.StdId = Equipment_ID;					 //使用的扩展ID，电调820R标识符0X200
	Tx_CAN2.IDE = CAN_ID_STD;				 //标准模式
	Tx_CAN2.RTR = CAN_RTR_DATA;			 //数据帧RTR=0，远程帧RTR=1
	Tx_CAN2.DLC = 8;							 	 //数据长度为8字节

	Tx_CAN2.Data[0] = Data[0]>>8; 
	Tx_CAN2.Data[1] = Data[0];
	Tx_CAN2.Data[2] = Data[1]>>8; 
	Tx_CAN2.Data[3] = Data[1];
	Tx_CAN2.Data[4] = Data[2]>>8; 
	Tx_CAN2.Data[5] =	Data[2];
	Tx_CAN2.Data[6] = Data[3]>>8; 
	Tx_CAN2.Data[7] =	Data[3];

	CAN_Transmit(CAN2, &Tx_CAN2);	//发送数据
}

uint8_t CAN2_DUM_Send(uint32_t Equipment_ID,uint8_t *Data)
{
	Tx_CAN2.StdId = Equipment_ID;					 //使用的扩展ID，电调820R标识符0X200
	Tx_CAN2.IDE = CAN_ID_STD;				 //标准模式
	Tx_CAN2.RTR = CAN_RTR_DATA;			 //数据帧RTR=0，远程帧RTR=1
	Tx_CAN2.DLC = 8;							 	 //数据长度为8字节
  
  memcpy(Tx_CAN2.Data,Data,8);

	CAN_Transmit(CAN2, &Tx_CAN2);	//发送数据
  return 8;
}

/**
 * @brief CAN2系统是否连接
 * @param 
 */
bool Judge_IF_CAN2_Normal(void)
{
  bool res = true;
  if(micros() >= CAN2_time)
  {
    res = false;
  }
  return res;
}


