
/*************************************************************************
 * Copyright (c) 2022,  WU JINCHANG
 * All rights reserved.
 * 
 * File name    :   MAIN.c
 * Brief        :   Health_watchr code.
 *               Introduce the main function or content of this document briefly.
 * Revision     :   1.1
 * Author       :   WU JINCHANG
 * Date         :   2022.03.8
 * Update       :   Itroduce the difference from previous version.
 * Website      :		http://hinuohui.com/
*************************************************************************/

#include "stm32f10x.h"
#include "OLED_I2C.h"
#include "ds1302.h"
#include "ds18b20.h"
#include "usart1.h"
#include "led.h"
#include "delay.h"
#include "algorithm.h"
#include "max30102.h"
#include "myiic.h"
#include "key.h"
#include "iic.h"
#include "stdio.h"
#include "string.h"
#include "stmflash.h"
#include "adxl345.h"

#define FLASH_SAVE_ADDR  ((u32)0x0800F000) 				//设置FLASH 保存地址(必须为偶数)

unsigned char setn=0;
unsigned char  p_r=0;		 						//平年/润年  =0表示平年，=1表示润年
float adx,ady,adz;
float acc;
u8 flag=0;
u16 bushu=0;
short temperature=0;
char display[16];

#define MAX_BRIGHTNESS 255

uint32_t aun_ir_buffer[150]; //infrared LED sensor data
uint32_t aun_red_buffer[150];  //red LED sensor data
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

int32_t hr_buf[16];
int32_t hrSum;
int32_t hrAvg;//心率
int32_t spo2_buf[16];
int32_t spo2Sum;
int32_t spo2Avg;//血氧浓度
int32_t spo2BuffFilled;
int32_t hrBuffFilled;
int32_t hrValidCnt = 0;
int32_t spo2ValidCnt = 0;
int32_t hrThrowOutSamp = 0;
int32_t spo2ThrowOutSamp = 0;
int32_t spo2Timeout = 0;
int32_t hrTimeout = 0;
uint32_t un_min, un_max,un_prev_data;
uint32_t  un_brightness;  //variables to calculate the on-board LED brightness that reflects the heartbeats

void Init_MAX30102(void)
{
    int32_t i;

    un_brightness = 0;
    un_min = 0x3FFFF;
    un_max = 0;
    
	  bsp_InitI2C();//IIC初始化
	  maxim_max30102_reset(); //resets the MAX30102
    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy); //Reads/clears the interrupt status register
    maxim_max30102_init();  //initialize the MAX30102
	
    n_ir_buffer_length = 150; //buffer length of 150 stores 3 seconds of samples running at 50sps

    //read the first 150 samples, and determine the signal range
    for(i = 0; i < n_ir_buffer_length; i++)
    {
        //while(KEY0 == 1); //wait until the interrupt pin asserts
        maxim_max30102_read_fifo((aun_ir_buffer+i), (aun_red_buffer+i));  //新版本 //read from MAX30102 FIFO

        if(un_min > aun_red_buffer[i])
            un_min = aun_red_buffer[i]; //update signal min
        if(un_max < aun_red_buffer[i])
            un_max = aun_red_buffer[i]; //update signal max
    }
    un_prev_data = aun_red_buffer[i];
    //calculate heart rate and SpO2 after first 150 samples (first 3 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
}

