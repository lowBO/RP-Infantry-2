#include "referee.h"
#include "masters_RTX.h"
/* TX */
#define    GPIO_TX                   GPIOC
#define    GPIO_PIN_TX               GPIO_Pin_12
#define    GPIO_PINSOURCE_TX         GPIO_PinSource12
#define    RCC_AHB1PERIPH_GPIO_TX    RCC_AHB1Periph_GPIOC

/* RX */
#define    GPIO_RX                   GPIOD
#define    GPIO_PIN_RX               GPIO_Pin_2
#define    GPIO_PINSOURCE_RX         GPIO_PinSource2
#define    RCC_AHB1PERIPH_GPIO_RX    RCC_AHB1Periph_GPIOD

/* if use DMA */
#define    DMA1_Stream_RX            DMA1_Stream0

#define    COM5_PACKAGE_HEADER       JUDGE_FRAME_HEADER


// 裁判系统信息
Referee_info_t 	REF = {
  .IF_REF_ONL = NO,
};




bool Judge_Data_TF = FALSE;//裁判数据是否可用,辅助函数调用
uint8_t Judge_Self_ID;//当前机器人的ID
uint16_t Judge_SelfClient_ID;//发送者机器人对应的客户端ID

/**************裁判系统数据辅助****************/
uint16_t ShootNum;//统计发弹量,0x0003触发一次则认为发射了一颗
bool Hurt_Data_Update = FALSE;//装甲板伤害数据是否更新,每受一次伤害置TRUE,然后立即置FALSE,给底盘闪避用


/**
  * @brief  读取裁判数据,loop中循环调用此函数来读取数据
  * @param  缓存数据
  * @retval 是否对正误判断做处理
  * @attention  在此判断帧头和CRC校验,无误再写入数据
  */
bool Judege_read_data(u8 *ReadFromUsart )
{
	bool retval_tf = FALSE;//数据正确与否标志,每次调用读取裁判系统数据函数都先默认为错误
	
	uint16_t judge_length;//统计一帧数据长度 
	
	int CmdID = 0;//数据命令码解析	
	
	if(ReadFromUsart == NULL)
	{
		return -1;
	}
	
	memcpy(&REF.FrameHeader,ReadFromUsart,LEN_HEADER);   //储存帧头数据
	
	if(ReadFromUsart[SOF] == JUDGE_FRAME_HEADER)                   //判断帧头是否为0xa5
	{
		if(Verify_CRC8_Check_Sum( ReadFromUsart, LEN_HEADER ) == TRUE)  //帧头CRC校验
		{
			judge_length = ReadFromUsart[DATA_LENGTH] + LEN_HEADER + LEN_CMDID + LEN_TAIL;	//统计一帧数据长度,用于CR16校验
			
			if(Verify_CRC16_Check_Sum(ReadFromUsart,judge_length) == TRUE)//帧尾CRC16校验
			{
				retval_tf = TRUE;//数据可用
				
				CmdID = (ReadFromUsart[6] << 8 | ReadFromUsart[5]);//解析数据命令码,将数据拷贝到相应结构体中(注意拷贝数据的长度)
				
				switch(CmdID)
				{
					case ID_game_state:     //0x0001
							 memcpy(&REF.GameState, (ReadFromUsart + DATA), LEN_game_state);
							 break;
					
					case ID_game_result:    //0x0002
							 memcpy(&REF.GameResult, (ReadFromUsart + DATA), LEN_game_result);
							 break;
					
					case ID_game_robot_survivors:    //0x0003
							 memcpy(&REF.GameRobotHP, (ReadFromUsart + DATA), LEN_game_robot_survivors);
							 break;
					
					case ID_game_missile_state:    //0x0004
							 memcpy(&REF.GameRobotmissile, (ReadFromUsart + DATA), LED_game_missile_state);
							 break;
					
					case ID_game_buff:    //0x0005
							 memcpy(&REF.Game_ICRA_buff, (ReadFromUsart + DATA), LED_game_buff);
							 break;
					
					case ID_event_data:    //0x0101
							 memcpy(&REF.EventData, (ReadFromUsart + DATA), LEN_event_data);
							 break;
					
					case ID_supply_projectile_action:    //0x0102
							 memcpy(&REF.SupplyProjectileAction, (ReadFromUsart + DATA), LEN_supply_projectile_action);
							 break;
					
					case ID_supply_warm:    //0x0104
							 memcpy(&REF.RefereeWarning, (ReadFromUsart + DATA), LEN_supply_warm);
							 break;
					
					case ID_missile_shoot_time:    //0x0105
							 memcpy(&REF.dart_remaining_time, (ReadFromUsart + DATA), LEN_missile_shoot_time);
							 break;
					
					case ID_game_robot_state:    //0x0201
               Determine_ID();
/*1*/          Referee_Update(GameRobotStat_ID);
							 memcpy(&REF.GameRobotStat, (ReadFromUsart + DATA), LEN_game_robot_state);
							 break;
				
					case ID_power_heat_data:    //0x0202
/*2*/          Referee_Update(PowerHeatData_ID);
							 memcpy(&REF.PowerHeatData, (ReadFromUsart + DATA), LEN_power_heat_data);
							 break;
					
					case ID_game_robot_pos:    //0x0203
							 memcpy(&REF.GameRobotPos, (ReadFromUsart + DATA), LEN_game_robot_pos);
							 break;
					
					case ID_buff_musk:    //0x0204
							 memcpy(&REF.Buff, (ReadFromUsart + DATA), LEN_buff_musk);
							 break;
					
					case ID_aerial_robot_energy:    //0x0205
							 memcpy(&REF.AerialRobotEnergy, (ReadFromUsart + DATA), LEN_aerial_robot_energy);
							 break;
					
					case ID_robot_hurt:      			//0x0206
							memcpy(&REF.RobotHurt, (ReadFromUsart + DATA), LEN_robot_hurt);
							if(REF.RobotHurt.hurt_type == 0)//非装甲板离线造成伤害
							{	
								Hurt_Data_Update = TRUE;
							}//装甲数据每更新一次则判定为受到一次伤害
							break;
					case ID_shoot_data:      			//0x0207
/*3*/          Referee_Update(ShootSpeed_ID);
							 memcpy(&REF.ShootData, (ReadFromUsart + DATA), LEN_shoot_data);
					     break;	
					
					case ID_bullet_remaining:    //0x0208
							 memcpy(&REF.bullet_remaining, (ReadFromUsart + DATA), LEN_bullet_remaining);
							 break;
          
					case ID_rfid_status: 
               memcpy(&REF.rfid_status, (ReadFromUsart+DATA), LEN_rfid_status);
               break;
              
					case ID_dart_client_directive:
               memcpy(&REF.dart_client,(ReadFromUsart+DATA),LEN_dart_client_directive);
               break;
					
				}
					
			}
		}
		//首地址加帧长度,指向CRC16下一字节,用来判断是否为0xA5,用来判断一个数据包是否有多帧数据
		if(*(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + REF.FrameHeader.DataLength + LEN_TAIL) == 0xA5)
		{
			//如果一个数据包出现了多帧数据,则再次读取
			Judege_read_data(ReadFromUsart + sizeof(xFrameHeader) + LEN_CMDID + REF.FrameHeader.DataLength + LEN_TAIL);
		}
	}
	if (retval_tf == TRUE)
	{
		Judge_Data_TF = TRUE;//辅助函数用
	}
	else		//只要CRC16校验不通过就为FALSE
	{
		Judge_Data_TF = FALSE;//辅助函数用
	}	
	return retval_tf;
}

