#ifndef __IIC_H
#define __IIC_H
#include "sys.h"

#define IIC_SCL    PAout(6) //SCL
#define IIC_SDA    PAout(7) //SDA	 
#define READ_SDA   PAin(7)  // ‰»ÎSDA 

void I2C_SDA_OUT(void);
void I2C_SDA_IN(void);
void IIC_init(void);
void IIC_start(void);
void IIC_stop(void);
void IIC_ack(void);
void IIC_noack(void);
u8 IIC_wait_ack(void);
void IIC_send_byte(u8 txd);
u8 IIC_read_byte(u8 ack);
#endif
