/***      《 IMU控制代码 》

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
 
#include "my_imu.h"
extern void DUM_Set_IMU_DataUpdate(void);

/* Init start */
IMU_Info_t IMU = {
  .YAW_Err_Init = 0,
  .PIT_Err_Init = 0,
  .gyrox = 0,
  .gyroy = 0,
  .gyroz = 0,
  .IF_GET_StaticErr = false,
  .IMU_YAW_Speed = 0,
  .IMU_YAW_Angle = 0,
  .IMU_PIT_Speed = 0,
  .IMU_PIT_Angle = 0,
};
/* Init end   */


/**
 * @brief IMU主控函数
 * @param 
 */
void Update_prev_Imu_Yaw(void)
{
  for(int i = 0;i < prev_Imu_Yaw_LEN-1;i++)
  {
    IMU.prev_Imu_Yaw[i] = IMU.prev_Imu_Yaw[i+1];
  }
  IMU.prev_Imu_Yaw[prev_Imu_Yaw_LEN-1] = IMU.IMU_YAW_Angle;
}
void Update_prev_Imu_Pit(void)
{
  for(int i = 0;i < prev_Imu_Pit_LEN-1;i++)
  {
    IMU.prev_Imu_Pit[i] = IMU.prev_Imu_Pit[i+1];
  }
  IMU.prev_Imu_Pit[prev_Imu_Pit_LEN-1] = IMU.IMU_PIT_Angle;
}

void IMU_Ctrl(void)
{
#if (MPU6050 == 1)
  MPU_Get_Gyroscope(&IMU.gyrox,&IMU.gyroy,&IMU.gyroz);	//读取角速度
  Speed_Filter(IMU.gyrox,IMU.gyroy,IMU.gyroz);    //对角速度进行滤波
  IMU_get_angle_to_use();		//获取绝对角度
  if (Master == MASTER_DOWN)DUM_Set_IMU_DataUpdate();//只有下主控发给上主控
 
#elif (BMI270 == 1)
//  BMI_Get_GRO(&IMU.gyrox,&IMU.gyroy,&IMU.gyroz);
  IMU_get_angle_to_use();		//获取绝对角度
  Speed_Filter(IMU.gyrox,IMU.gyroy,IMU.gyroz);    //对角速度进行滤波
  Gimbal_Measure_Data(&IMU);
  if (Master == MASTER_DOWN)DUM_Set_IMU_DataUpdate();//只有下主控发给上主控
  
#endif
  Update_prev_Imu_Yaw();
  Update_prev_Imu_Pit();
}

int8_t rslt;
int pass_num;
bool pass_flag=1;
void IMU_Init(void)
{
#if (MPU6050 == 1)  
  uint32_t XcurrentTime;
  static uint32_t loopTime_mpu6050 = 0;
	MPU_Init();
	while(mpu_dmp_init())//注意自检函数
	{
		XcurrentTime = micros();//获取当前系统时间	
		if((int32_t)(XcurrentTime - loopTime_mpu6050) >= 100000)  
		{	
			loopTime_mpu6050 = XcurrentTime + 100000;			//100ms
			pass_num++;
			if(pass_num>=3)//若超时 则屏蔽自检函数
			{
				pass_flag=0;
				pass_num=10;
			}
		}
	}
  delay_ms(800);
  IMU.IF_GET_StaticErr = Static_Error_Calculate();
#endif
  
#if (BMI270 == 1)
    rslt = BMI_Init();
	while(rslt) {
        // 如果初始化失败则重新初始化
        rslt = BMI_Init();
    }
  delay_ms(800);
    IMU.IF_GET_StaticErr = Static_Error_Calculate();
#endif
  

}

/**
 * @brief 获取mpu速度静差
 * @param 
 */