//裁判系统发过来的数据暂存在这里
uint8_t  Judge_Buffer[ JUDGE_BUFFER_LEN ] = {0};

int Usart5_Clean_IDLE_Flag = 0;
DMA_InitTypeDef xCom5DMAInit;
/***************************裁判系统串口初始化***********************************/
void UART5_Init( void )
{
	USART_InitTypeDef  xUsartInit;
	GPIO_InitTypeDef   xGpioInit;
	NVIC_InitTypeDef   xNvicInit;

	RCC_AHB1PeriphClockCmd( RCC_AHB1PERIPH_GPIO_TX | RCC_AHB1PERIPH_GPIO_RX, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART5, ENABLE );
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, ENABLE );
	
	GPIO_PinAFConfig( GPIO_TX, GPIO_PINSOURCE_TX, GPIO_AF_UART5 );
	GPIO_PinAFConfig( GPIO_RX, GPIO_PINSOURCE_RX, GPIO_AF_UART5 ); 

	xGpioInit.GPIO_Pin   = GPIO_PIN_TX;
	xGpioInit.GPIO_Mode  = GPIO_Mode_AF;
	xGpioInit.GPIO_OType = GPIO_OType_PP;
	xGpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	xGpioInit.GPIO_PuPd  = GPIO_PuPd_UP;

	GPIO_Init( GPIO_TX, &xGpioInit );

	xGpioInit.GPIO_Pin = GPIO_PIN_RX;
	GPIO_Init( GPIO_RX, &xGpioInit );

	xUsartInit.USART_BaudRate            = 115200;   
	xUsartInit.USART_WordLength          = USART_WordLength_8b;
	xUsartInit.USART_StopBits            = USART_StopBits_1;
	xUsartInit.USART_Parity              = USART_Parity_No;
	xUsartInit.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
	xUsartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	
	USART_Init( UART5, &xUsartInit );
	USART_Cmd( UART5, ENABLE );
	
	USART_ITConfig( UART5, USART_IT_IDLE, ENABLE  ); //注意要配置成串口空闲中断 

	USART_DMACmd( UART5, USART_DMAReq_Rx, ENABLE );
	USART_DMACmd( UART5, USART_DMAReq_Tx, ENABLE );
	
	UART5_DMA_Init( );//初始化usart5的DMA
	
	xNvicInit.NVIC_IRQChannel                    = UART5_IRQn;
	xNvicInit.NVIC_IRQChannelPreemptionPriority  = 0;
	xNvicInit.NVIC_IRQChannelSubPriority         = 0;
	xNvicInit.NVIC_IRQChannelCmd                 = ENABLE;
	NVIC_Init( &xNvicInit );
}

//DMA初始化
void UART5_DMA_Init( void )
{		
	DMA_DeInit( DMA1_Stream_RX );
	xCom5DMAInit.DMA_Channel = DMA_Channel_4;

	xCom5DMAInit.DMA_DIR = DMA_DIR_PeripheralToMemory;//方向外设到存储器

	xCom5DMAInit.DMA_PeripheralBaseAddr  = (uint32_t)&(UART5->DR);
	xCom5DMAInit.DMA_Memory0BaseAddr     = (uint32_t)Judge_Buffer;
	xCom5DMAInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
	xCom5DMAInit.DMA_BufferSize = 100;
	xCom5DMAInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	xCom5DMAInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
	xCom5DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	xCom5DMAInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	xCom5DMAInit.DMA_Mode = DMA_Mode_Circular;
	xCom5DMAInit.DMA_Priority = DMA_Priority_VeryHigh;
	xCom5DMAInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
	xCom5DMAInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	xCom5DMAInit.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	xCom5DMAInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init( DMA1_Stream_RX, &xCom5DMAInit );	
	DMA_Cmd( DMA1_Stream_RX, ENABLE);  //stream0   GameRobotStat.robot_id  GameRobotStat.
}

extern uint32_t Refer_time ;//裁判系统
void UART5_IRQHandler( void )
{
	if(USART_GetITStatus(UART5,USART_IT_IDLE)!=RESET)//检测到空闲线路
	{		
		//根据ST官方手册,读一下SR和DR寄存器,IDLE才能再次使用,否则会一直进入中断,就会跟串口接收中断没区别
		Usart5_Clean_IDLE_Flag = UART5->SR ;
		Usart5_Clean_IDLE_Flag = UART5->DR ;
		
		DMA_Cmd(DMA1_Stream0,DISABLE);
		
		Usart5_Clean_IDLE_Flag = JUDGE_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream0);

		Judege_read_data(Judge_Buffer);		//读取裁判系统数据
		
		memset(Judge_Buffer, 0, 200);
		
		DMA_ClearFlag(DMA1_Stream0, DMA_IT_TCIF0);
		DMA1_Stream0->NDTR = 200;	
		DMA_Cmd(DMA1_Stream0,ENABLE);
    
    Refer_time = micros() + 50000;
	}
}




/**
  * @brief  串口一次发送一个字节数据
  * @param  自己打包好的要发给裁判的数据
  * @retval void
  * @attention  串口移位发送
  */
void UART5_SendChar(uint8_t cData)
{
	while (USART_GetFlagStatus( UART5, USART_FLAG_TC ) == RESET);
	
	USART_SendData( UART5, cData );   
}

/**
 * @brief 判断裁判系统是否连接
 * @param 
 */
bool Judge_IF_Refer_Normal(void)
{
  bool res = true;
  if(micros() >= Refer_time)
  {
    res = false;
  }
  return res;
}

