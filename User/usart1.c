#include <stm32f10x.h>
#include <stdio.h>
#include "usart1.h"

char Usart1RecBuf[USART1_RXBUFF_SIZE];//串口1接收数据缓存
unsigned int RxCounter = 0;   //串口1收到数据标志位

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}

#endif



/****************************************************************************

描述：Usart1 初始化 
   PA9： USART1_TX
   PA10: USART1_RX

参数：none

返回: none

****************************************************************************/

void uart1_Init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	
	USART_DeInit(USART1);  
 
     //USART1_TX   PA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART1_RX	  PA.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

   //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		

  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//IRQ通道使能
  NVIC_Init(&NVIC_InitStructure);	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器USART1
  
   //USART 初始化设置
  USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
   

  USART_Init(USART1, &USART_InitStructure);     //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口1接受中断
  USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);//使能串口1总线空闲中断/
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断，可以扩展来控制
		{
			  if(RxCounter >= USART1_RXBUFF_SIZE) RxCounter = 0;
				Usart1RecBuf[RxCounter++] =USART_ReceiveData(USART1);//接收模块的数据
    } 
		 USART_ClearITPendingBit(USART1, USART_IT_RXNE);
} 

void uart1_SendStr(char*SendBuf)//串口1打印数据
{
	while(*SendBuf)
	{
	  while((USART1->SR&0X40)==0);//等待发送完成 
    USART1->DR = (u8) *SendBuf; 
		SendBuf++;
	}
}

void uart1_send(unsigned char *bufs,unsigned char len)
{
		while (len--)
		{
				while((USART1->SR&0X40)==0);//等待发送完成 
				USART1->DR = (u8) *bufs; 
				bufs ++;
		}
}