void GetHeartRateSpO2(void)
{
	  int32_t i;
	  float f_temp;
	  static u8 COUNT=8;
	  unsigned char x=0;
	
		i = 0;
		un_min = 0x3FFFF;
		un_max = 0;

		//dumping the first 50 sets of samples in the memory and shift the last 100 sets of samples to the top
		for(i = 50; i < 150; i++)
		{
				aun_red_buffer[i - 50] = aun_red_buffer[i];
				aun_ir_buffer[i - 50] = aun_ir_buffer[i];

				//update the signal min and max
				if(un_min > aun_red_buffer[i])
						un_min = aun_red_buffer[i];
				if(un_max < aun_red_buffer[i])
						un_max = aun_red_buffer[i];
		}

		//take 50 sets of samples before calculating the heart rate.
		for(i = 100; i < 150; i++)
		{
				un_prev_data = aun_red_buffer[i - 1];
				maxim_max30102_read_fifo((aun_ir_buffer+i), (aun_red_buffer+i));  //新版本

				//calculate the brightness of the LED
				if(aun_red_buffer[i] > un_prev_data)
				{
						f_temp = aun_red_buffer[i] - un_prev_data;
						f_temp /= (un_max - un_min);
						f_temp *= MAX_BRIGHTNESS;
						f_temp = un_brightness - f_temp;
						if(f_temp < 0)
								un_brightness = 0;
						else
								un_brightness = (int)f_temp;
				}
				else
				{
						f_temp = un_prev_data - aun_red_buffer[i];
						f_temp /= (un_max - un_min);
						f_temp *= MAX_BRIGHTNESS;
						un_brightness += (int)f_temp;
						if(un_brightness > MAX_BRIGHTNESS)
								un_brightness = MAX_BRIGHTNESS;
				}
		}
		maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		if(COUNT++ > 8)
		{
					COUNT = 0;
					temperature = DS18B20_Get_Temp();//读取温度
					if ((ch_hr_valid == 1) && (n_heart_rate < 150) && (n_heart_rate > 60))
					{
							hrTimeout = 0;

							// Throw out up to 1 out of every 5 valid samples if wacky
							if (hrValidCnt == 4)
							{
									hrThrowOutSamp = 1;
									hrValidCnt = 0;
									for (i = 12; i < 16; i++)
									{
											if (n_heart_rate < hr_buf[i] + 10)
											{
													hrThrowOutSamp = 0;
													hrValidCnt   = 4;
											}
									}
							}
							else
							{
									hrValidCnt = hrValidCnt + 1;
							}

							if (hrThrowOutSamp == 0)
							{

									// Shift New Sample into buffer
									for(i = 0; i < 15; i++)
									{
											hr_buf[i] = hr_buf[i + 1];
									}
									hr_buf[15] = n_heart_rate;

									// Update buffer fill value
									if (hrBuffFilled < 16)
									{
											hrBuffFilled = hrBuffFilled + 1;
									}

									// Take moving average
									hrSum = 0;
									if (hrBuffFilled < 2)
									{
											hrAvg = 0;
									}
									else if (hrBuffFilled < 4)
									{
											for(i = 14; i < 16; i++)
											{
													hrSum = hrSum + hr_buf[i];
											}
											hrAvg = hrSum >> 1;
									}
									else if (hrBuffFilled < 8)
									{
											for(i = 12; i < 16; i++)
											{
													hrSum = hrSum + hr_buf[i];
											}
											hrAvg = hrSum >> 2;
									}
									else if (hrBuffFilled < 16)
									{
											for(i = 8; i < 16; i++)
											{
													hrSum = hrSum + hr_buf[i];
											}
											hrAvg = hrSum >> 3;
									}
									else
									{
											for(i = 0; i < 16; i++)
											{
													hrSum = hrSum + hr_buf[i];
											}
											hrAvg = hrSum >> 4;
									}
							}
							hrThrowOutSamp = 0;
					}
					else
					{
							hrValidCnt = 0;
							if (hrTimeout == 4)
							{
									hrAvg = 0;
									hrBuffFilled = 0;
							}
							else
							{
									hrTimeout++;
							}
					}

					if ((ch_spo2_valid == 1) && (n_spo2 > 80))
					{
							spo2Timeout = 0;

							// Throw out up to 1 out of every 5 valid samples if wacky
							if (spo2ValidCnt == 4)
							{
									spo2ThrowOutSamp = 1;
									spo2ValidCnt = 0;
									for (i = 12; i < 16; i++)
									{
											if (n_spo2 > spo2_buf[i] - 10)
											{
													spo2ThrowOutSamp = 0;
													spo2ValidCnt   = 4;
											}
									}
							}
							else
							{
									spo2ValidCnt = spo2ValidCnt + 1;
							}

							if (spo2ThrowOutSamp == 0)
							{

									// Shift New Sample into buffer
									for(i = 0; i < 15; i++)
									{
											spo2_buf[i] = spo2_buf[i + 1];
									}
									spo2_buf[15] = n_spo2;

									// Update buffer fill value
									if (spo2BuffFilled < 16)
									{
											spo2BuffFilled = spo2BuffFilled + 1;
									}

									// Take moving average
									spo2Sum = 0;
									if (spo2BuffFilled < 2)
									{
											spo2Avg = 0;
									}
									else if (spo2BuffFilled < 4)
									{
											for(i = 14; i < 16; i++)
											{
													spo2Sum = spo2Sum + spo2_buf[i];
											}
											spo2Avg = spo2Sum >> 1;
									}
									else if (spo2BuffFilled < 8)
									{
											for(i = 12; i < 16; i++)
											{
													spo2Sum = spo2Sum + spo2_buf[i];
											}
											spo2Avg = spo2Sum >> 2;
									}
									else if (spo2BuffFilled < 16)
									{
											for(i = 8; i < 16; i++)
									 		{
													spo2Sum = spo2Sum + spo2_buf[i];
											}
											spo2Avg = spo2Sum >> 3;
									}
									else
									{
											for(i = 0; i < 16; i++)
											{
													spo2Sum = spo2Sum + spo2_buf[i];
											}
											spo2Avg = spo2Sum >> 4;
									}
							}
							spo2ThrowOutSamp = 0;
					}
					else
					{
							spo2ValidCnt = 0;
							if (spo2Timeout == 4)
							{
									spo2Avg = 0;
									spo2BuffFilled = 0;
							}
							else
							{
									spo2Timeout++;
							}
					}
					//显示相关的信息
					OLED_ShowChar((x++)*8,6,hrAvg%1000/100+'0',2,0);
					OLED_ShowChar((x++)*8,6,hrAvg%100/10+'0',2,0);
					OLED_ShowChar((x++)*8,6,hrAvg%10+'0',2,0);
          x=6;
					OLED_ShowChar((x++)*8,6,spo2Avg%1000/100+'0',2,0);
					OLED_ShowChar((x++)*8,6,spo2Avg%100/10+'0',2,0);
					OLED_ShowChar((x++)*8,6,spo2Avg%10+'0',2,0);
		}
}