void Judge_IF_REF_ONL(void)
{
  REF.IF_REF_ONL = Judge_IF_Refer_Normal();
}


//------------PowerHeatData------------//
float REF_Report_CHAS_Power(void)   //4
{
  return REF.PowerHeatData.chassis_power;
}

uint16_t REF_Report_CHAS_PowerBuffer(void)   //2
{
  return REF.PowerHeatData.chassis_power_buffer;
}  

uint16_t REF_Report_Shooter_Heat(void)    //2
{
  return REF.PowerHeatData.shooter_heat0;
}
uint16_t REF_Report_Shooter_Heat_01(void)
{
  return REF.PowerHeatData.shooter_heat1;
}
//------------ShootInfo----------------//
float REF_Report_RealShootSpeed(void)
{
  return REF.ShootData.bullet_speed;
}
uint8_t REF_Report_IF_shooter_output(void)
{
  return REF.GameRobotStat.mains_power_shooter_output;
}
uint8_t REF_Report_IF_chassis_output(void)
{
  return REF.GameRobotStat.mains_power_chassis_output;
}

//------------GameRobotStat------------//
uint8_t REF_Report_robot_ID(void)   //1
{
  return REF.GameRobotStat.robot_id;
}

uint8_t REF_Report_Shoot_SpeedLimit(void)  //1
{
  return REF.GameRobotStat.shooter1_17mm_speed_limit;
}

uint16_t REF_Report_Shoot_CoolingLimit(void)  //2
{
  return REF.GameRobotStat.shooter1_17mm_cooling_limit;
}

uint16_t REF_Report_Shoot_CoolingRate(void)  //2
{
  return REF.GameRobotStat.shooter1_17mm_cooling_rate;
}

uint16_t REF_Report_CHAS_MaxPower(void)  //2
{
  return REF.GameRobotStat.max_chassis_power;
}
//------------------RFID-------------------------//
uint8_t REF_Report_RFID_State(void)
{
  uint8_t res = 0;
  res = res | REF.rfid_status.rfid_status;
  return res;
//  return (uint8_t)REF.rfid_status.rfid_status;
}


//------------------------------------------------------------------------


