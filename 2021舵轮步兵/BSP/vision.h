#ifndef __VISION_H_
#define __VISION_H_

#include "GO.h"


#define VISION_TX_BUFFER_LEN	50
#define VISION_FRAME_HEADER		(0xA5)

#define Trigger_IO_UP  	   GPIO_SetBits(GPIOD,GPIO_Pin_13)
#define Trigger_IO_DOWN	   GPIO_ResetBits(GPIOD,GPIO_Pin_13)
#define Trigger_IO       PDout(13)


/**
 *	@brief	自身颜色
 */
typedef enum
{
  UNKNOWN = 0x00,
  I_AM_BLUE = 0x01,
  I_AM_RED = 0x02,
  BASE = 0x03,
}Color_t;

/**
 *	@brief	视觉读取项目
 */
typedef enum
{
	IF_LOCK_BUFF = 0, // 识别到符
  IF_LOCK_ARMOR = 1,//识别到装甲板
  IF_FRAMEDROP = 2,
  IF_CHANGE_ARMOR = 3,//识别到切换装甲板
  IF_IDENTIFY_SPIN = 4,
  IF_CHANGE_ARMOR_BUFF = 5,
} Vision_Flag;

/**
 *	电控->视觉
 */
typedef enum {
  NO_VISION = 0x00,//松开手时
	CMD_AIM_AUTO		= 0x01,	// 自瞄
	CMD_AIM_SMALL_BUFF	= 0x02,	// 识别小符
	CMD_AIM_BIG_BUFF	= 0x03,	// 识别大符
	CMD_AIM_SENTRY		= 0x04,	// 击打哨兵
	CMD_AIM_BASE		= 0x05	,// 吊射基地
} Vision_Cmd_Id_t;

/* 帧头格式 */
typedef __packed struct
{
	uint8_t  			sof;		// 同步头
	Vision_Cmd_Id_t  	cmd_id;		// 命令码
	uint8_t  			crc8;		// CRC8校验码
} Vision_Frame_Header_t;

/* 帧尾格式 */
typedef __packed struct 
{
	uint16_t crc16;					// CRC16校验码
} Vision_Frame_Tailer_t;

/* 帧头字节偏移 */
typedef enum {
	 SOF_Vision    =  0 ,//帧头偏移
	 CMD_ID_Vision	=  1 ,//ID偏移
	 CRC8_Vision   =  2 ,//crc8偏移
	 DATA_Vision	 = 3 , //数据起始偏移
	 LEN_FRAME_HEADER = 3	,	// 帧头长度
	 LEN_FRAME_TAILER = 2	,	// 帧尾CRC16 
}Frame_Header_Offset_t;

/* 接收数据长度信息 */
typedef struct {
	/* Std */
	uint8_t LEN_RX_DATA 			;	// 接收数据段长度
  uint8_t LEN_RX_PACKET	    ;	// 接收包整包长度
} DataRX_Length_t;

/* 发送数据长度信息 */
typedef struct {
	/* Std */
  uint8_t TX_CRC16          ;//crc16偏移 
	uint8_t LEN_TX_DATA 		  ;	// 发送数据段长度
	uint8_t LEN_TX_PACKET	    ;	// 发送包整包长度
} DataTX_Length_t;

//------------------------------打符区------------------------


/* 接收数据段格式 */
typedef __packed struct 
{
	float 	pitch_angle;	// pitch偏差角度/像素点	单位：角度/像素点
	float 	yaw_angle;		// yaw偏差角度/像素点	单位：角度/像素点
	float 	distance;			// 距离				单位：mm
	uint8_t identify_framedrop;	// 是否掉帧		单位：0/1
	uint8_t identify_target;// 是否识别到目标	单位：0/1
	uint8_t identify_buff;	// 是否识别到Buff	单位：0/1
	uint8_t identify_too_close;	// 目标距离过近	单位：0/1
	uint8_t anti_gyro;	// 是否识别到小陀螺	单位：0/1
	uint8_t	change_armor;	// 是否在反陀螺状态下切换装甲板	单位：0/1
} BuffAim_Rx_Data_t;

/* 打符发送数据段格式 */
typedef __packed struct
{
	uint8_t fric_speed;		// 射速档位(根据等级来分)
	Color_t my_color;			// 机器人自己的颜色
  float IMU_YAW;
  float IMU_PIT;
  bool IF_anti_top;
} BuffAim_Tx_Data_t;

/* 打符接收包格式 */
typedef __packed struct 
{
	Vision_Frame_Header_t FrameHeader;	// 帧头
	BuffAim_Rx_Data_t	  RxData;		// 数据
	Vision_Frame_Tailer_t FrameTailer;	// 帧尾	
} BuffAim_Rx_Packet_t;


/* 打符发送包格式 */
typedef __packed struct
{
	Vision_Frame_Header_t FrameHeader;	// 帧头
	BuffAim_Tx_Data_t	  TxData;		// 数据
	Vision_Frame_Tailer_t FrameTailer;	// 帧尾		
} BuffAim_Tx_Packet_t;

/*接收端的信息*/
typedef struct
{
  BuffAim_Rx_Packet_t Packet; 
  DataRX_Length_t  LEN;
}BuffAim_Rx_Info_t;

