#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

#define KEY_PORT GPIOA
#define KEY_PIN  GPIO_Pin_8
#define KEY_CLK  RCC_APB2Periph_GPIOA

#define KEY PAin(8)

void Key_Init(void);
u8   Key_Read(void);

#endif
