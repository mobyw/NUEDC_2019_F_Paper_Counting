#include "fdc2214.h"
#include "usart.h"
#include "delay.h"
#include "oled.h"
#include "bmp.h"
#include "sys.h"
#include "key.h"

u16 data0[30];

void blink(void);

int main(void)
{
    //float res [4];
    char  disp[20];
    u32   org [4];
    u8    flag = 0;
    u8    index = 0;
    
    delay_init();
    //KEY_Init();
    uart_init(115200);       // 串口 1 波特率 112500
    NVIC_Configuration();
    
	OLED_Init();
    OLED_ColorTurn(0);       // 0 正常显示 1 反色显示
    OLED_DisplayTurn(0);     // 0 正常显示 1 屏幕翻转显示
    OLED_Refresh();

    while(FDC2214_Init())
    {
        delay_ms(50);
    }

    blink();
    delay_ms(1000);

    for(index = 0; index < 11; index++)
    {
        OLED_Clear();
        sprintf((char*)disp, "Put on %d paper(s)", index);
        OLED_ShowString (12, 2, (u8*)disp, 12);
        OLED_ShowString (12, 20, (u8*)"Then press button", 12);

        OLED_Refresh();

        while(KEY_Read())
        {
            delay_ms(10);
        }

        delay_ms(1000);
        
        org[0] = FCD2214_ReadCH(0);
        org[1] = FCD2214_ReadCH(1);

        org[0] = org[0] + org[1];
        data0[index] = org[0] / 2;

        blink();

        printf("Paper: %d, value: %d \r\n", index, data0[index]);
    }
    
    while(1)
    {
        if(!flag)
        {
            while(FDC2214_Init())
            {
                delay_ms(50);
            }

            org[0] = FCD2214_ReadCH(0);
            org[1] = FCD2214_ReadCH(1);
            printf("DATA: \r\n ch0: %d, ch1: %d \r\n", org[0], org[1]);
            
            blink();
            OLED_Clear();
            
            sprintf((char*)disp, "ch0:%d", org[0]);
            OLED_ShowString (30, 36, (u8*)disp, 12);
            sprintf((char*)disp, "ch1:%d", org[1]);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            
            OLED_Refresh();
            LED_OFF;
        }
        else
        {
            OLED_Clear();
            sprintf((char*)disp, "ch0:%d", org[0]);
            OLED_ShowString (30, 36, (u8*)disp, 12);
            sprintf((char*)disp, "ch1:%d", org[1]);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            OLED_Refresh();
        }

        flag = KEY_Read();
        delay_ms(100);
    }
}

void blink(void)
{
    uint8_t i = 3;
    do{
        delay_ms(200);  LED_ON;
        delay_ms(200);  LED_OFF;
    } while(i--);
}
