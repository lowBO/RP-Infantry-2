/***      《 自瞄算法代码 》

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
#include "AUTO.h"
#include "Device.h"
#include "Function.h"

AutoAim_t AutoAim = {
  .Init_Over = NO,
  .YAW.TargerFix = 0,
  .PIT.TargerFix = PIT_FIX,
};

void AutoAimInit(void)
{
  AutoAim.YAW.Param.Ke = AutoAim_Ke;//视觉的一度等于我的21度，所以角度项要小
  AutoAim.YAW.Param.Ko = ShootSpeed_15_PRE;//预测作用最大的一项 10.7
  AutoAim.YAW.Param.Ka = 9;//9
  AutoAim.YAW.Coordinate.Axis = YAW;
  AutoAim.YAW.Coordinate.fix = 21;//先调到绝对角度不变
  
  AutoAim.PIT.Param.Ke = 0.23;//视觉的一度等于我的100度，所以角度项要小
  AutoAim.PIT.Param.Ko = 0;//预测作用最大的一项
  AutoAim.PIT.Param.Ka = 0;
  AutoAim.PIT.Coordinate.Axis = PIT;
  AutoAim.PIT.Coordinate.fix = 21;//先调到绝对角度不变
}




/**
 * @brief 启动自瞄建立坐标系
 * @note   第一次进入时只执行一次    由vision.c启动
 */
void AutoAim_ON(void)
{
  Coordinate_t *YAW = &AutoAim.YAW.Coordinate;
  Coordinate_t *PIT = &AutoAim.PIT.Coordinate;
  if(!AutoAim.Init_Over)
  {
    Predict_Reset();
    VErrSlope_Reset();
    YAW->abso_Angle = Report_prev_imu_yaw_angle(6);//YAW_GetGyroAngle();
    PIT->abso_Angle = PIT_GetMechAngle();
    AutoAim.Init_Over = YES;
  }
}

/**
 * @brief 自瞄关闭
 * @note  由vision.c关闭
 */
void AutoAim_OFF(void)
{
  AutoAim.Init_Over = NO;
  Predict_Reset();
  VErrSlope_Reset();
}

/**
 * @brief 返回自瞄是否开启
 * @note  
 */
bool Report_IF_AutoAim_ON(void)
{
  return AutoAim.Init_Over;
}
//--------------------------算法层---------------------------


/**
 * @brief 计算坐标系角度
 * @param 
 */
float VisionYawErr_queue[VErrSlope_LEN];
float VisionPitErr_queue[VErrSlope_LEN];
float GetAngle(Coordinate_t *str)
{
  switch(str->Axis)
  {
    case YAW:
        str->Vision_Err = Slope(-YAW_GetAutoAimErr() * str->fix,
                                VisionYawErr_queue,
                                VErrSlope_LEN);
        str->Gim_Err = Report_prev_imu_yaw_angle(6) - str->abso_Angle;//YAW_GetGyroAngle();
      break;
    case PIT:
        str->Vision_Err = Slope(-PIT_GetAutoAimErr() * str->fix,
                                VisionPitErr_queue,
                                VErrSlope_LEN);
        str->Gim_Err = PIT_GetMechAngle() - str->abso_Angle;   
      break;
  }
  
  return str->Vision_Err + str->Gim_Err ;
}
void VErrSlope_Reset(void)
{
  for(int i = 0;i<VErrSlope_LEN;i++)
  {
    VisionYawErr_queue[i] = 0;
    VisionPitErr_queue[i] = 0;
  } 
}

/**
 * @brief 滑动滤波计算
 * @note  把数据往上一层计算
           角度->速度
           速度->加速度
           
 *  滤波长度不要大于100
 */
float Get_SlopeTarget(float M ,float *queue ,uint16_t len)
{
	float sum=0;
	float res=0;
  
//  if(VISION_GetFlagStatus(IF_CHANGE_ARMOR))
//    for(uint16_t k=0;k<len-1;k++)
//    {
//      queue[k] = M;
//    }
  
		//队列已满，FIFO。
		for(uint16_t i=0;i<len-1;i++)
		{
			queue[i] = queue[i+1];
			//更新队列
		}
		queue[len-1] = M;
    
	//更新完队列
	for(uint16_t j=0;j<len;j++)
	{
		sum+=queue[j];
	}
	res = sum/(len);
	
//	res = (M - res);	
	
	return res;
}

float prev_yaw_A = 0,prev_pit_A = 0;
float Get_OmigaData(AutoAim_Data_t *str)
{
  float time = Get_FrameTime();
  float res = 0;
  res = (str->Angle - str->prev_Angle) / time ;
  switch(str->Coordinate.Axis)
  {
    case YAW:
        if(abs(res) <= 0.5f)
        {
          prev_yaw_A = res;
        }
        else res = prev_yaw_A;
        break;
      
    case PIT:
        if(abs(res) <= 0.5f)
        {
          prev_pit_A = res;
        }
        else res = prev_pit_A;   
        break;
  }
  str->prev_Angle = str->Angle;  
  
  return res;
}