float MPU_Initcnt = 0;
float static_error_pitch_total =0 , static_error_yaw_total =0 ;//静差计算
bool Static_Error_Calculate(void)//计算静差的函数，如果有返回值则计算成功
{
  #if (MPU6050 == 1)
  int i;
  static float calculate_pitch[CALCULATE_LENGTH] , calculate_yaw[CALCULATE_LENGTH] ;
  float error_pitch1 , error_pitch2 , error_yaw1 , error_yaw2;
  do
  {
    MPU_Get_Gyroscope(&IMU.gyrox,&IMU.gyroy,&IMU.gyroz);	//读 原始数据  gyrox为pitch速度，gyroz为yaw速度

    for(i=(CALCULATE_LENGTH-2);i>-1;i--)
    {
      calculate_pitch[i+1] = calculate_pitch[i];
      calculate_yaw[i+1] = calculate_yaw[i];
    }
    calculate_pitch[0] = (float)IMU.gyrox;
    calculate_yaw[0] = (float)IMU.gyroz;
    
    error_pitch1 = (int16_t)(calculate_pitch[2]-calculate_pitch[1]);
    error_pitch2 = (int16_t)(calculate_pitch[1]-calculate_pitch[0]);
    
    error_yaw1   = (int16_t)(calculate_yaw[2]-calculate_yaw[1]);
    error_yaw2   = (int16_t)(calculate_yaw[1]-calculate_yaw[0]);
  }while(abs(error_pitch1-error_pitch2)>JUDGE_ERR || 
         abs(error_yaw1-error_yaw2)>JUDGE_ERR     ||
         abs(error_pitch1+error_pitch2)>JUDGE_ERR || 
         abs(error_yaw1+error_yaw2)>JUDGE_ERR      );
  
  for(i=0;i<CALCULATE_LENGTH;i++)
  {
    static_error_pitch_total += calculate_pitch[i];
    static_error_yaw_total   += calculate_yaw[i];
  }
  IMU.PIT_Err_Init = static_error_pitch_total/CALCULATE_LENGTH;
  IMU.YAW_Err_Init = static_error_yaw_total  /CALCULATE_LENGTH;
  
  return 1;

  #elif (BMI270 == 1)
  for(uint16_t i=0; i<1000; i++) {
    BMI_Get_GRO(&IMU.gyrox,&IMU.gyroy,&IMU.gyroz);
    static_error_yaw_total   += IMU.gyroz;
    static_error_pitch_total += IMU.gyrox;
	}
    /**
        @note
        如果上电的时候云台运动，会导致计算出来的静态偏差数值出错。如果每次上电的时候，静态偏差均
        差别不大的话，可以直接给定一个固定值。或者，另外对计算出来的偏差值做判断等。
    */
	IMU.YAW_Err_Init = static_error_yaw_total  /1000.f;
	IMU.PIT_Err_Init = static_error_pitch_total/1000.f;
  return 1;
  #endif
  
}

/**
 * @brief 速度滤波
 * @param 
 */
float Pitch_total , Yaw_total ;
void Speed_Filter(short gx,short gy,short gz)//速度滤波
{ 
  #if (MPU6050 == 1)
  gx = (float)gx;
  gy = (float)gy;
  gz = (float)gz;
  #elif (BMI270 == 1)
  gx = (float)gy;
  gz = (float)gz;  
  #endif
  static float Pitch_Filter[SPEED_FILTER_LEN],Yaw_Filter[SPEED_FILTER_LEN];
  static float MPU_yaw_speed = 0,MPU_pitch_speed = 0;
  int i;
  gx = (gx - IMU.PIT_Err_Init); 
  gz = (gz - IMU.YAW_Err_Init); 

  for(i=(SPEED_FILTER_LEN-2);i>-1;i--)
  {
    Pitch_Filter[i+1] =   Pitch_Filter[i];
    Yaw_Filter[i+1]   =   Yaw_Filter[i];
  }
  Pitch_Filter[0] = gx;
  Yaw_Filter[0] = gz;
  Pitch_total = 0;
  Yaw_total = 0;
  for(i=0;i<SPEED_FILTER_LEN;i++)
  {
    Pitch_total = Pitch_total + Pitch_Filter[i];
    Yaw_total   = Yaw_total   + Yaw_Filter[i];
  }
  MPU_yaw_speed = Yaw_total/SPEED_FILTER_LEN;
  MPU_pitch_speed = Pitch_total/SPEED_FILTER_LEN;
  
  if(abs(MPU_yaw_speed)<10)MPU_yaw_speed = 0;
  if(abs(MPU_pitch_speed)<10)MPU_pitch_speed = 0;
  
  IMU.IMU_YAW_Speed = -MPU_yaw_speed ;
  IMU.IMU_PIT_Speed = MPU_pitch_speed ;

}

