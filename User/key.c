#include "key.h"
#include "delay.h"
 	    
//������ʼ������ 
//PA15��PC5 ���ó�����
void KEY_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTA,PORTCʱ��

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
} 
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==0))
	{
		DelayMs(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(KEY3==0)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1)key_up=1; 	     
	return 0;// �ް�������
}