/*发送端的信息*/
typedef struct
{
  BuffAim_Tx_Packet_t Packet;
  DataTX_Length_t  LEN;
}BuffAim_Tx_Info_t;




//------------------------------自瞄区------------------------

/* 接收数据段格式 */
typedef __packed struct 
{
	float 	pitch_angle;	// pitch偏差角度/像素点	单位：角度/像素点
	float 	yaw_angle;		// yaw偏差角度/像素点	单位：角度/像素点
	float 	distance;			// 距离				单位：mm
	uint8_t identify_framedrop;	// 是否掉帧		单位：0/1
	uint8_t identify_target;// 是否识别到目标	单位：0/1
	uint8_t identify_buff;	// 是否识别到Buff	单位：0/1
	uint8_t identify_too_close;	// 目标距离过近	单位：0/1
	uint8_t anti_gyro;	// 是否识别到小陀螺	单位：0/1
	uint8_t	change_armor;	// 是否在反陀螺状态下/打符状态切换装甲板	单位：0/1
} AutoAim_Rx_Data_t;

/* 自瞄发送数据段格式 */
typedef __packed struct
{
  //大于50个字节就需要修改发送缓存的长度
	uint8_t fric_speed;		// 射速档位(根据等级来分)
	Color_t my_color;			// 机器人自己的颜色    //0我蓝色 敌人红色 1我红色
  float IMU_YAW;
  float IMU_PIT;
  bool IF_anti_top;
} AutoAim_Tx_Data_t;

/* 自瞄接收包格式 */
typedef __packed struct 
{
	Vision_Frame_Header_t FrameHeader;	// 帧头
	AutoAim_Rx_Data_t	  RxData;		// 数据
	Vision_Frame_Tailer_t FrameTailer;	// 帧尾	
} AutoAim_Rx_Packet_t;

/* 自瞄发送包格式 */
typedef __packed struct
{
	Vision_Frame_Header_t FrameHeader;	// 帧头
	AutoAim_Tx_Data_t	  TxData;		// 数据
	Vision_Frame_Tailer_t FrameTailer;	// 帧尾		
} AutoAim_Tx_Packet_t;

/*接收端的信息*/
typedef struct
{
  AutoAim_Rx_Packet_t Packet;
  DataRX_Length_t  LEN;
}AutoAim_Rx_Info_t;

/*发送端的信息*/
typedef struct
{
  AutoAim_Tx_Packet_t Packet;
  DataTX_Length_t  LEN;
}AutoAim_Tx_Info_t;

//------------------------------------------



/*电控<-->视觉 通信信息*/
typedef struct
{
  BuffAim_Rx_Info_t BuffAim_RX;
  BuffAim_Tx_Info_t BuffAim_Tx;
  AutoAim_Rx_Info_t AutoAim_Rx;
  AutoAim_Tx_Info_t AutoAim_Tx;
}VisionRTx_t;

typedef struct
{
  int FrameRate;
  float FrameTime;
  bool IF_GET_CNT;
}FrameRate_Cnt_t;


/*视觉任务总控结构体*/
typedef struct
{
  System_Action_t action_mode;
  VisionRTx_t  VisionRTx;
  Vision_Cmd_Id_t System_Cmd_Id;
  bool Trigger_Signal;
  FrameRate_Cnt_t FrameRate_Cnt;
}Vision_Info_t;

extern Vision_Info_t Vision;

void Set_IF_AutoAim(void);
void VISION_GET_Info(void);
void VISION_Ctrl(void);
void AUTO_AIM_Ctrl(void);
void BIG_BUFF_Ctrl(void);
void SMALL_BUFF_Ctrl(void);
void Vision_Reset(void);
void Vision_Init(void);
bool VISION_ReadData(uint8_t *rxBuf);   
void VISION_SendData(Vision_Cmd_Id_t cmd_id);

bool VISION_GetFlagStatus(Vision_Flag flag);

float Get_Aim_distance(void);
Color_t Judge_MyColor(uint8_t robotID);
float Get_FrameTime(void);
float YAW_GetAutoAimErr(void);
float PIT_GetAutoAimErr(void);
float Get_VisionDistance(void);
float YAW_GetBuffCoordinate(void);
float PIT_GetBuffCoordinate(void);

System_Action_t Vision_Handler(void); //视觉多模式控制（内部启动）
void Vision_OFF(void);               //视觉关闭
void Vision_BuffCmd_Switch(void);

void Trigger_ON(void);
void Trigger_OFF(void);
void VisionTrigger_Init(void);
void Trigger(void);//偶数触发 2 4 6 8ms
void Trigger_5ms(void);//奇数触发 5ms
void TriggerSignal_YES(void);
void TriggerSignal_NO(void);
bool Report_TriggerSignal(void);

void Set_IF_ShootSentry(void);
void Reset_IF_ShootSentry(void);
bool Report_IF_ShoorSentry(void);
void Set_IF_BigBuff(void);
void Set_IF_SmallBuff(void);
void Reset_IF_BuffMode(void);

float YawVision_nolook(void);
float PitVision_nolook(void);

void Set_anti_GYRO(void);
void Reset_anti_GYRO(void);
bool Report_IF_anti_GYRO(void);

void Set_ShootBase(void);
void Reset_ShootBase(void);
bool Report_IF_ShootBase(void);

#endif
