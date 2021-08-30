/**    
  * @note 
  * 一阶卡尔曼滤波器来自RoboMaster论坛  
  *   一维卡尔曼滤波器                     
  *   使用时先定义一个kalman指针，然后调用kalmanCreate()创建一个滤波器 
  *   每次读取到传感器数据后即可调用KalmanFilter()来对数据进行滤波
  *          使用示例                                             
  *          extKalman_t p;                  //定义一个卡尔曼滤波器结构体                                                 
  *          float SersorData;             //需要进行滤波的数据                                          
  *          KalmanCreate(&p,20,200);      //初始化该滤波器的Q=20 R=200参数                                                  
  *          while(1)                                                                
  *          {                                                                            
  *             SersorData = sersor();                     //获取数据                                           
  *             SersorData = KalmanFilter(&p,SersorData);  //对数据进行滤波                                                                            
  *          }                                                                            
  */

#include "kalman.h"
#include "Device.h"
#include "Function.h"

/**
  * @name   kalmanCreate
  * @brief  创建一个卡尔曼滤波器
  * @param  p:  滤波器
  *         T_Q:系统噪声协方差
  *         T_R:测量噪声协方差
  *         
  * @retval none
  * @attention R固定，Q越大，代表越信任侧量值，Q无穷代表只用测量值
  *		       	反之，Q越小代表越信任模型预测值，Q为零则是只用模型预测
  */
void KalmanCreate(extKalman_t *p,float T_Q,float T_R)
{
    p->X_last = (float)0;
    p->P_last = 0;
    p->Q = T_Q;
    p->R = T_R;
    p->A = 1;
    p->B = 0;
    p->H = 1;
    p->X_mid = p->X_last;
}

/**
  * @name   KalmanFilter
  * @brief  卡尔曼滤波器
  * @param  p:  滤波器
  *         dat:待滤波数据
  * @retval 滤波后的数据
  * @attention Z(k)是系统输入,即测量值   X(k|k)是卡尔曼滤波后的值,即最终输出
  *            A=1 B=0 H=1 I=1  W(K)  V(k)是高斯白噪声,叠加在测量值上了,可以不用管
  *            以下是卡尔曼的5个核心公式
  *            一阶H'即为它本身,否则为转置矩阵
  */

float KalmanFilter(extKalman_t* p,float dat)
{
    p->X_mid =p->A*p->X_last;                     //百度对应公式(1)    x(k|k-1) = A*X(k-1|k-1)+B*U(k)+W(K)     状态方程
    p->P_mid = p->A*p->P_last+p->Q;               //百度对应公式(2)    p(k|k-1) = A*p(k-1|k-1)*A'+Q            观测方程
    p->kg = p->P_mid/(p->P_mid+p->R);             //百度对应公式(4)    kg(k) = p(k|k-1)*H'/(H*p(k|k-1)*H'+R)   更新卡尔曼增益
    p->X_now = p->X_mid + p->kg*(dat-p->X_mid);   //百度对应公式(3)    x(k|k) = X(k|k-1)+kg(k)*(Z(k)-H*X(k|k-1))  修正估计值
    p->P_now = (1-p->kg)*p->P_mid;                //百度对应公式(5)    p(k|k) = (I-kg(k)*H)*P(k|k-1)           更新后验估计协方差
    p->P_last = p->P_now;                         //状态更新
    p->X_last = p->X_now;
    return p->X_now;							  //输出预测结果x(k|k)
}



extern extKalman_t KF_Mouse_X_Speed,KF_Mouse_Y_Speed;
void Kalman_Init(void)
{
  //鼠标
    KalmanCreate(&KF_Mouse_X_Speed,1,60);
    KalmanCreate(&KF_Mouse_Y_Speed,1,60);
  //普通模式
    //云台yaw
    KalmanCreate(&Gimbal.YAW.KF_Angle[RC_MECH],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[RC_GYRO],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[KEY_MECH],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[KEY_GYRO],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[ACT_AUTO_AIM],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[ACT_BIG_BUFF],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[ACT_SMALL_BUFF],1,30);
    KalmanCreate(&Gimbal.YAW.KF_Angle[ACT_PARK],1,30);

    //云台pitch
    KalmanCreate(&Gimbal.PIT.KF_Angle[RC_MECH],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[RC_GYRO],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[KEY_MECH],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[KEY_GYRO],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[ACT_AUTO_AIM],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[ACT_BIG_BUFF],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[ACT_SMALL_BUFF],1,30);
    KalmanCreate(&Gimbal.PIT.KF_Angle[ACT_PARK],1,30);
    
    //底盘电机
    KalmanCreate(&Chassis.RC_Move.GYRO_Move.KF_GYRO_Angle,1,10);
    KalmanCreate(&Chassis.Key_Move.GYRO_Move.KF_GYRO_Angle,1,10);  
    KalmanCreate(&Chassis.SPIN_Move.GYRO_Move.KF_GYRO_Angle,1,0); 
    
    //舵轮电机
    KalmanCreate(&Rudder.Rudder_Axis[RUD_LF].KF_Angle[RC_GYRO],1,30);
    KalmanCreate(&Rudder.Rudder_Axis[RUD_RF].KF_Angle[RC_GYRO],1,30);
    KalmanCreate(&Rudder.Rudder_Axis[RUD_LB].KF_Angle[RC_GYRO],1,30);
    KalmanCreate(&Rudder.Rudder_Axis[RUD_RB].KF_Angle[RC_GYRO],1,30);


  //特殊模式
    //自瞄
    KalmanCreate(&AutoAim.YAW.KF.Angle_KF , 1,40);
    KalmanCreate(&AutoAim.YAW.KF.Omiga_KF , 1,35);
    KalmanCreate(&AutoAim.YAW.KF.Accel_KF , 1,35);
    KalmanCreate(&AutoAim.YAW.KF.Out_KF , 1 ,35);
    KalmanCreate(&AutoAim.PIT.KF.Angle_KF , 1,40);
    KalmanCreate(&AutoAim.PIT.KF.Omiga_KF , 1,35);
    KalmanCreate(&AutoAim.PIT.KF.Accel_KF , 1,35);
    KalmanCreate(&AutoAim.PIT.KF.Out_KF , 1 ,35);
    //打符
    KalmanCreate(&BuffAim.YAW.KF , 1 , 20 );
    KalmanCreate(&BuffAim.PIT.KF , 1 , 20 );   
    
    
  
    

  
}
