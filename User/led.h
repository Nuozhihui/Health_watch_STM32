#ifndef __LED_H
#define	__LED_H

#include "stm32f10x.h"
#include "sys.h"

/* the macro definition to trigger the led on or off 
 * 1 - off
 - 0 - on
 */
#define ON  1
#define OFF 0

#define BEEP PBout(11) 
#define LED PCout(13) 
	
void BEEP_GPIO_Config(void);

#endif /* __LED_H */
