#include "iic.h"
#include "delay.h"
void I2C_SDA_OUT(void)//SDA输出方向配置
{
    GPIO_InitTypeDef GPIO_InitStructure;	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//SDA推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure); 						

}

void I2C_SDA_IN(void)//SDA输入方向配置
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//SCL上拉输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
}
//以下为模拟IIC总线函数
void IIC_init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PD端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//PD6配置为推挽输出,SCL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD
	GPIO_SetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7); 
//	I2C_SCL_H;
//	I2C_SDA_H;//均拉高
}
void IIC_start()
{
	I2C_SDA_OUT();
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	DelayUs(5);
	IIC_SDA=0;
	DelayUs(5);
	IIC_SCL=0;
}
void IIC_stop()
{
	I2C_SDA_OUT();
	IIC_SCL=0;
	IIC_SDA=0;
	DelayUs(5);
	IIC_SCL=1; 
	IIC_SDA=1;
	DelayUs(5);
}
//主机产生一个应答信号
void IIC_ack()
{
	IIC_SCL=0;
	I2C_SDA_OUT();
  IIC_SDA=0;
   DelayUs(2);
   IIC_SCL=1;
   DelayUs(5);
   IIC_SCL=0;	
}
//主机不产生应答信号
void IIC_noack()
{
	IIC_SCL=0;
	I2C_SDA_OUT();
   IIC_SDA=1;
   DelayUs(2);
   IIC_SCL=1;
   DelayUs(2);
   IIC_SCL=0;
}
//等待从机应答信号
//返回值：1 接收应答失败
//		  0 接收应答成功
u8 IIC_wait_ack()
{
	u8 tempTime=0;
	I2C_SDA_IN();
	IIC_SDA=1;
	DelayUs(1);
	IIC_SCL=1;
	DelayUs(1);

	while(READ_SDA)
	{
		tempTime++;
		if(tempTime>250)
		{
			IIC_stop();
			return 1;
		}	 
	}

	IIC_SCL=0;
	return 0;
}
void IIC_send_byte(u8 txd)
{
	u8 i=0;
	I2C_SDA_OUT();
	IIC_SCL=0;;//拉低时钟开始数据传输
	for(i=0;i<8;i++)
	{
		IIC_SDA=(txd&0x80)>>7;//读取字节
		txd<<=1;
		IIC_SCL=1;
		DelayUs(2); //发送数据
		IIC_SCL=0;
		DelayUs(2);
	}
}
//读取一个字节
u8 IIC_read_byte(u8 ack)
{
	u8 i=0,receive=0;
	I2C_SDA_IN();
   for(i=0;i<8;i++)
   {
   		IIC_SCL=0;
		DelayUs(2);
		IIC_SCL=1;
		receive<<=1;//左移
		if(READ_SDA)
		   receive++;//连续读取八位
		DelayUs(1);	
   }

   	if(!ack)
	   	IIC_noack();
	else
		IIC_ack();

	return receive;//返回读取到的字节
}

	
