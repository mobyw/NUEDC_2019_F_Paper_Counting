#include "key.h"
#include "delay.h"

// Initial key input
void Key_Init(void) 
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(KEY_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = KEY_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
	GPIO_SetBits(KEY_PORT, GPIO_Pin_8);
}

// Read key input
// 0: pressed
// 1: not pressed
// Return when release
u8 Key_Read(void)
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