float prev_yaw_S = 0,prev_pit_S = 0;
float Get_AccelData(AutoAim_Data_t *str)
{
  float time = Get_FrameTime();
  float res = 0;
  res = (str->Omiga - str->prev_Omiga) / time ;
  switch(str->Coordinate.Axis)
  {
    case YAW:
        if(abs(res) <= 0.5f)
        {
          prev_yaw_S = res;
        }
        else res = prev_yaw_S;
        break;
      
    case PIT:
        if(abs(res) <= 0.5f)
        {
          prev_pit_S = res;
        }
        else res = prev_pit_S;   
        break;
  }
  str->prev_Omiga = str->Omiga;  
  
  return res;
}

//预测量切换
void PRE_switch(void)
{
  AutoAim.YAW.Param.Ke = AutoAim_Ke;
  if(IF_DUM_NORMAL)
  switch(DUM_Report_Shoot_SpeedLimit())
  {
    case 15:
      AutoAim.YAW.Param.Ko = ShootSpeed_15_PRE;
      break;
    
    case 18:
      AutoAim.YAW.Param.Ko = ShootSpeed_18_PRE;
      break;
    
    case 30:
      AutoAim.YAW.Param.Ko = ShootSpeed_30_PRE;
      break;
    
    default:
      AutoAim.YAW.Param.Ko = ShootSpeed_15_PRE;
      break;
  }
  else AutoAim.YAW.Param.Ko = ShootSpeed_15_PRE;
}

/**
 * @brief 自瞄算法
 * @param 
 */
float AutoAim_Algorithm(AutoAim_Data_t *str)
{
  float res;
  str->Predict.start_predict_TIM++;
  /*获取三项*/
  str->Angle = //KalmanFilter(&str->KF.Angle_KF,
                           GetAngle(&str->Coordinate);//);

  str->Omiga = KalmanFilter(&str->KF.Omiga_KF,
                           Get_SlopeTarget(Get_OmigaData(str) ,str->Angle_queue , AngleSlope_LEN));

  str->Accel = KalmanFilter(&str->KF.Accel_KF,
                           Get_SlopeTarget(Get_AccelData(str) ,str->Omiga_queue , OmigaSlope_LEN));
  
 /*获取三项输出*/
  str->Angle_Out = str->Coordinate.Vision_Err * str->Param.Ke  ;
  str->Omiga_Out = str->Omiga * str->Param.Ko;
  str->Accel_Out = str->Accel/2 * str->Param.Ka;
  
  if(str->Predict.start_predict_TIM > 30)//30ms后开启预测,防止数据突变
    str->Predict.START_PREDICT = NO;//YES;
  if(str->Predict.START_PREDICT)
    str->Out = str->Angle_Out + str->Omiga_Out + str->Accel_Out;
  else str->Out = str->Angle_Out;
  
  res = str->Out;
  res = KalmanFilter(&str->KF.Out_KF , str->Out);
  return res;
}

/**
 * @brief 滑动滤波数组数据清空
 * @param 
 */
void SlopeData_Reset(void)
{
  for(int i;i<OmigaSlope_LEN;i++)
  {
    AutoAim.YAW.Omiga_queue[i] = 0;
    AutoAim.YAW.Angle_queue[i] = 0;
    
    AutoAim.PIT.Omiga_queue[i] = 0;
    AutoAim.PIT.Angle_queue[i] = 0;
  }
}

void Predict_Reset(void)
{
  //可以进行参数的变化
  SlopeData_Reset();
  AutoAim.YAW.Predict.START_PREDICT = NO;
  AutoAim.PIT.Predict.START_PREDICT = NO;
  AutoAim.YAW.Predict.start_predict_TIM = 0;
  AutoAim.PIT.Predict.start_predict_TIM = 0;
}

bool Report_IF_START_yawPREDICT(void)
{
  return AutoAim.YAW.Predict.START_PREDICT ;
}
bool Report_IF_START_pitPREDICT(void)
{
  return AutoAim.PIT.Predict.START_PREDICT ;
}
/**
 * @brief YAW自瞄输出口，供云台调用
 * @param 
 */