void Determine_ID(void)//判断自己是哪个队伍
{
	if(REF.GameRobotStat.robot_id < 10)//本机器人的ID，红方
	{ 
		REF.ids.teammate_hero 		 	= 1;
		REF.ids.teammate_engineer  = 2;
		REF.ids.teammate_infantry3 = 3;
		REF.ids.teammate_infantry4 = 4;
		REF.ids.teammate_infantry5 = 5;
		REF.ids.teammate_plane		 	= 6;
		REF.ids.teammate_sentry		= 7;
		
		REF.ids.client_hero 		 	= 0x0101;
		REF.ids.client_engineer  = 0x0102;
		REF.ids.client_infantry3 = 0x0103;
		REF.ids.client_infantry4 = 0x0104;
		REF.ids.client_infantry5 = 0x0105;
		REF.ids.client_plane			= 0x0106;
		
		if     (REF.GameRobotStat.robot_id == hero_red)//不断刷新放置在比赛中更改颜色
			REF.self_client = REF.ids.client_hero;
		else if(REF.GameRobotStat.robot_id == engineer_red)
			REF.self_client = REF.ids.client_engineer;
		else if(REF.GameRobotStat.robot_id == infantry3_red)
			REF.self_client = REF.ids.client_infantry3;
		else if(REF.GameRobotStat.robot_id == infantry4_red)
			REF.self_client = REF.ids.client_infantry4;
		else if(REF.GameRobotStat.robot_id == infantry5_red)
			REF.self_client = REF.ids.client_infantry5;
		else if(REF.GameRobotStat.robot_id == plane_red)
			REF.self_client = REF.ids.client_plane;
	}
	else //蓝方
	{
		REF.ids.teammate_hero 		 	= 101;
		REF.ids.teammate_engineer  = 102;
		REF.ids.teammate_infantry3 = 103;
		REF.ids.teammate_infantry4 = 104;
		REF.ids.teammate_infantry5 = 105;
		REF.ids.teammate_plane		 	= 106;
		REF.ids.teammate_sentry		= 107;
		
		REF.ids.client_hero 		 	= 0x0165;
		REF.ids.client_engineer  = 0x0166;
		REF.ids.client_infantry3 = 0x0167;
		REF.ids.client_infantry4 = 0x0168;
		REF.ids.client_infantry5 = 0x0169;
		REF.ids.client_plane			= 0x016A;
		
		if     (REF.GameRobotStat.robot_id == hero_blue)
			REF.self_client = REF.ids.client_hero;
		else if(REF.GameRobotStat.robot_id == engineer_blue)
			REF.self_client = REF.ids.client_engineer;
		else if(REF.GameRobotStat.robot_id == infantry3_blue)
			REF.self_client = REF.ids.client_infantry3;
		else if(REF.GameRobotStat.robot_id == infantry4_blue)
			REF.self_client = REF.ids.client_infantry4;
		else if(REF.GameRobotStat.robot_id == infantry5_blue)
			REF.self_client = REF.ids.client_infantry5;
		else if(REF.GameRobotStat.robot_id == plane_blue)
			REF.self_client = REF.ids.client_plane;
		
	}

}
/*********************************************/
uint8_t CliendTxBuffer[200];
uint8_t TeammateTxBuffer[200];
char first_line[30]  = {"          CLIP:"};//是否可以射击,最多放30个字符串，bool
char second_line[30] = {"          SPIN:"};//小陀螺
char third_line[30]  = {"      auto_aim:"};//自瞄
char fourth_line[30] = {"shootheatlimit:"};//热量限制
char fifth_line[30]  = {":block"};//发弹量int
char sixth_line[30]  = {":bullet_speed"};//射速int
char seventh_line[30]= {"supercapacitor:"};//超级电容剩余量,float
char empty_line[30] = {"                              "};
//*******************************绘字符串******************************/
ext_charstring_data_t tx_client_char;
uint8_t state_first_graphic;//0~7循环
void Char_Graphic(ext_client_string_t* graphic,//最终要发出去的数组中的数据段内容
									const char* name,
									uint32_t operate_tpye,
									
									uint32_t layer,
									uint32_t color,
									uint32_t size,
									uint32_t length,
									uint32_t width,
									uint32_t start_x,
									uint32_t start_y,

									
									
									const char *character)//外部放入的数组
{
	graphic_data_struct_t *data_struct = &graphic->grapic_data_struct;
	for(char i=0;i<3;i++)
		data_struct->graphic_name[i] = name[i];	//字符索引
	data_struct->operate_tpye = operate_tpye; //图层操作
	data_struct->graphic_tpye = CHAR;         //Char型
	data_struct->layer = layer;//都在第零层
	data_struct->color = color;//都是白色
	data_struct->start_angle = size;
	data_struct->end_angle = length;	
	data_struct->width = width;
	data_struct->start_x = start_x;
	data_struct->start_y = start_y;	
	
	data_struct->radius = 0;
	data_struct->end_x = 0;
	data_struct->end_y = 0;
	memcpy(graphic->data,empty_line,19);
  memcpy(graphic->data,character,length);
}
static void Draw_char()
{
	if(state_first_graphic == 0)//不知道什么时候进入客户端所以要不断更新
	{
		Char_Graphic(&tx_client_char.clientData,"CL1",ADD,0,RED_BLUE,20,strlen(first_line),2,(50),(1080*9/12),first_line);//x1920/18
		state_first_graphic = 1;
	}
	else if(state_first_graphic == 1)
	{
		Char_Graphic(&tx_client_char.clientData,"CL2",ADD,0,RED_BLUE,20,strlen(second_line),2,(50),(1080*8/12),second_line);
		state_first_graphic = 2;
	}
	else if(state_first_graphic == 2)
	{
		Char_Graphic(&tx_client_char.clientData,"CL3",ADD,0,RED_BLUE,20,strlen(third_line),2,(50),(1080*7/12),third_line);
		state_first_graphic = 3;
	}
	else if(state_first_graphic == 3)
	{
		Char_Graphic(&tx_client_char.clientData,"CL4",ADD,0,RED_BLUE,20,strlen(fourth_line),2,(50),(1080*6/12),fourth_line);
		state_first_graphic = 4;
	}
	else if(state_first_graphic == 4)
	{
		Char_Graphic(&tx_client_char.clientData,"CL5",ADD,0,RED_BLUE,20,strlen(fifth_line),2,(1920-320),(1080*9/12),fifth_line);
		state_first_graphic = 5;
	}
	else if(state_first_graphic == 5)
	{
		Char_Graphic(&tx_client_char.clientData,"CL6",DELETE,0,WHITE,10,strlen(sixth_line),1,(1920-200),(1080*8/12),sixth_line);
		state_first_graphic = 6;
	}
	else if(state_first_graphic == 6)
	{
		Char_Graphic(&tx_client_char.clientData,"CL7",ADD,0,RED_BLUE,20,strlen(seventh_line),2,(1920-200-200),(1080*7/12),seventh_line);
		state_first_graphic = 7;
	}
}
void Client_graphic_Init()
{
	if(state_first_graphic>=7)
	{
		state_first_graphic = 0;
	}
		//帧头
		tx_client_char.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_client_char.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_string_t);
		tx_client_char.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_client_char.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验
	
		//命令码
		tx_client_char.CmdID = ID_robot_interactive_header_data;
		
		//数据段头结构
		tx_client_char.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_char_graphic;
		tx_client_char.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_client_char.dataFrameHeader.receiver_ID = REF.self_client;
		
		//数据段
		Draw_char();
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_client_char.CmdID, LEN_CMD_ID+tx_client_char.txFrameHeader.DataLength);//加上命令码长度2
		
		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_client_char));
		
    for(int i = 0; i < sizeof(tx_client_char); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//************************************绘制象形*******************************/
ext_graphic_seven_data_t tx_client_graphic_figure;
void Figure_Graphic(graphic_data_struct_t* graphic,//最终要发出去的数组的数据段内容
									const char* name,
									uint32_t operate_tpye,
									uint32_t graphic_tpye,//绘制什么图像
									uint32_t layer,
									uint32_t color,
									uint32_t start_angle,
									uint32_t end_angle,
									uint32_t width,
									uint32_t start_x,
									uint32_t start_y,
									uint32_t radius,
									uint32_t end_x,
									uint32_t end_y)							
{
	for(char i=0;i<3;i++)
		graphic->graphic_name[i] = name[i];	//字符索引
	graphic->operate_tpye = operate_tpye; //图层操作
	graphic->graphic_tpye = graphic_tpye;         //Char型
	graphic->layer        = layer;//都在第一层
	graphic->color        = color;//变色
	graphic->start_angle  = start_angle;
	graphic->end_angle    = end_angle;	
	graphic->width        = width;
	graphic->start_x      = start_x;
	graphic->start_y      = start_y;	
	graphic->radius = radius;
	graphic->end_x  = end_x;
	graphic->end_y  = end_y;
}
int update_figure_flag;
static void clip_figure(bool clip)//舵机关上为false，
{
	if( clip == true)//可准备射击为绿色
		Figure_Graphic(&tx_client_graphic_figure.clientData[0],"GL1",update_figure_flag,CIRCLE,1,GREEN,0,0,5,  200+160,1080*9/12, 20,0,0);
	else if( clip == false)//舵机打开，摩擦轮没有开启，紫红色装弹
		Figure_Graphic(&tx_client_graphic_figure.clientData[0],"GL1",update_figure_flag,CIRCLE,1,FUCHSIA,0,0,5,  200+160,1080*9/12, 20,0,0);
}

static void spin_second_figure(bool spin)//小陀螺打开为true
{
	if(spin == true)//打开小陀螺为绿色
		Figure_Graphic(&tx_client_graphic_figure.clientData[1],"GL2",update_figure_flag,CIRCLE,1,GREEN,0,0,5,  200+160,1080*8/12, 20,0,0);
	else if(spin == false)//没开小陀螺为紫红色
		Figure_Graphic(&tx_client_graphic_figure.clientData[1],"GL2",update_figure_flag,CIRCLE,1,FUCHSIA,0,0,5,  200+160,1080*8/12, 20,0,0);
}
static void auto_aim_third_figure(bool auto_aim)//自瞄打开为true
{
	if(auto_aim == true)//打开自瞄为绿色
		Figure_Graphic(&tx_client_graphic_figure.clientData[2],"GL3",update_figure_flag,CIRCLE,1,GREEN,0,0,5,  200+160,1080*7/12, 20,0,0);
	else if(auto_aim == false)//没开自瞄为紫红色
		Figure_Graphic(&tx_client_graphic_figure.clientData[2],"GL3",update_figure_flag,CIRCLE,1,FUCHSIA,0,0,5,  200+160,1080*7/12, 20,0,0);
}
static void shootheatlimit_fourth_figure(bool limit)//热量限制
{
	if(limit == true)
		Figure_Graphic(&tx_client_graphic_figure.clientData[3],"GL4",update_figure_flag,CIRCLE,1,GREEN,0,0,5,  200+160,1080*6/12, 20,0,0);
	else if(limit == false)
		Figure_Graphic(&tx_client_graphic_figure.clientData[3],"GL4",update_figure_flag,CIRCLE,1,FUCHSIA,0,0,5,  200+160,1080*6/12, 20,0,0);	
}



//
//static void sight_bead_fifsix_figrue(uint32_t x,uint32_t y)//可移动准心，请强制转换成uint32_t1920*1080有部分地区无法画出
//{
//	Figure_Graphic(&tx_client_graphic_figure.clientData[4],"GL5",DELETE,LINE,1,FUCHSIA,0,0,3,  x-20,y+20  ,0,  x+20,y-20);
//	Figure_Graphic(&tx_client_graphic_figure.clientData[5],"GL6",DELETE,LINE,1,FUCHSIA,0,0,3,  x-20,y-20  ,0,  x+20,y+20);
//}

//static void supercapacitor_seventh_figure(float remain_energy,uint32_t turning_point)//剩余超级电容（单位百分比），低于某百分比变红色
//{
//	uint32_t remaining = (uint32_t)remain_energy;//强制转换
//	if(remaining >= turning_point)//直线长度为3
//		Figure_Graphic(&tx_client_graphic_figure.clientData[6],"GL7",update_figure_flag,LINE,1,GREEN,0,0,10,(1920-350),585  ,0,  (1920-350)+remaining*3,585);
//	else if(remaining < turning_point)
//		Figure_Graphic(&tx_client_graphic_figure.clientData[6],"GL7",update_figure_flag,LINE,1,FUCHSIA,0,0,10,(1920-350),585  ,0,  (1920-350)+remaining*3,585);		
//}

static void BlockMuch_fifth_figure(bool Block)//反陀螺
{
	if(Block == true)
		Figure_Graphic(&tx_client_graphic_figure.clientData[4],"GL5",update_figure_flag,CIRCLE,1,GREEN,0,0,5,  (1920-350),1080*9/12, 20,0,0);
	else if(Block == false)
		Figure_Graphic(&tx_client_graphic_figure.clientData[4],"GL5",update_figure_flag,CIRCLE,1,FUCHSIA,0,0,5,  (1920-350),1080*9/12, 20,0,0);
}

static void Bullet_Warning(bool Warning)
{
	if(Warning == true)
		Figure_Graphic(&tx_client_graphic_figure.clientData[5],"GL6",ADD,RECTANGLE,1,GREEN,0,0,5,       (1920-580),610, 0,1920-450,540-70);
	else if(Warning == false)
		Figure_Graphic(&tx_client_graphic_figure.clientData[5],"GL6",DELETE,RECTANGLE,1,FUCHSIA,0,0,5,  (1920-580),610, 0,1920-450,540-70);
}
static void EMPTY(bool a)
{
  
  Figure_Graphic(&tx_client_graphic_figure.clientData[6],"GL7",DELETE,FUCHSIA,1,GREEN,0,0,5,  (1920-350),1080*9/12, 20,0,0);
}
//!!!!!!!!!!!!!!!!!!!全局变量！！！！！
bool global_fiction,global_clip,global_spin,global_auto_aim,global_shootlimit,global_block,global_bulletWarning;
uint32_t global_sight_bead_x = 960,global_sight_bead_y = 720,global_supercapacitor_point;//[0,100]
float global_supercapacitor_remain = 77.3,//[0,100]
	    global_gimble_pitch,global_gimble_yaw;
int   global_bullet_speed,global_bullet_sum;
static void Draw_Figure_bool()
{
	clip_figure(global_clip);
	spin_second_figure    (global_spin);
	auto_aim_third_figure (global_auto_aim);
	shootheatlimit_fourth_figure   (global_shootlimit);
	BlockMuch_fifth_figure (global_block);
  Bullet_Warning(global_bulletWarning);
  EMPTY(0);
}
//删除图层信息
ext_deleteLayer_data_t tx_client_delete;
void Client_graphic_delete_update(uint8_t delete_layer)//删除图层信息
{
		//帧头
		tx_client_delete.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_client_delete.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(ext_client_custom_graphic_delete_t);
		tx_client_delete.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_client_delete.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_client_delete.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_client_delete.dataFrameHeader.data_cmd_id = INTERACT_ID_delete_graphic;
		tx_client_delete.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_client_delete.dataFrameHeader.receiver_ID = REF.self_client;
		
		//数据段
		tx_client_delete.clientData.operate_type = ALL_delete;
		tx_client_delete.clientData.layer = delete_layer;
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_client_delete.CmdID, LEN_CMD_ID+tx_client_delete.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_client_delete));
		
    for(int i = 0; i < sizeof(tx_client_delete); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
void Client_graphic_Info_update()//七个图像一起更新
{
		//帧头
		tx_client_graphic_figure.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_client_graphic_figure.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*7;
		tx_client_graphic_figure.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_client_graphic_figure.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_client_graphic_figure.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_client_graphic_figure.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
		tx_client_graphic_figure.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_client_graphic_figure.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		Draw_Figure_bool();
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_client_graphic_figure.CmdID, LEN_CMD_ID+tx_client_graphic_figure.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_client_graphic_figure));
		
    for(int i = 0; i < sizeof(tx_client_graphic_figure); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }

}
//*************************绘制变化图像*****************************************/