void DisplayTime(void)//显示时间函数
{
	  unsigned char i=0,x=0;
	  u16 nian_temp;
	
	  if(setn==0)DS1302_DateRead(&SysDate);//读时间
	  nian_temp=2000+SysDate.year;
		if((nian_temp%400==0)||((nian_temp%100!=0)&&(nian_temp%4==0)))  //判断是否为闰年
				p_r=1;
		else
				p_r=0;
	
	  OLED_ShowChar((x++)*8,0,'2',2,setn+1-1);
	  OLED_ShowChar((x++)*8,0,'0',2,setn+1-1);
	  OLED_ShowChar((x++)*8,0,SysDate.year/10+'0',2,setn+1-1);
	  OLED_ShowChar((x++)*8,0,SysDate.year%10+'0',2,setn+1-1);
	  OLED_ShowChar((x++)*8,0,'-',2,0);
	  OLED_ShowChar((x++)*8,0,SysDate.mon/10+'0',2,setn+1-2);
	  OLED_ShowChar((x++)*8,0,SysDate.mon%10+'0',2,setn+1-2);
	  OLED_ShowChar((x++)*8,0,'-',2,0);
	  OLED_ShowChar((x++)*8,0,SysDate.day/10+'0',2,setn+1-3);
	  OLED_ShowChar((x++)*8,0,SysDate.day%10+'0',2,setn+1-3);
	  
	  OLED_ShowCN(i*16+88,0,0,setn+1-4);//测试显示中文：周
	  switch(SysDate.week)
    {
    case 1: 
			  OLED_ShowCN(i*16+104,0,1,setn+1-4);//测试显示中文：一
        break;

    case 2: 
			  OLED_ShowCN(i*16+104,0,2,setn+1-4);//测试显示中文：二
        break;

    case 3: 
			  OLED_ShowCN(i*16+104,0,3,setn+1-4);//测试显示中文：三
        break;

    case 4: 
			  OLED_ShowCN(i*16+104,0,4,setn+1-4);//测试显示中文：四
        break;

    case 5: 
			  OLED_ShowCN(i*16+104,0,i+5,setn+1-4);//测试显示中文：五
        break;

    case 6: 
			  OLED_ShowCN(i*16+104,0,6,setn+1-4);//测试显示中文：六
        break;

    case 7: 
			  OLED_ShowCN(i*16+104,0,7,setn+1-4);//测试显示中文：日
        break;
    }
    x=0;
	  OLED_ShowChar((x++)*8,2,SysDate.hour/10+'0',2,setn+1-5);
	  OLED_ShowChar((x++)*8,2,SysDate.hour%10+'0',2,setn+1-5);
	  OLED_ShowChar((x++)*8,2,':',2,0);
	  OLED_ShowChar((x++)*8,2,SysDate.min/10+'0',2,setn+1-6);
	  OLED_ShowChar((x++)*8,2,SysDate.min%10+'0',2,setn+1-6);
	  OLED_ShowChar((x++)*8,2,':',2,0);
	  OLED_ShowChar((x++)*8,2,SysDate.sec/10+'0',2,setn+1-7);
	  OLED_ShowChar((x++)*8,2,SysDate.sec%10+'0',2,setn+1-7);
}

