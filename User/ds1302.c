#include "ds1302.h"
#include "delay.h"

DATE SysDate = {00, 00, 12, 22,7,4, 21};  //Date: 2021.7.22 Time: 12:00:00 Week: 4(Sat)

void DAT_MODE_OUT(void)//IO�������ó����ģʽ
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void DAT_MODE_IN(void)//IO�������ó�����ģʽ
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //Set as pull up res
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief DS1302���ų�ʼ��
 */
void DS1302_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);//C13,C14,C15��ʼ��Ϊ�͵�ƽ
}

/**
 * @brief ���ֽ�д
 * @param ��д������
 */
void DS1302_WriteOneByte(uint8_t data) 
{
	uint8_t index = 0;
	DAT_MODE_OUT();//IO�������ó����ģʽ
	for(index=0; index<8; index++)
	{
		SCK_CLR;
 
		if (data & 0x01) //�����ɵ͵���
        {
			DAT_SET;//��1
        }
		else
        { 
			DAT_CLR;//��0
        }
				//tdc = 200ns, tcl = 1us
				DelayUs(2);
        SCK_SET; /* ������д���� */
				//tcdh = 280ns, tch = 1us
				DelayUs(2);
		data >>= 1;      /* �����λ��ʼд */
	}
}

/**
 * @brief ��ָ����ַд��ָ������
 * @param ָ����ַ
 * @param ָ������
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
 * @brief ���ֽڶ�
 * @param �����ĵ�ַ
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
    DAT_MODE_IN();//IO���ó�����ģʽ
	
	for(index=0; index<8; index++)
	{
		SCK_CLR;
        //SCK_SET;
		//tcdd = 800ns tcl = 1us
		DelayUs(2);
		data >>= 1; //�������ɸߵ��͵�˳���ȡ   0100 0000  
		if (DAT_READ == 1) /* �½��ض����� */
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
 * @brief DS1302����
 * @param ����/ʱ��ṹ��ָ��
 */
int DS1302_DateSet(const DATE * const date)
{
	DS1302_WriteByte(WRITE_CONTROL_REG, 0x00); /* ȥ��д���� */
	
	//The MSB of sec reg must be 0 to enable the clock!!
	DS1302_WriteByte(WRITE_SEC_REG,  DataToBcd(date->sec)); //д�� 
	
	DS1302_WriteByte(WRITE_MIN_REG,  DataToBcd(date->min));//д��
	DS1302_WriteByte(WRITE_HOUR_REG, DataToBcd(date->hour));//дСʱ
	DS1302_WriteByte(WRITE_DAY_REG,  DataToBcd(date->day));//д��
	DS1302_WriteByte(WRITE_MON_REG,  DataToBcd(date->mon));//д��
	DS1302_WriteByte(WRITE_WEEK_REG, DataToBcd(date->week));//д����
	DS1302_WriteByte(WRITE_YEAR_REG, DataToBcd(date->year));//д��
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x80); /* ����д���� */
	return 0;
}

/**
 * @brief DS1302��ȡ
 * @param ����/ʱ��ṹ��ָ��
 */
int DS1302_DateRead(DATE * const date)
{
	date->sec  = BcdToData(DS1302_ReadByte(READ_SEC_REG));//����
	date->min  = BcdToData(DS1302_ReadByte(READ_MIN_REG));//����
	date->hour = BcdToData(DS1302_ReadByte(READ_HOUR_REG));//��Сʱ
	date->day  = BcdToData(DS1302_ReadByte(READ_DAY_REG));//����
	date->mon  = BcdToData(DS1302_ReadByte(READ_MON_REG));//����
	date->week = BcdToData(DS1302_ReadByte(READ_WEEK_REG));//������
	date->year = BcdToData(DS1302_ReadByte(READ_YEAR_REG));//����
	return 0;
}

/**
 * @brief DS1302��ʼ��
 * @param ����/ʱ��ṹ��ָ��
 * @note  ����ǳ����,���Կ�������������µ�Դ�Ե�ص������繦��,ֻ�����쳣����²�ʹ�õ�ع���
 * @note  ֱ������DS1302Ƭ��RAMʵ�� 1>����ǵ�һ���ϵ�,����������/ʱ�� 2>����,�Ͳ���Ҫ��������/ʱ��
 */
int DS1302_Init(const DATE * const date)
{
	DS1302_GPIO_Init();//���ų�ʼ��
    if (BcdToData(DS1302_ReadByte(READ_RAM_REG)) == 0x01) /* ������ǵ�һ���ϵ�,��ֱ���˳� */
	{
		return 0;
	}
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x00); /* ȥ��д���� */
//  DS1302_WriteByte(WRITE_CHARGE_REG, 0xa9);  /* ʹ�ܵ�������繦��(һ��Ҫ�ǳ���زſ���ʹ�ô˹���!!!) */
    DS1302_WriteByte(WRITE_RAM_REG, DataToBcd(0x01));//��־д1
    DS1302_WriteByte(WRITE_CONTROL_REG, 0x80); /* ����д���� */
	DS1302_DateSet(date);
	return 0;
}

