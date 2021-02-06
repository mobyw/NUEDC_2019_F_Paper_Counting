#include "key.h"
#include "delay.h"

void Key_Init(void) 
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

u8 KEY_Read(void)
{
    if(!KEY)
    {
        delay_ms(10);
        if(!KEY)
        {
            while(!KEY){delay_ms(10);};
            return 0;
        }
    }

    return 1;
}