//void Client_Data_Info_update()
//{
//		
//}
ext_graphic_two_data_t tx_aim_figure;//第三层放准心
int update_aim_flag;//1-add,3删除
static void sight_bead_figrue(uint32_t x,uint32_t y)//可移动准心，请强制转换成uint32_t 1920*1080有部分地区无法画出
{
	Figure_Graphic(&tx_aim_figure.clientData[0],"GR1",update_aim_flag,LINE,2,FUCHSIA,0,0,3,  x-20,y+20  ,0,  x+20,y-20);//graphic_Remove
	Figure_Graphic(&tx_aim_figure.clientData[1],"GR2",update_aim_flag,LINE,2,FUCHSIA,0,0,3,  x-20,y-20  ,0,  x+20,y+20);
}
void Client_aim_update()//两个个图像一起更新
{
		//帧头
		tx_aim_figure.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_aim_figure.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*2;
		tx_aim_figure.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_aim_figure.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_aim_figure.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_aim_figure.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_two_graphic;
		tx_aim_figure.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_aim_figure.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		sight_bead_figrue(global_sight_bead_x,global_sight_bead_y);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_aim_figure.CmdID, LEN_CMD_ID+tx_aim_figure.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_aim_figure));
		
    for(int i = 0; i < sizeof(tx_aim_figure); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//剩余电容只有一个图层
ext_graphic_one_data_t tx_supercapacitor_figure;
int update_supercapacitor_flag;
static void supercapacitor_figure(float remain_energy,uint32_t turning_point)//剩余超级电容（单位百分比），低于某百分比变红色
{
	uint32_t remaining = (uint32_t)remain_energy;//强制转换
	if(remaining >= turning_point)//直线长度为3
		Figure_Graphic(&tx_supercapacitor_figure.clientData,"SR1",update_supercapacitor_flag,LINE,2,GREEN,0,0,10,(1920-350),585  ,0,  (1920-350)+remaining*3,585);
	else if(remaining < turning_point)
		Figure_Graphic(&tx_supercapacitor_figure.clientData,"SR1",update_supercapacitor_flag,LINE,2,FUCHSIA,0,0,10,(1920-350),585  ,0,  (1920-350)+remaining*3,585);		
}
void Client_supercapacitor_update()//一个图像更新
{
		//帧头
		tx_supercapacitor_figure.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_supercapacitor_figure.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t);
		tx_supercapacitor_figure.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_supercapacitor_figure.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_supercapacitor_figure.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_supercapacitor_figure.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_one_graphic;
		tx_supercapacitor_figure.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_supercapacitor_figure.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		supercapacitor_figure(global_supercapacitor_remain,global_supercapacitor_point);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_supercapacitor_figure.CmdID, LEN_CMD_ID+tx_supercapacitor_figure.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_supercapacitor_figure));
		
    for(int i = 0; i < sizeof(tx_supercapacitor_figure); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//******************绘制浮点数*************************/
//第五层图层
ext_float_two_data_t tx_gimbal_angle;
int update_float_flag;
void Float_Graphic(Float_data_struct_t* graphic,//最终要发出去的数组的数据段内容
									const char* name,
									uint32_t operate_tpye,
									uint32_t graphic_tpye,//绘制什么图像
									uint32_t layer,
									uint32_t color,
									uint32_t size,
									uint32_t decimal,
									uint32_t width,
									uint32_t start_x,
									uint32_t start_y,
									float number)							
{
	for(char i=0;i<3;i++)
		graphic->graphic_name[i] = name[i];	//字符索引
	graphic->operate_tpye = operate_tpye; //图层操作
	graphic->graphic_tpye = graphic_tpye;  
	graphic->layer        = layer;//
	graphic->color        = color;//变色
	graphic->start_angle  = size;
	graphic->end_angle    = decimal;//小数有效位	
	graphic->width        = width;
	graphic->start_x      = start_x;
	graphic->start_y      = start_y;	
	graphic->number       = number;
}
static void gimbal_angle_float(float gimble_pitch,float gimble_yaw)//当前云台角度
{
	//青色pitch第一行，黄色yaw第二行
		Float_Graphic(&tx_gimbal_angle.clientData[0],"FR1",update_float_flag,FLOAT,4,CYAN_BLUE,30,2,3,(1920*4/6),810  ,(float)gimble_pitch);
		Float_Graphic(&tx_gimbal_angle.clientData[1],"FR2",update_float_flag,FLOAT,4,YELLOW,   30,2,3,(1920*4/6),760  ,(float)gimble_yaw);		
}
void Client_gimbal_angle_update()//两个图像更新
{
		//帧头
		tx_gimbal_angle.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_gimbal_angle.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*2;
		tx_gimbal_angle.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_gimbal_angle.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_gimbal_angle.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_gimbal_angle.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_two_graphic;
		tx_gimbal_angle.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_gimbal_angle.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		gimbal_angle_float(global_gimble_pitch,global_gimble_yaw);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_gimbal_angle.CmdID, LEN_CMD_ID+tx_gimbal_angle.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_gimbal_angle));
		
    for(int i = 0; i < sizeof(tx_gimbal_angle); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//**********************绘制int类型***************************/
ext_int_two_data_t tx_bullet_int;
int update_int_flag;
void Int_Graphic(Int_data_struct_t* graphic,//最终要发出去的数组的数据段内容
									const char* name,
									uint32_t operate_tpye,
									uint32_t graphic_tpye,//绘制什么图像
									uint32_t layer,
									uint32_t color,
									uint32_t size,
									uint32_t zero,
									uint32_t width,
									uint32_t start_x,
									uint32_t start_y,
									int number)							
{
	for(char i=0;i<3;i++)
		graphic->graphic_name[i] = name[i];	//字符索引
	graphic->operate_tpye = operate_tpye; //图层操作
	graphic->graphic_tpye = graphic_tpye;        
	graphic->layer        = layer;//都在第一层
	graphic->color        = color;//变色
	graphic->start_angle  = size;
	graphic->end_angle    = zero;	
	graphic->width        = width;
	graphic->start_x      = start_x;
	graphic->start_y      = start_y;	
	graphic->number       = number;
}
static void bullet_int(int bullet_speed,int bullet_sum)//子弹射速和发弹量
{
	//总数量第一行，射速第二行
		Int_Graphic(&tx_bullet_int.clientData[0],"IR1",update_int_flag,INT,5,WHITE,30,0,3,(1920-280),(820)  ,(int)bullet_sum);
		Int_Graphic(&tx_bullet_int.clientData[1],"IR2",update_int_flag,INT,5,WHITE,30,0,3,(1920-280),(730)  ,(int)bullet_speed);		
}
void Client_bullet_int_update()//两个图像更新
{
		//帧头
		tx_bullet_int.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		tx_bullet_int.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*2;
		tx_bullet_int.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&tx_bullet_int.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		tx_bullet_int.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		tx_bullet_int.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_two_graphic;
		tx_bullet_int.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		tx_bullet_int.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		bullet_int(global_bullet_speed,global_bullet_sum);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&tx_bullet_int.CmdID, LEN_CMD_ID+tx_bullet_int.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(tx_bullet_int));
		
    for(int i = 0; i < sizeof(tx_bullet_int); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}

//*****************************英雄需求*************************************/
//*****************************英雄需求*************************************/
ext_graphic_five_data_t aim_line_graphic;
uint32_t global_vertical_1,global_vertical_2,global_horizontal_1,global_horizontal_2,global_horizontal_3;
//绘图
static void Draw_Figure_aimline(uint32_t vertical_1,uint32_t vertical_2,//竖线范围在0~1920
	                              uint32_t horizontal_1,uint32_t horizontal_2,uint32_t horizontal_3)//水平线范围在0~1080
{
	//竖线
		Figure_Graphic(&aim_line_graphic.clientData[0],"LL1",ADD,LINE,6,WHITE,0,0,3,  vertical_1,1080, 0,vertical_1,0);
		Figure_Graphic(&aim_line_graphic.clientData[1],"LL2",ADD,LINE,6,WHITE,0,0,3,  vertical_2,1080, 0,vertical_2,0);
	//水平线
		Figure_Graphic(&aim_line_graphic.clientData[2],"LL3",ADD,LINE,6,WHITE,0,0,3,  0,horizontal_1, 0,1920,horizontal_1);
		Figure_Graphic(&aim_line_graphic.clientData[3],"LL4",ADD,LINE,6,WHITE,0,0,3,  0,horizontal_2, 0,1920,horizontal_2);
		Figure_Graphic(&aim_line_graphic.clientData[4],"LL5",ADD,LINE,6,WHITE,0,0,3,  0,horizontal_3, 0,1920,horizontal_3);
		

}
void Client_aim_line()//五个图像不更新
{
		//帧头
		aim_line_graphic.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		aim_line_graphic.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*5;
		aim_line_graphic.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&aim_line_graphic.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		aim_line_graphic.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		aim_line_graphic.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_five_graphic;
		aim_line_graphic.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		aim_line_graphic.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
    Draw_Figure_aimline(global_vertical_1,global_vertical_2,global_horizontal_1,global_horizontal_2,global_horizontal_3);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&aim_line_graphic.CmdID, LEN_CMD_ID+aim_line_graphic.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(aim_line_graphic));
		
    for(int i = 0; i < sizeof(tx_bullet_int); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}

////三号准心图层
//void Figure_Graphic(graphic_data_struct_t* graphic,//最终要发出去的数组的数据段内容
//									const char* name,
//									uint32_t operate_tpye,
//									uint32_t graphic_tpye,//绘制什么图像
//									uint32_t layer,
//									uint32_t color,
//									uint32_t start_angle,
//									uint32_t end_angle,
//									uint32_t width,
//									uint32_t start_x,
//									uint32_t start_y,
//									uint32_t radius,
//									uint32_t end_x,
//									uint32_t end_y)		
ext_graphic_seven_data_t high_aim_figure;//操作手准心之上,不更新
ext_graphic_seven_data_t low_aim_shortfigure_1;//准心下的第一个短线
ext_graphic_seven_data_t low_aim_shortfigure_2;
ext_graphic_seven_data_t  low_aim_shortfigure_3;//五个短线两个纵线
ext_graphic_five_data_t  low_aim_longfigure;//准心下的长横线
//!!!!!!!!!!!!!!!!!!全局变量
uint32_t division_value = 10;
//division_value分度值,line_length短线长度的一半
static void aim_1(uint32_t division_value,uint32_t line_length)//准心上半部分的宽度"AH"--aim_high
{
	Figure_Graphic(&high_aim_figure.clientData[0],"AH1",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30                 ,0,  960+line_length,540+30);//graphic_Remove
	Figure_Graphic(&high_aim_figure.clientData[1],"AH2",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value  ,0,  960+line_length,540+30+division_value  );
	Figure_Graphic(&high_aim_figure.clientData[2],"AH3",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value*2,0,  960+line_length,540+30+division_value*2);
	Figure_Graphic(&high_aim_figure.clientData[3],"AH4",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value*3,0,  960+line_length,540+30+division_value*3);
	Figure_Graphic(&high_aim_figure.clientData[4],"AH5",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value*4,0,  960+line_length,540+30+division_value*4);
	Figure_Graphic(&high_aim_figure.clientData[5],"AH6",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value*5,0,  960+line_length,540+30+division_value*5);
	Figure_Graphic(&high_aim_figure.clientData[6],"AH7",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540+30+division_value*6,0,  960+line_length,540+30+division_value*6);
}
void _high_aim_()
{
		//帧头
		high_aim_figure.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		high_aim_figure.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*7;
		high_aim_figure.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&high_aim_figure.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		high_aim_figure.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		high_aim_figure.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
		high_aim_figure.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		high_aim_figure.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		aim_1(division_value,10);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&high_aim_figure.CmdID, LEN_CMD_ID+high_aim_figure.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(high_aim_figure));
		
    for(int i = 0; i < sizeof(high_aim_figure); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}

static void aim_lowshort_2(uint32_t division_value,uint32_t line_length)//准心上半部分的宽度"AL"--aim_low
{
	Figure_Graphic(&low_aim_shortfigure_1.clientData[0],"AL1",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30                 ,0,  960+line_length,540-30);//graphic_Remove
	Figure_Graphic(&low_aim_shortfigure_1.clientData[1],"AL2",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value  ,0,  960+line_length,540-30-division_value  );
	Figure_Graphic(&low_aim_shortfigure_1.clientData[2],"AL3",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*2,0,  960+line_length,540-30-division_value*2);
	Figure_Graphic(&low_aim_shortfigure_1.clientData[3],"AL4",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*4,0,  960+line_length,540-30-division_value*4);
	Figure_Graphic(&low_aim_shortfigure_1.clientData[4],"AL5",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*5,0,  960+line_length,540-30-division_value*5);
	Figure_Graphic(&low_aim_shortfigure_1.clientData[5],"AL6",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*6,0,  960+line_length,540-30-division_value*6);
	Figure_Graphic(&low_aim_shortfigure_1.clientData[6],"AL7",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*8,0,  960+line_length,540-30-division_value*8);
}
void _lowshort_aim_2()
{
		//帧头
		low_aim_shortfigure_1.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		low_aim_shortfigure_1.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*7;
		low_aim_shortfigure_1.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&low_aim_shortfigure_1.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		low_aim_shortfigure_1.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		low_aim_shortfigure_1.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
		low_aim_shortfigure_1.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		low_aim_shortfigure_1.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		aim_lowshort_2(division_value,10);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&low_aim_shortfigure_1.CmdID, LEN_CMD_ID+low_aim_shortfigure_1.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(low_aim_shortfigure_1));
		
    for(int i = 0; i < sizeof(low_aim_shortfigure_1); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
static void aim_lowshort_3(uint32_t division_value,uint32_t line_length)//准心上半部分的宽度"AM"--aim_low_middle
{
	Figure_Graphic(&low_aim_shortfigure_2.clientData[0],"AM1",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*9 ,0,  960+line_length,540-30-division_value*9 );//graphic_Remove
	Figure_Graphic(&low_aim_shortfigure_2.clientData[1],"AM2",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*10,0,  960+line_length,540-30-division_value*10);
	Figure_Graphic(&low_aim_shortfigure_2.clientData[2],"AM3",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*12,0,  960+line_length,540-30-division_value*12);
	Figure_Graphic(&low_aim_shortfigure_2.clientData[3],"AM4",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*13,0,  960+line_length,540-30-division_value*13);
	Figure_Graphic(&low_aim_shortfigure_2.clientData[4],"AM5",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*14,0,  960+line_length,540-30-division_value*14);
	Figure_Graphic(&low_aim_shortfigure_2.clientData[5],"AM6",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*16,0,  960+line_length,540-30-division_value*16);
	Figure_Graphic(&low_aim_shortfigure_2.clientData[6],"AM7",ADD,LINE,3,YELLOW,0,0,1,  960-line_length,540-30-division_value*17,0,  960+line_length,540-30-division_value*17);
}
void _lowshort_aim_3()
{
		//帧头
		low_aim_shortfigure_2.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		low_aim_shortfigure_2.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*7;
		low_aim_shortfigure_2.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&low_aim_shortfigure_2.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		low_aim_shortfigure_2.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		low_aim_shortfigure_2.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
		low_aim_shortfigure_2.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		low_aim_shortfigure_2.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		aim_lowshort_3(division_value,10);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&low_aim_shortfigure_2.CmdID, LEN_CMD_ID+low_aim_shortfigure_2.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(low_aim_shortfigure_2));
		
    for(int i = 0; i < sizeof(low_aim_shortfigure_2); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//stem--茎
static void aim_lowshort_stem(uint32_t division_value,uint32_t line_length)//准心上半部分的宽度"AM"--aim_low_bottom,"AS"--aim_stem
{ 
	Figure_Graphic(&low_aim_shortfigure_3.clientData[0],"AB1",ADD,LINE,3,YELLOW,0,0,1,   960-line_length,540-30-division_value*18,0,   960+line_length,540-30-division_value*18);//graphic_Remove
	Figure_Graphic(&low_aim_shortfigure_3.clientData[1],"AB2",ADD,LINE,3,YELLOW,0,0,1,   960-line_length,540-30-division_value*20,0,   960+line_length,540-30-division_value*20);
	Figure_Graphic(&low_aim_shortfigure_3.clientData[2],"AB3",ADD,LINE,3,YELLOW,0,0,1,   960-line_length,540-30-division_value*21,0,   960+line_length,540-30-division_value*21);
	Figure_Graphic(&low_aim_shortfigure_3.clientData[3],"AB4",ADD,LINE,3,YELLOW,0,0,1,   960-line_length,540-30-division_value*22,0,   960+line_length,540-30-division_value*22);
	Figure_Graphic(&low_aim_shortfigure_3.clientData[4],"AB5",ADD,LINE,3,YELLOW,0,0,1,960-line_length-10,540-30-division_value*23,0,960+line_length+10,540-30-division_value*23);
	
	Figure_Graphic(&low_aim_shortfigure_3.clientData[5],"AS1",ADD,LINE,3,YELLOW,0,0,1,   960,            540+30+division_value*6 ,0,   960,            540+30);
	Figure_Graphic(&low_aim_shortfigure_3.clientData[6],"AS2",ADD,LINE,3,YELLOW,0,0,1,   960,            540-30-division_value*23,0,   960,            540-30);

}
void _lowshortstem_aim_4()
{
		//帧头
		low_aim_shortfigure_3.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		low_aim_shortfigure_3.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*7;
		low_aim_shortfigure_3.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&low_aim_shortfigure_3.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		low_aim_shortfigure_3.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		low_aim_shortfigure_3.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
		low_aim_shortfigure_3.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		low_aim_shortfigure_3.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		aim_lowshort_stem(division_value,10);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&low_aim_shortfigure_3.CmdID, LEN_CMD_ID+low_aim_shortfigure_3.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(low_aim_shortfigure_3));
		
    for(int i = 0; i < sizeof(low_aim_shortfigure_3); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
//图层四
static void aim_lowlong(uint32_t division_value,uint32_t line_length)//准心上半部分的宽度"AM"--aim_low_Long,"AS"--aim_stem
{ 
	Figure_Graphic(&low_aim_longfigure.clientData[0],"AL1",ADD,LINE,4,YELLOW,0,0,1,960-line_length-30,540-30-division_value*19,0,960+line_length+30,540-30-division_value*19);//graphic_Remove
	Figure_Graphic(&low_aim_longfigure.clientData[1],"AL2",ADD,LINE,4,YELLOW,0,0,1,960-line_length-40,540-30-division_value*15,0,960+line_length+40,540-30-division_value*15);
	Figure_Graphic(&low_aim_longfigure.clientData[2],"AL3",ADD,LINE,4,YELLOW,0,0,1,960-line_length-50,540-30-division_value*11,0,960+line_length+50,540-30-division_value*11);
	Figure_Graphic(&low_aim_longfigure.clientData[3],"AL4",ADD,LINE,4,YELLOW,0,0,1,960-line_length-60,540-30-division_value*7 ,0,960+line_length+60,540-30-division_value*7 );
	Figure_Graphic(&low_aim_longfigure.clientData[4],"AL5",ADD,LINE,4,YELLOW,0,0,1,960-line_length-70,540-30-division_value*3 ,0,960+line_length+70,540-30-division_value*3 );
	
}
void _lowlong_aim_()
{
		//帧头
		low_aim_longfigure.txFrameHeader.SOF = JUDGE_FRAME_HEADER;
		low_aim_longfigure.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(graphic_data_struct_t)*5;
		low_aim_longfigure.txFrameHeader.Seq = 0;//包序号
		memcpy(CliendTxBuffer,&low_aim_longfigure.txFrameHeader,sizeof(xFrameHeader));
		Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//头校验

		//命令码
		low_aim_longfigure.CmdID = ID_robot_interactive_header_data;

		//数据段头结构
		low_aim_longfigure.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_five_graphic;
		low_aim_longfigure.dataFrameHeader.send_ID     = REF.GameRobotStat.robot_id;
		low_aim_longfigure.dataFrameHeader.receiver_ID = REF.self_client;
	
		//数据段
		aim_lowlong(division_value,10);
		memcpy(CliendTxBuffer+LEN_FRAME_HEAD, (uint8_t*)&low_aim_longfigure.CmdID, LEN_CMD_ID+low_aim_longfigure.txFrameHeader.DataLength);//加上命令码长度2

		//帧尾
		Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(low_aim_longfigure));
		
    for(int i = 0; i < sizeof(low_aim_longfigure); i++) {
      UART5_SendChar(CliendTxBuffer[i]);
    }
}