void DisplayTemperature(void)//显示温度函数
{
	  unsigned char x=10;//显示的第几个字符
		
	  temperature=DS18B20_Get_Temp();
		OLED_ShowChar((x++)*8,2,temperature/100+'0',2,0);
		OLED_ShowChar((x++)*8,2,temperature%100/10+'0',2,0);
		OLED_ShowChar((x++)*8,2,'.',2,0);
		OLED_ShowChar((x++)*8,2,temperature%10+'0',2,0);
		OLED_ShowChar((x++)*8,2,'C',2,0);
}

void CHECK_NEW_MCU(void)  // 检查是否是新的单片机，是的话清空存储区，否则保留
{
	  u8 comper_str[6];
		
	  STMFLASH_Read(FLASH_SAVE_ADDR + 0x10,(u16*)comper_str,5); //从0X08010010这个地址读出数据 
	  comper_str[5] = '\0';
	  if(strstr(comper_str,"FDYDZ") == NULL)  //新的单片机
		{
			 STMFLASH_Write(FLASH_SAVE_ADDR + 0x10,(u16*)"FDYDZ",5); //写入“FDYDZ”，方便下次校验
			 STMFLASH_Write(FLASH_SAVE_ADDR + 0x20,&bushu,1);
	  }
		DelayMs(100);
		STMFLASH_Read(FLASH_SAVE_ADDR + 0x20,&bushu,1);//读取存储的步数
    if(bushu>60000||bushu<0)bushu=0;
}

void GetSteps(void)//获取步数函数
{
	  static u16 temp=0;
	  u16 x=11; 
	
	  adxl345_read_average(&adx,&ady,&adz,10);//获取数据
		acc=adx;
		if(acc>0)//在正轴
		{
			if(acc/10>5&&flag==1)//加速度值，在正轴值是否大于5，并且flag为1，则视为一个周期完成，步数加1
			{
				flag=0;	
				if(bushu<60000)bushu++;	//步数加1
				if(temp!=bushu)//当步数发生变化才去存储步数
				{
						temp=bushu;
					  STMFLASH_Write(FLASH_SAVE_ADDR + 0x20,&bushu,1); //存储步数
				}
			}
		}
		if(acc<0)//在负轴
		{
			acc=-acc;
			if(acc/10>=5)//加速度值，在负轴是否小于-5
			{
					flag=1;//falg置1
			}
		}
		if(bushu>9999)
		{
				OLED_ShowChar((x++)*8,6,bushu/10000+'0',2,0);
				OLED_ShowChar((x++)*8,6,bushu%10000/1000+'0',2,0);
				OLED_ShowChar((x++)*8,6,bushu%1000/100+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%100/10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%10+'0',2,0);
		}
		else if(bushu>999)
		{
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,bushu%10000/1000+'0',2,0);
				OLED_ShowChar((x++)*8,6,bushu%1000/100+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%100/10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%10+'0',2,0);
		}
		else if(bushu>99)
		{
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,bushu%1000/100+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%100/10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%10+'0',2,0);
		}
		else if(bushu>9)
		{
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,bushu%100/10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,bushu%10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,' ',2,0);
		}
		else
		{
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,' ',2,0);
				OLED_ShowChar((x++)*8,6,bushu%10+'0',2,0);
			  OLED_ShowChar((x++)*8,6,' ',2,0);
			  OLED_ShowChar((x++)*8,6,' ',2,0);
		}
}

