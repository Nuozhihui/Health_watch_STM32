#include "ds1302.h"
#include "delay.h"

DATE SysDate = {00, 00, 12, 22,7,4, 21};  //Date: 2021.7.22 Time: 12:00:00 Week: 4(Sat)

void DAT_MODE_OUT(void)//IO引脚配置成输出模式
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void DAT_MODE_IN(void)//IO引脚配置成输如模式
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //Set as pull up res
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief DS1302引脚初始化
 */
void DS1302_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);//C13,C14,C15初始化为低电平
}

/**
 * @brief 单字节写
 * @param 待写的数据
 */
void DS1302_WriteOneByte(uint8_t data) 
{
	uint8_t index = 0;
	DAT_MODE_OUT();//IO引脚配置成输出模式
	for(index=0; index<8; index++)
	{
		SCK_CLR;
 
		if (data & 0x01) //数据由低到高
        {
			DAT_SET;//置1
        }
		else
        { 
			DAT_CLR;//置0
        }
				//tdc = 200ns, tcl = 1us
				DelayUs(2);
        SCK_SET; /* 上升沿写数据 */
				//tcdh = 280ns, tch = 1us
				DelayUs(2);
		data >>= 1;      /* 从最低位开始写 */
	}
}

/**
 * @brief 向指定地址写入指定数据
 * @param 指定地址
 * @param 指定数据
 */
void DS1302_WriteByte(const uint8_t addr, const uint8_t data)
{
	RST_CLR;
	SCK_CLR;
	//tcwh = 4us
	DelayUs(5);
	RST_SET;  
	//tcc = 4us
	DelayUs(5);
	DS1302_WriteOneByte(addr);
	DS1302_WriteOneByte(data);
	//tcch = 240ns, tch = 1us
	//delay_us(1);
	RST_CLR;
	SCK_CLR;

}

/**
 * @brief 单字节读
 * @param 待读的地址
 */
uint8_t DS1302_ReadByte(const uint8_t addr)
{
	uint8_t index = 0, data = 0;
	
	RST_CLR;
	SCK_CLR;
	//tcwh = 4us
	DelayUs(5);		
	RST_SET;
	//tcc = 4us
	DelayUs(5);	
	DS1302_WriteOneByte(addr);
    DAT_MODE_IN();//IO配置成输入模式
	
	for(index=0; index<8; index++)
	{
		SCK_CLR;
        //SCK_SET;
		//tcdd = 800ns tcl = 1us
		DelayUs(2);
		data >>= 1; //数据是由高到低的顺序读取   0100 0000  
		if (DAT_READ == 1) /* 下降沿读数据 */
    {
			data |= 0x80;   
		}
		//data >>= 1;
		//SCK_CLR;
		SCK_SET;
		//tch = 1us
		DelayUs(2);
	}
	SCK_CLR;
	RST_CLR;
	return data;
}

/**
 * @brief DS1302设置
 * @param 日期/时间结构体指针
 */
int DS1302_DateSet(const DATE * const date)
{
	DS1302_WriteByte(WRITE_CONTROL_REG, 0x00); /* 去除写保护 */
	
	//The MSB of sec reg must be 0 to enable the clock!!
	DS1302_WriteByte(WRITE_SEC_REG,  DataToBcd(date->sec)); //写秒 
	
	DS1302_WriteByte(WRITE_MIN_REG,  DataToBcd(date->min));//写分
	DS1302_WriteByte(WRITE_HOUR_REG, DataToBcd(date->hour));//写小时
	DS1302_WriteByte(WRITE_DAY_REG,  DataToBcd(date->day));//写天
	DS1302_WriteByte(WRITE_MON_REG,  DataToBcd(date->mon));//写月
	DS1302_WriteByte(WRITE_WEEK_REG, DataToBcd(date->week));//写星期
	DS1302_WriteByte(WRITE_YEAR_REG, DataToBcd(date->year));//写年
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x80); /* 加上写保护 */
	return 0;
}

/**
 * @brief DS1302读取
 * @param 日期/时间结构体指针
 */
int DS1302_DateRead(DATE * const date)
{
	date->sec  = BcdToData(DS1302_ReadByte(READ_SEC_REG));//读秒
	date->min  = BcdToData(DS1302_ReadByte(READ_MIN_REG));//读分
	date->hour = BcdToData(DS1302_ReadByte(READ_HOUR_REG));//读小时
	date->day  = BcdToData(DS1302_ReadByte(READ_DAY_REG));//读天
	date->mon  = BcdToData(DS1302_ReadByte(READ_MON_REG));//读月
	date->week = BcdToData(DS1302_ReadByte(READ_WEEK_REG));//读星期
	date->year = BcdToData(DS1302_ReadByte(READ_YEAR_REG));//读年
	return 0;
}

/**
 * @brief DS1302初始化
 * @param 日期/时间结构体指针
 * @note  如果是充电电池,可以开启在正常情况下电源对电池的涓流充电功能,只有在异常情况下才使用电池供电
 * @note  直接利用DS1302片上RAM实现 1>如果是第一次上电,则设置日期/时间 2>否则,就不需要设置日期/时间
 */
int DS1302_Init(const DATE * const date)
{
	DS1302_GPIO_Init();//引脚初始化
    if (BcdToData(DS1302_ReadByte(READ_RAM_REG)) == 0x01) /* 如果不是第一次上电,则直接退出 */
	{
		return 0;
	}
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x00); /* 去除写保护 */
//  DS1302_WriteByte(WRITE_CHARGE_REG, 0xa9);  /* 使能电池涓流充电功能(一定要是充电电池才可以使用此功能!!!) */
    DS1302_WriteByte(WRITE_RAM_REG, DataToBcd(0x01));//标志写1
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x80); /* 加上写保护 */
	DS1302_DateSet(date);
	return 0;
}

