#ifndef __ADXL345_H
#define __ADXL345_H
#include "sys.h"
/************************************************
		ADXL345���ּ��ٶ�˵���ļ�
����    : ADXL345���ּ��ٶ�.h�ļ�
����    : ���������Լ���ؽӿڵ�����
ռ����Դ: ��ADXL345���ּ��ٶȹ���ʱ,ռ��
          100%��CPU��Դ
Ӳ��˵��: STCϵ��1T��Ƭ��,���11.0592MHz����
������  : J.H.Chen
����ʱ��: 2013.08.29
˵��    : ��ϸ�Ķ����뿴����ʵ��
************************************************/

#define X_AXLE 0   //x��
#define Y_AXLE 1   //y��
#define Z_AXLE 2   //z��

#define	slaveaddress   0xA6	  //����������IIC�����еĴӵ�ַ,����ALT  ADDRESS��ַ���Ų�ͬ�޸�
//                              ALT  ADDRESS���Žӵ�ʱ��ַΪ0xA6���ӵ�Դʱ��ַΪ0x3A

#define THRESH_TAP     0X1D	  //�û��ж���ֵ(���������û����)
//                              16gģʽ��,62.5mg/Bit

#define OFSX           0X1E   //X��ƫ�ƼĴ���  15.6mg/Bit  0xff = 4g
#define OFSY           0X1F   //X��ƫ�ƼĴ���  15.6mg/Bit  0xff = 4g
#define OFSZ           0X20   //X��ƫ�ƼĴ���  15.6mg/Bit  0xff = 4g
#define DUR            0x21   //�û���ֵʱ��   625uS/Bit
#define LATENT         0X22   //�û��¼���ʱ�䴰�ڵ��ӳ�ʱ��,�ڴ��ڼ�
//                              �ɼ��ڶ����û�ʱ��  1.25mg/Bit
#define WINDOW         0X23   //�û�����,�ӳ�ʱ�������ʱ����,�ڴ��ڼ�
//                              �ܿ�ʼ���еڶ�����Ч�û�  1.25mg/Bit
#define THRESH_ACT     0X24   //�������ֵ,��¼��ķ�����üĴ���
//                              ��ֵ���бȽ�  62.5mg/Bit
#define	THRESH_INACT   0X25   //��⾲ֹ����ֵ,��ֹ�¼��ķ�����üĴ���
//                              ��ֵ���бȽ�  62.5mg/Bit
#define TIME_INACT     0X26   //���ٶ�ʱ����С�ڸüĴ�����ֵ��ʾ��ֹ 1S/Bit

#define ACT_INACT_CTL  0X27 
//Bit  7,3  ACT����/ֱ��[INACT����/ֱ��]:0ѡ��ֱ�����;1ʹ�ܽ������ ֱ��ʱ��
//                                       ��ǰ���ٶ�ֱֵ����THRESH_ACT��THRESH_INACT
//                                       ���бȽ�,ȷ����⵽���ǻ���Ǿ�ֹ
//     6,2  ACT_Xʹ��[INACT_Xʹ��]:����Ϊ1,ʹ��X���������ֹ;����ʱ,
//                                 ������Ϊ�߼�"��",�������ᳬ����ֵʱ,����ܴ���
//                                 ��ֹ���ʱ,������Ϊ�߼�"��",ֻ�е������������ֵ
//                                 ʱ,��ֹ���ܴ���
//     5,1  ACT_Yʹ��[INACT_Yʹ��]:��X������ 
//     4,0  ACT_Zʹ��[INACT_Zʹ��]:��X������
#define THRESH_FF      0X28   //��ֵ,��������������,��������ٶ���üĴ���ֵ�Ƚ�
//                              ,��ȷ���Ƿ����������� 62.5mg/Bit,����300mg��600mg֮��
#define TIME_FF        0X29   //ά��THRESH_FF��ֵ����Сʱ��,���������������ж� 5mS/Bit

