# Health_watch_STM32
 健康手表

 ### ADXL345三轴传感器
本次设计采用的是ADXL345三轴传感器来检测当前行走的步数，ADXL345有2种通信方式，我们采用的是IIC进行数据传输。首先我们需要在程序里面配置IIC数据传输的引脚分别为SCL对应A6 ,SDL对应A7，然后对ADXL345进行初始化Init_ADXL345()；写入开始信号和停止信号。void ADXL345_Start()；void ADXL345_Stop()；接收应答信号：bit ADXL345_RecvACK()，ADXL345开始读取采集到的数据BYTE ADXL345_RecvByte()；读取ADXL345的加速度值，从而判断是否发生位移void Multiple_read_ADXL345(void)；如果读取到的数据为0XE5表示发生位移行走。devid=Single_Read_ADXL345(0X00); if(devid!=0XE5)

 ### DS18B20内部结构
主要由4部分组成：64 位ROM、温度传感器、非挥发的温度报警触发器TH和TL、配置寄存器。ROM中的64位序列号是出厂前被光刻好的，它可以看作是该DS18B20的地址序列码，每个DS18B20的64位序列号均不相同。64位ROM的排的循环冗余校验码（CRC=X^8＋X^5＋X^4＋1）。 ROM的作用是使每一个DS18B20都各不相同，这样就可以实现一根总线上挂接多个DS18B20的目的。
    
开启开关电源后，温度传感器开始工作DS18B20先进性初始化WENDU(int temperature)； 
程序按数据手册的时序图编写子函数模块：
1 ：DS18B20 复位函数：resetDS18B20（void0）
2：写一位的函数：  WriteBit (unsigned char wb)
3读一位的函数：  unsigned char ReadBit (void)   
4：读一个字节的函数unsigned char readByteDS18B20(void) 即将位读取的时序循环  8 次。    
5：写一个字节的函数：void writeByteDS18B20(unsigned char Data) 。即将位写入的时序循环  8  次。
  ### MAX30102芯片对人体心率血氧
  本次设计采用的是MAX30102芯片对人体心率血氧进行采集，MAX30102我们采用的是IIC进行通信首先我们需要在程序里面配置IIC数据传输的引脚分别进行配置，然后对IIC进行初始化bsp_InitI2C()； MAX30102写寄存器函数maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data);  MAX30102读寄存器函数maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data); MAX30102初始化maxim_max30102_init(); MAX30102读缓冲器FIFO maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led); 计算心率和血氧饱和度，通过检测PPG (photoplethysmographic,光电容积脉搏波描记法)周期的峰值和相应的红/红外信号的AC/DC，计算出SPO2的an/u比值和心率值
maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid,int32_t *pn_heart_rate, int8_t *pch_hr_valid)；

   ### OLED
   此次设计我们采用的是单片机的内部iic，所以我们需要进行配置才可以正常使用。我们结合iic时序图以及需要显示的内容进行处理。I2C 总线是主从结构，单片机是主器件，存储器是从器件。一条总线可以带多个从器件（ 也可以有多主结构），I2C 总线的SDA 和SCL 是双向的，开路门结构，通过上拉电阻接正电源。进行数据传输时，SDA 线上的数据必须在时钟的高电平周期保持稳定。数据线的高或低电平状态只有在SCL 线的时钟信号是低电平时才能改变.
   首先stm32单片机进行配置引脚STM32F103C8T6  引脚I2C: PB6 -- SCL; PB7 -- SDA ，GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; I2C引脚必须要开漏输出，I2C_DeInit(I2C1);//使用IIC引脚。void I2C_WriteByte(uint8_t addr,uint8_t data)iic进行写入数据信息I2C_GenerateSTART(I2C1, ENABLE);开启I2C1   I2C_SendData(I2C1, addr);寄存器地址I2C_SendData(I2C1, data);发送数据I2C_GenerateSTOP(I2C1, ENABLE);关闭I2C1总线  void WriteCmd(unsigned char I2C_Command)写命令void WriteDat(unsigned char I2C_Data)//写数据void OLED_Init(void) IIC初始化，显示当前数据
   ###  蓝牙串口
   为蓝牙串口模块，ATK-HC05模块是主从一体的，而且性能较高，可以与PDA、手机、电脑等具有蓝牙功能的设备实现配对，该模块所支持的波特率范围非常大，为4800~1382400，而且该模块和3.3V或5V的单片机系统相兼容，极其方便、灵活。
在建立微微网前，一切设备都是就绪状态，在此状态中，每1.28s未连接的设备便会对一次消息进行监听，唤醒设备后，在监听信息时可以预设32个调频频率，主设备初始化后实现连接进程。倘若已知设备的地址，通过页信息实现连接；倘若地址是未知的，那么通过页信息的查询信息实现连接。微微网中，如果设备未进行数据传输，那么它便进入节能状态。主设备设置从设备为保持方式，此过程中处于工作状态的只有内部定时器；从设备亦可以进入到保持方式。设备一旦转出保持方式便可以开始数据传输。在和几个微微网相连，或者对低功耗器件进行管理时，常用保持方式。低功耗的另外两种工作方式是休眠、监听方式。鉴于蓝牙基带技术，其支持面向连接方式、无连接方式，前者传输语音，后者传输分组数据，温度数据就通过无连接方式传输。 
蓝牙采用的是跳频和时分多址技术。为了使频谱扩展，需要利用伪随机码序列实现频移键控，此载波频率发生跳变，即为跳频。传统通信系统中使用定频方式，在发射机中，主振荡器具有固定的振荡频率，为了实现载波频率的跳变，得到跳频信号，按照控制指令改变主振荡器的频率。能够得到跳频信号的装置被称作跳频器，它主要包括跳频指令发生器、频率合成器。如果将跳频器等同于主振荡器，那么和传统的发信机没有任何不同。可以对模拟、数字形式的信号进行传送，之后利用调制器实现调制，进而得到固定频率的已调波信号，接着和频率合成器的主载波频率信号实现混频，此时输出载波频率符合射频通带要求的已调波信号，在经过高通滤波器反馈后，利用天线将信号发射出去，此过程即为发送定频信号的过程。时分多址是分割时间为不重叠的帧，再分割帧为不重叠的信道，和用户一一对应，主要利用信道对地质不同的信号进行区分，实现多址连接。
   ###  蓝牙串口