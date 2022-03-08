#ifndef __ADXL345_H
#define __ADXL345_H
#include "sys.h"
/************************************************
		ADXL345数字加速度说明文件
描述    : ADXL345数字加速度.h文件
功能    : 声明函数以及相关接口的声明
占用资源: 当ADXL345数字加速度工作时,占用
          100%的CPU资源
硬件说明: STC系列1T单片机,配合11.0592MHz晶振
创建者  : J.H.Chen
创建时间: 2013.08.29
说明    : 详细的定义请看函数实现
************************************************/

#define X_AXLE 0   //x轴
#define Y_AXLE 1   //y轴
#define Z_AXLE 2   //z轴

#define	slaveaddress   0xA6	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
//                              ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

#define THRESH_TAP     0X1D	  //敲击中断阈值(用于正常敲击检测)
//                              16g模式中,62.5mg/Bit

#define OFSX           0X1E   //X轴偏移寄存器  15.6mg/Bit  0xff = 4g
#define OFSY           0X1F   //X轴偏移寄存器  15.6mg/Bit  0xff = 4g
#define OFSZ           0X20   //X轴偏移寄存器  15.6mg/Bit  0xff = 4g
#define DUR            0x21   //敲击阈值时间   625uS/Bit
#define LATENT         0X22   //敲击事件到时间窗口的延迟时间,在此期间
//                              可检测第二次敲击时间  1.25mg/Bit
#define WINDOW         0X23   //敲击窗口,延迟时间满后的时间量,在此期间
//                              能开始进行第二次有效敲击  1.25mg/Bit
#define THRESH_ACT     0X24   //检测活动的阈值,活动事件的幅度与该寄存器
//                              的值进行比较  62.5mg/Bit
#define	THRESH_INACT   0X25   //检测静止的阈值,静止事件的幅度与该寄存器
//                              的值进行比较  62.5mg/Bit
#define TIME_INACT     0X26   //加速度时间量小于该寄存器的值表示静止 1S/Bit

#define ACT_INACT_CTL  0X27 
//Bit  7,3  ACT交流/直流[INACT交流/直流]:0选择直流耦合;1使能交流耦合 直流时将
//                                       当前加速度值直接与THRESH_ACT和THRESH_INACT
//                                       进行比较,确定检测到的是活动还是静止
//     6,2  ACT_X使能[INACT_X使能]:设置为1,使能X轴参与检测活动或静止;活动检测时,
//                                 所有轴为逻辑"或",有任意轴超过阈值时,活动功能触发
//                                 禁止检测时,所有轴为逻辑"与",只有当所有轴低于阈值
//                                 时,静止功能触发
//     5,1  ACT_Y使能[INACT_Y使能]:与X轴类似 
//     4,0  ACT_Z使能[INACT_Z使能]:与X轴类似
#define THRESH_FF      0X28   //阈值,用于自由落体检测,所有轴加速度与该寄存器值比较
//                              ,以确定是否发生自由落体 62.5mg/Bit,建议300mg与600mg之间
#define TIME_FF        0X29   //维持THRESH_FF阈值的最小时间,以生成自由落体中断 5mS/Bit