#define TAP_AXES       0X2A   
// Bit  7:4  0
//       3   ����  �����û�֮����ִ���THRESH_TAPֵ�ü��ٶ�,�������ƻ�����˫�����
//       2   TAP_Xʹ��  ����Ϊ1ʱ,ʹ��X������û����,0ʱ�ų�������û����
//       1   TAP_Yʹ��  ����Ϊ1ʱ,ʹ��Y������û����,0ʱ�ų�������û����
//       0   TAP_Zʹ��  ����Ϊ1ʱ,ʹ��Z������û����,0ʱ�ų�������û����
#define ACT_TAP_STATUS 0X2B   /*ֻ���Ĵ���*/
// Bit  7   0
//     6,2  ACT_X��Դ,TAP_X��Դ:��ʾ�漰�û����¼��ĵ�һ��,����Ϊ1ʱ,��Ӧ�¼�����
//                              ����Ϊ0ʱ,��Ӧδ����.�����Զ�����,�����ݸ���,�ж�����ǰ
//                              Ӧ��ȡ�üĴ���
//     5,1	ACT_Y��Դ,TAP_Y��Դ:��X����
//     4,0	ACT_Z��Դ,TAP_Z��Դ:��X����
//      3   ��������Ϊ1ʱ,������������״̬
#define BW_RATE        0X2C  
// Bit  7:5   0
//       4	 LOW_POWER �͹���λ,0ѡ������ģʽ,1����͹���ģʽ
//      3:0  ����λ
#define POWER_CTL      0X2D
// Bit  7,6   0
//       5    ����   ����1ʱ,�ӳٻ��ʼ,ֱ����⵽��ֹ.��⵽���,��ֹ��⿪ʼ,�
//                   ���ֹͣ,����ʱ��̬���ӻ�;�ֹ������;����0ʱ��ֹ��ͬʱ���
//       4    AUTO_SLEEP  ����1ʱ�Զ�����,������ֹ��,��������ģʽ,�ʹ�ܺ󱻻���
//       3    ����   0���� 1����ģʽ
//       2	  ����   0��ͨ 1����
//      1,0	  ����(����ģʽ�µĶ�ȡƵ��) "00":8HZ  "01":4HZ  "10":2HZ  "11":1HZ
#define INT_ENABLE     0X2E  //�ж�ʹ������
// Bit  7  DATA_READY
//      6  SINGLE_TAP
//      5  DOUBLE_TAP
//      4  Activity
//      3  Inactivity
//      2  FREE_FALL   ���������ж�
//      1  Watermark
//      0  Overrun
#define INT_MAP        0X2F   //�ж�ӳ�� �Զ��Ĵ���
//λ��INT_ENABLE��Ӧ,,����Ϊ0,���ж�ӳ�䵽INT1����;����Ϊ1,���ж�ӳ�䵽INT2����
#define INT_SOURCE     0X30   //�ж���Դ
//λ��INT_ENABLE��Ӧ,1��ʾ�ù��ܴ���
#define DATA_FORMAT    0X31
// Bit   7  SELF_TEST  ����1,�Բ���Ӧ����������,����������ת��;0ʱ�����Բ���
//       6  SPI        1����Ϊ3��SPIģʽ,0ʱ����4��SPIģʽ
//       5  INT_INVERT 0ʱ�жϸߵ�ƽ��Ч,1ʱ�͵�ƽ��Ч
//       4	0
//       3  FULL_RES   1ʱ����ȫ�ֱ���ģʽ,�����4mg/Bit����;0ʱΪ10λģʽ
//       2  Justify    1Ϊ�����ģʽ;8Ϊ�Ҷ���ģʽ,�����з�����չ
//      1:0 ��Χλ     "00"��2g  "01"��4g  "10"��8g  "11"��16g
#define DATAX0         0X32
#define DATAX1         0X33  //��DATAX0���x���������(�����Ʋ���),DATAX1Ϊ��λ,4mg/Bit
#define DATAY0         0X34
#define DATAY1         0X35  //��DATAY0���Y���������(�����Ʋ���),DATAY1Ϊ��λ,4mg/Bit
#define DATAZ0         0X36
#define DATAZ1         0X37  //��DATAZ0���Z���������(�����Ʋ���),DATAZ1Ϊ��λ,4mg/Bit
#define FIFO_CTL       0X38
// Bit  7,6  FIFO_MODE  "00" ��·ģʽ
//						"01" FIFOģʽ ���ռ����32��ֵ,Ȼ��ֹͣ�ռ�����
//						"10" ��ģʽ   FIFO�������32������ֵ,FIFO��ʱ,�����ݸ�����������(�Ƚ��ȳ�)
//						"11" ������   ͨ������λ����,FIFO�ڴ����¼�ǰ����������������,Ȼ��
//									  �����ռ�����ֱ������;������,�����ռ�������
//		 5   ����λ     0���Ӵ�����ģʽ�µĴ����¼���INT1,1������INT2
//		 4:0 ����		����ȡ����FIFOģʽ:FIFOģʽʱ,ָ������ˮӡ�ж���Ҫ��FIFO��Ŀ��
//                                         ��ģʽʱ,ָ������ˮӡ�ж���Ҫ��FIFO��Ŀ��
//                                         ������ģʽ:ָ�������¼�֮ǰ��FIFO������Ҫ������FIFO������
//                      ����λ����Ϊ0ʱ,��������FIFOģʽ,������INT_SOURCE�Ĵ�������ˮӡ״̬λ
#define FIFO_STATUS    0X39	  /*ֻ���Ĵ���*/
// Bit   7   FIFO_TRIG  FIFO_TRIGΪ1ʱ��ʾ�д����¼����� 
//       6   0
//      5:0	 ��Ŀλ ����FIFO�洢������ֵ������

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