void KeySettings(void)//按键设置函数
{
	  unsigned char keynum = 0;
	
	  keynum = KEY_Scan(0);//获取按键值
		if(keynum==1)//设置
		{
				setn++;
				if(setn > 7)setn=0;
		}
		if(keynum==2)//加
		{
				if(setn == 1)//设置年
				{
						SysDate.year ++;
						if(SysDate.year == 100)SysDate.year=0;
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 2)//设置月
				{
						SysDate.mon ++;
						if(SysDate.mon == 13)SysDate.mon = 1;
						if((SysDate.mon==4)||(SysDate.mon==6)||(SysDate.mon==9)||(SysDate.mon==11))
						{
								if(SysDate.day>30)SysDate.day=1;
						}
						else
						{
								if(SysDate.mon==2)
								{
										if(p_r==1)
										{
												if(SysDate.day>29)
														SysDate.day=1;
										}
										else
										{
												if(SysDate.day>28)
														SysDate.day=1;
										}
								}
						}
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 3)//设置日
				{
						SysDate.day ++;
						if((SysDate.mon==1)||(SysDate.mon==3)||(SysDate.mon==5)||(SysDate.mon==7)||(SysDate.mon==8)||(SysDate.mon==10)||(SysDate.mon==12))//大月
						{
								if(SysDate.day==32)//最大31天
										SysDate.day=1;//从1开始
						}
						else
						{
								if(SysDate.mon==2)//二月份
								{
										if(p_r==1)//闰年
										{
												if(SysDate.day==30)//最大29天
														SysDate.day=1;
										}
										else
										{
												if(SysDate.day==29)//最大28天
														SysDate.day=1;
										}
								}
								else
								{
										if(SysDate.day==31)//最大30天
												SysDate.day=1;
								}
						}
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 4)//设置星期
				{
						SysDate.week ++;
						if(SysDate.week == 8)SysDate.week=1;
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 5)//设置时
				{
						SysDate.hour ++;
						if(SysDate.hour == 24)SysDate.hour=0;
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 6)//设置分
				{
						SysDate.min ++;
						if(SysDate.min == 60)SysDate.min=0;
						DS1302_DateSet(&SysDate);//设置时间
				}
				if(setn == 7)//设置秒
				{
						SysDate.sec ++;
						if(SysDate.sec == 60)SysDate.sec=0;
						DS1302_DateSet(&SysDate);//设置时间
				}
		}
		if(keynum==3)//减
		{
				if(setn == 1)//设置年
				{
						if(SysDate.year == 0)SysDate.year=100;
						SysDate.year --;
						DS1302_DateSet(&SysDate);
				}
				if(setn == 2)//设置月
				{
						if(SysDate.mon == 1)SysDate.mon=13;
						SysDate.mon --;
						if((SysDate.mon==4)||(SysDate.mon==6)||(SysDate.mon==9)||(SysDate.mon==11))
						{
								if(SysDate.day>30)
										SysDate.day=1;
						}
						else
						{
								if(SysDate.mon==2)
								{
										if(p_r==1)
										{
												if(SysDate.day>29)
														SysDate.day=1;
										}
										else
										{
												if(SysDate.day>28)
														SysDate.day=1;
										}
								}
						}
						DS1302_DateSet(&SysDate);
				}
				if(setn == 3)//设置日
				{
						SysDate.day --;
						if((SysDate.mon==1)||(SysDate.mon==3)||(SysDate.mon==5)||(SysDate.mon==7)||(SysDate.mon==8)||(SysDate.mon==10)||(SysDate.mon==12))
						{
								if(SysDate.day==0)
										SysDate.day=31;
						}
						else
						{
								if(SysDate.mon==2)
								{
										if(p_r==1)
										{
												if(SysDate.day==0)
														SysDate.day=29;
										}
										else
										{
												if(SysDate.day==0)
														SysDate.day=28;
										}
								}
								else
								{
										if(SysDate.day==0)
												SysDate.day=30;
								}
						}
						DS1302_DateSet(&SysDate);
				}
				if(setn == 4)//设置星期
				{
						if(SysDate.week == 1)SysDate.week=8;
						SysDate.week --;
						DS1302_DateSet(&SysDate);
				}
				if(setn == 5)//设置时
				{
						if(SysDate.hour == 0)SysDate.hour=24;
						SysDate.hour --;
						DS1302_DateSet(&SysDate);
				}
				if(setn == 6)//设置分
				{
						if(SysDate.min == 0)SysDate.min=60;
						SysDate.min --;
						DS1302_DateSet(&SysDate);
				}
				if(setn == 7)//设置秒
				{
						if(SysDate.sec == 0)SysDate.sec=60;
						SysDate.sec --;
						DS1302_DateSet(&SysDate);
				}
		}
		if(keynum==4)//步数清零
		{
			 bushu = 0;
			 STMFLASH_Write(FLASH_SAVE_ADDR + 0x20,&bushu,1);
			 DelayMs(50); 
		}
}