#define TAP_AXES       0X2A   
// Bit  7:4  0
//       3   抑制  两次敲击之间出现大于THRESH_TAP值得加速度,设置抑制会抑制双击检测
//       2   TAP_X使能  设置为1时,使能X轴进行敲击检测,0时排除该轴的敲击检测
//       1   TAP_Y使能  设置为1时,使能Y轴进行敲击检测,0时排除该轴的敲击检测
//       0   TAP_Z使能  设置为1时,使能Z轴进行敲击检测,0时排除该轴的敲击检测
#define ACT_TAP_STATUS 0X2B   /*只读寄存器*/
// Bit  7   0
//     6,2  ACT_X来源,TAP_X来源:表示涉及敲击或活动事件的第一轴,设置为1时,对应事件参与
//                              设置为0时,对应未参与.不会自动清零,新数据覆盖,中断清零前
//                              应读取该寄存器
//     5,1	ACT_Y来源,TAP_Y来源:与X相似
//     4,0	ACT_Z来源,TAP_Z来源:与X相似
//      3   休眠设置为1时,器件进入休眠状态
#define BW_RATE        0X2C  
// Bit  7:5   0
//       4	 LOW_POWER 低功耗位,0选择正常模式,1进入低功耗模式
//      3:0  速率位
#define POWER_CTL      0X2D
// Bit  7,6   0
//       5    链接   设置1时,延迟活动开始,直到检测到静止.检测到活动后,禁止检测开始,活动
//                   检测停止,设置时动态链接活动和静止交替检测;设置0时静止与活动同时检测
//       4    AUTO_SLEEP  设置1时自动休眠,检测出静止后,进行休眠模式,活动使能后被唤醒
//       3    测量   0待机 1测量模式
//       2	  休眠   0普通 1休眠
//      1,0	  唤醒(休眠模式下的读取频率) "00":8HZ  "01":4HZ  "10":2HZ  "11":1HZ
#define INT_ENABLE     0X2E  //中断使能配置
// Bit  7  DATA_READY
//      6  SINGLE_TAP
//      5  DOUBLE_TAP
//      4  Activity
//      3  Inactivity
//      2  FREE_FALL   自由落体中断
//      1  Watermark
//      0  Overrun
#define INT_MAP        0X2F   //中断映射 自读寄存器
//位与INT_ENABLE对应,,设置为0,该中断映射到INT1引脚;设置为1,该中断映射到INT2引脚
#define INT_SOURCE     0X30   //中断来源
//位与INT_ENABLE对应,1表示该功能触发
#define DATA_FORMAT    0X31
// Bit   7  SELF_TEST  设置1,自测力应用至传感器,造成输出数据转换;0时禁用自测力
//       6  SPI        1设置为3线SPI模式,0时设置4线SPI模式
//       5  INT_INVERT 0时中断高电平有效,1时低电平有效
//       4	0
//       3  FULL_RES   1时设置全分辨率模式,输出以4mg/Bit增加;0时为10位模式
//       2  Justify    1为左对齐模式;8为右对齐模式,并带有符号扩展
//      1:0 范围位     "00"±2g  "01"±4g  "10"±8g  "11"±16g
#define DATAX0         0X32
#define DATAX1         0X33  //与DATAX0组成x轴输出数据(二进制补码),DATAX1为高位,4mg/Bit
#define DATAY0         0X34
#define DATAY1         0X35  //与DATAY0组成Y轴输出数据(二进制补码),DATAY1为高位,4mg/Bit
#define DATAZ0         0X36
#define DATAZ1         0X37  //与DATAZ0组成Z轴输出数据(二进制补码),DATAZ1为高位,4mg/Bit
#define FIFO_CTL       0X38
// Bit  7,6  FIFO_MODE  "00" 旁路模式
//						"01" FIFO模式 可收集最多32个值,然后停止收集数据
//						"10" 流模式   FIFO保存最后32个数据值,FIFO满时,新数据覆盖最早数据(先进先出)
//						"11" 触发器   通过触发位触发,FIFO在触发事件前保存最后的数据样本,然后
//									  继续收集数据直到填满;填满后,不再收集新数据
//		 5   触发位     0链接触发器模式下的触发事件至INT1,1链接至INT2
//		 4:0 样本		功能取决于FIFO模式:FIFO模式时,指定触发水印中断需要的FIFO条目数
//                                         流模式时,指定触发水印中断需要的FIFO条目数
//                                         触发器模式:指定触发事件之前在FIFO缓冲区要保留的FIFO样本数
//                      样本位设置为0时,不管哪种FIFO模式,立即在INT_SOURCE寄存器设置水印状态位
#define FIFO_STATUS    0X39	  /*只读寄存器*/
// Bit   7   FIFO_TRIG  FIFO_TRIG为1时表示有触发事件发生 
//       6   0
//      5:0	 条目位 报告FIFO存储的数据值的数量

//#define slaveaddress 0XA6//write
#define regaddress 0XA7//read
#define DEVICE_ID 0X00
void adxl345_init(void);
u8 adxl345_read_reg(u8 addr);
void adxl345_write_reg(u8 addr,u8 val);
void adxl345_read_data(short *x,short *y,short *z);
void adxl345_read_average(float *x,float *y,float *z,u8 times);
void ADXL345_Start(void);
void ADXL345_Stop(void);
void get_angle(float x_angle,float y_angle,float z_angle);
#endif