float imu_yaw_round = 0,imu_pitch_round = 0; //MPU转动角度的圈数,用于计算绝对角度
float yaw_last = 0,pitch_last = 0,pitch_judge_angle = 0;
float pitch,roll,yaw;		//欧拉角
void IMU_angle_process(void)
{
  #if (MPU6050 == 1)
  if(yaw-yaw_last <-300)//向左边
    imu_yaw_round++;
  if(yaw-yaw_last > 300)//向右边3306
    imu_yaw_round--;
  if(pitch>0)imu_pitch_round = 0;
  if(pitch<0)imu_pitch_round = 1;
  #elif (BMI270 == 1)
  if(yaw-yaw_last <-300)//向左边
    imu_yaw_round++;
  if(yaw-yaw_last > 300)//向右边3306
    imu_yaw_round--;
//  if(pitch>0)imu_pitch_round = 0.5f;
//  if(pitch<0)imu_pitch_round = 0.5f;  
  #endif
}
short ggx,ggy,ggz,aax,aay,aaz;
float TIM1t = 0,TIM2t =0 ;
bool Firstin = 0;
void IMU_get_angle_to_use(void)
{
  #if (MPU6050 == 1)
		mpu_dmp_get_data(&roll,&pitch,&yaw);		//读取欧拉角
    IMU_angle_process();                    //判断越界
    yaw_last = yaw;                         //判断越界用的参数
    pitch_last = pitch;
    IMU.IMU_YAW_Angle = (yaw +180+360*imu_yaw_round)*8191/360; //MPU绝对角度yaw
  
    //限定pitch范围
    pitch_judge_angle = pitch +360*imu_pitch_round;
//    if(pitch_judge_angle < PIT_IMU_MAX_ANGLE)pitch_judge_angle = PIT_IMU_MAX_ANGLE; //上
//    if(pitch_judge_angle > PIT_IMU_MIN_ANGLE)pitch_judge_angle = PIT_IMU_MIN_ANGLE; //下
    IMU.IMU_PIT_Angle = (pitch_judge_angle+180)*8191/360; //MPU绝对角度pitch
 
  #elif (BMI270 == 1)
//    BMI_Get_GRO(&ggx,&ggy,&ggz);
//    BMI_Get_ACC(&aax,&aay,&aaz);
          if(Firstin ==0)  TIM1t = micros();
    BMI_Get_RawData(&IMU.gyrox,&IMU.gyroy,&IMU.gyroz,&aax,&aay,&aaz);
    BMI_Get_EulerAngle(&pitch,&roll,&yaw,&IMU.gyrox,&IMU.gyroy,&IMU.gyroz,&aax,&aay,&aaz);
          if(Firstin ==0)   TIM2t = micros();
          Firstin = TIM2t - TIM1t;
    IMU_angle_process();                    //判断越界
    yaw_last = yaw;                         //判断越界用的参数
    pitch_last = pitch;
    IMU.IMU_YAW_Angle = (yaw +180+360*imu_yaw_round)*8191/360; //MPU绝对角度yaw
  
    //限定pitch范围
    pitch_judge_angle = pitch +360*imu_pitch_round;
//    if(pitch_judge_angle < PIT_IMU_MAX_ANGLE)pitch_judge_angle = PIT_IMU_MAX_ANGLE; //上
//    if(pitch_judge_angle > PIT_IMU_MIN_ANGLE)pitch_judge_angle = PIT_IMU_MIN_ANGLE;   //下
    IMU.IMU_PIT_Angle = (pitch_judge_angle+180)*8191/360; //MPU绝对角度pitch
  #endif
}

//int i = 6; //1左边摇先低再高，18左边摇先高再低
float Report_prev_imu_yaw_angle(int i)
{
  return IMU.prev_Imu_Yaw[i];//[n] n越大值越新
}
float Report_prev_imu_pit_angle(int i)
{
  return IMU.prev_Imu_Pit[i];//[n] n越大值越新
}

float Report_YawSpeed(void)
{
  return IMU.IMU_YAW_Speed;
}

float Report_YawAngle(void)
{
  return IMU.IMU_YAW_Angle;  
}

float Report_PitSpeed(void)
{
  return IMU.IMU_PIT_Speed;  
}

float Report_PitAngle(void)
{
  return IMU.IMU_PIT_Angle;  
}
bool IF_IMU_InitOver(void)
{
  return IMU.IF_GET_StaticErr;
}

/**
    @note
    BMI270的速度合成，用于获得陀螺仪角速度
*/
float roll_cos,roll_sin,pitch_cos,pitch_sin;
void Gimbal_Measure_Data(IMU_Info_t *str)
{
    static float roll_cal,pitch_cal;
    roll_cal = (roll*3.1415f)/180.0f;
    pitch_cal = (pitch*3.1415f)/180.0f;
  
    /*世界Yaw轴速度合成*/
    roll_cos = arm_cos_f32(roll_cal);
    roll_sin = arm_sin_f32(roll_cal);
  
  pitch_cos = arm_cos_f32(pitch_cal);
  pitch_sin = arm_sin_f32(pitch_cal);
  
    str->IMU_YAW_Speed = str->gyroz * roll_cos + str->gyrox * roll_sin + \
                         str->gyroz * pitch_cos + str->gyroy * pitch_sin;
}