bool YawAutoAim_Data_Update = false;
float AutoAimYaw_Ctrl(void)
{
  float res;
  
  /*根据裁判系统射速切换预测量*/
  PRE_switch();
  
  if(YawAutoAim_Data_Update == true){
    if(VISION_GetFlagStatus(IF_LOCK_ARMOR))
    {
//      //切换装甲板或识别小陀螺都关预测
//      if(VISION_GetFlagStatus(IF_CHANGE_ARMOR) || VISION_GetFlagStatus(IF_IDENTIFY_SPIN))
//        Predict_Reset();
      
////      //距离太近关预测
////      if(Get_Aim_distance()<680.1f)
////      {
////        Predict_Reset();
////        AutoAim.YAW.Param.Ke = 0.6;//提高跟随量
////      }
      
      //自主开启反陀螺模式，关预测
      if(Report_IF_anti_GYRO())
      {
        Predict_Reset();
        AutoAim.YAW.Param.Ke = 0.25;//跟随量减少
      }
      
      res = AutoAim_Algorithm(&AutoAim.YAW) + AutoAim.YAW.TargerFix +  + AutoAim_Barrelturn();
      YawAutoAim_Data_Update = false;
    }
    else 
    {
      res = YawVision_nolook();
      Predict_Reset();
      YawAutoAim_Data_Update = false;
    }
//    /*处理完之后提醒进行下一次硬触发*/
//    TriggerSignal_YES();
  }else res = YawVision_nolook();//if(!VISION_GetFlagStatus(IF_LOCK_ARMOR))
  
  if(Judge_IF_NAN(res))res = 0;
  
  return res;
}

/**
 * @brief Pit自瞄输出口，供云台调用
 * @param 
 */
bool PitAutoAim_Data_Update = false;
float AutoAimPit_Ctrl(void)
{
  float res;
  
  if(PitAutoAim_Data_Update == true){
    if(VISION_GetFlagStatus(IF_LOCK_ARMOR))
    {
      res = AutoAim_Algorithm(&AutoAim.PIT) + AutoAim.PIT.TargerFix + Report_PIT_AUTO_NUM();
      PitAutoAim_Data_Update = false;
    }
    else 
    {
      res = PitVision_nolook();
      Predict_Reset();
      PitAutoAim_Data_Update = false;
    }
//    /*处理完之后提醒进行下一次硬触发*/
//    TriggerSignal_YES();
  }else res = PitVision_nolook(); 
  
  if(Judge_IF_NAN(res))res = 0;
 
  return res;
}


//-------------------------------自动打弹------------------------------------

void AutoShoot_Reset(void)
{
  AutoAim_Data_t *str = &AutoAim.YAW;
  AutoShoot_t *Shoot = &str->AutoShoot;
    if(Shoot->ConFireAllow_cnt>=1000)
      ContinFire_OFF();//关连发
    Shoot->SinFireAllow_cnt = 0;//单发计时清0
    Shoot->ConFireAllow_cnt = 0;//连发计时清0 
}

void Yaw_AutoShoot(void)
{
  AutoAim_Data_t *str = &AutoAim.YAW;
  AutoShoot_t *Shoot = &str->AutoShoot;
  if(Report_IF_START_yawPREDICT()){
  if(VISION_GetFlagStatus(IF_LOCK_ARMOR))
  {
    if(str->Omiga > 2 && str->Coordinate.Vision_Err < -14)//左超
    {
      //符合打弹允许条件
      Shoot->SinFireAllow_cnt++;//计时
      //Shoot->ConFireAllow_cnt++;//计时
      if(Shoot->SinFireAllow_cnt > 350 && Shoot->ConFireAllow_cnt<1000)//计时有效
      {
        SingleFireRequest();//请求打弹
        Shoot->SinFireAllow_cnt = 0;
      }
      if(Shoot->ConFireAllow_cnt >= 1000)
      {
        ContinFire_ON();//开连发
      }
      
    }
    else if(str->Omiga < -2 && str->Coordinate.Vision_Err > 14)//右超
    {
      //符合打弹允许条件
      Shoot->SinFireAllow_cnt++;//计时
      //Shoot->ConFireAllow_cnt++;//计时
      if(Shoot->SinFireAllow_cnt > 150 && Shoot->ConFireAllow_cnt<1000)//计时有效
      {
        SingleFireRequest();//请求打弹
        Shoot->SinFireAllow_cnt = 0;
      }
      if(Shoot->ConFireAllow_cnt >= 1000)
      {
        ContinFire_ON();//开连发
      }
    }else AutoShoot_Reset();    //不符合自动开火条件则复位 
  }else AutoShoot_Reset();      //视觉没识别到则复位
  }else AutoShoot_Reset();      //没开启预测则复位
}


bool Report_IF_Auto_InitOver(void)
{
  return AutoAim.Init_Over;
}


//--------------------------------------硬触发----------------------------------------------
float YAW_IMU_Angle[2];//触发数值缓冲队列
float PIT_IMU_Angle[2];//触发数值缓冲队列
void AutoAim_Trigger(void)
{
  AutoAim.YAW.Coordinate.Gim_Err = YAW_GetGyroAngle() - AutoAim.YAW.Coordinate.abso_Angle;
  AutoAim.PIT.Coordinate.Gim_Err = PIT_GetMechAngle() - AutoAim.PIT.Coordinate.abso_Angle;
}



float PIT_AUTO_NUM = 0;
void PIT_AUTO_ADD(void)
{
  PIT_AUTO_NUM = PIT_AUTO_NUM - 10;
}
void PIT_AUTO_DEC(void)
{
  PIT_AUTO_NUM = PIT_AUTO_NUM + 10;
}
float Report_PIT_AUTO_NUM(void)
{
  return PIT_AUTO_NUM;
}