void UsartSendData(void)//串口发送数据
{
	  static u8 flag1=0,flag2=1;
	  float temp=0.0;  
	
	  if(flag1!=SysDate.sec)
		{
			  flag1 = SysDate.sec;
			  flag2 = !flag2;
			  if(flag2==1)//2秒发送一次数据
				{
						temp=(float)temperature/10; 
						printf("体温:%4.1fC  \r\n",temp);         //发送体温
						DelayMs(10); 
						printf("心率:%d  \r\n",hrAvg);     //发送心率
						DelayMs(10); 
					  printf("血氧:%d  \r\n",spo2Avg);     //发送血氧
						DelayMs(10); 
						printf("步数:%d     \r\n",bushu);     //发送步数
						DelayMs(10);
						printf("\r\n");
				}
		}
}

int main(void)
{
	char i;
	
	DelayInit();
	I2C_Configuration(); //IIC初始化
	OLED_Init(); //OLED初始化
	KEY_Init(); //按键初始化
	DelayMs(200);
	CHECK_NEW_MCU();
  OLED_CLS();//清屏
	DS18B20_Init();
	DS1302_Init(&SysDate);
	DelayMs(100); 
	DS1302_DateRead(&SysDate);//读时间
	for(i=0;i<8;i++)OLED_ShowCN(i*16,2,i+8,0);//测试显示中文：欢迎使用使能手环
	DelayMs(1000);DelayMs(1000);
	OLED_CLS();//清屏
	for(i=0;i<2;i++)OLED_ShowCN(i*16,4,i+16,0);//测试显示中文：心率
	for(i=0;i<2;i++)OLED_ShowCN(i*16+48,4,i+18,0);//测试显示中文：血氧
	for(i=0;i<2;i++)OLED_ShowCN(i*16+95,4,i+20,0);//测试显示中文：步数
	IIC_init();//IIC初始化
	adxl345_init();//ADXL345初始化
	Init_MAX30102();//MAX30102初始化
	uart1_Init(9600);
	
	while(1)
	{
		  KeySettings();
	    DisplayTime();
		  if(setn == 0)//不在设置状态下，读取相关数据
			{
					DisplayTemperature();
					GetSteps();
					GetHeartRateSpO2();
			}
		  UsartSendData();
	}
}
