#include "fdc2214.h"
#include "usart.h"
#include "delay.h"
#include "oled.h"
#include "bmp.h"
#include "sys.h"
#include "key.h"

#define MAXSIZE 5

// Calibration data
u32 data0[MAXSIZE];
u32 res;
u8  num;

void blink(void);
void sortout(void);
u8 recognize(u32 data);

int main(void)
{
    char  disp[20];         // Display buffer
    u32   org [4];          // Original data
    u8    flag = 0;         // Key press flag
    u8    index = 0;
    
    delay_init();
    Key_Init();
    uart_init(115200);      // Serial port 1 @ 112500
    NVIC_Configuration();
    
	OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Refresh();

    while(FDC2214_Init())
    {
        delay_ms(50);
    }

    blink();
    delay_ms(500);

    for(index = 0; index < MAXSIZE; index++)
    {
        sprintf((char*)disp, "Put on %d paper(s)", index);
        OLED_ShowString (12, 2, (u8*)disp, 12);
        OLED_ShowString (12, 20, (u8*)"Then press button", 12);

        OLED_Refresh();

        while(Key_Read())
        {
            delay_ms(10);
        }

        while(FDC2214_Init())
        {
            delay_ms(50);
        }

        delay_ms(500);
        
        org[0] = FCD2214_ReadCH(0);
        org[1] = FCD2214_ReadCH(1);

        res = (org[0] + org[1]) / 2;
        data0[index] = res;

        OLED_Clear();
        sprintf((char*)disp, "num%d: %d", index, res);
        OLED_ShowString (20, 50, (u8*)disp, 12);
        OLED_Refresh();

        blink();

        if(res == 0)
        {
            index--;
        }

        // printf("Paper: %d, value: %d \r\n", );
    }
    
    sortout();
    OLED_Clear();
    OLED_Refresh();
    
    while(1)
    {
        if(!flag)
        {
            do{
                while(FDC2214_Init())
                {
                    delay_ms(50);
                }

                org[0] = FCD2214_ReadCH(0);
                org[1] = FCD2214_ReadCH(1);
                printf("DATA: \r\n ch0: %d, ch1: %d \r\n", org[0], org[1]);

                res = (org[0] + org[1]) / 2;
            } while (res ==0);

            num = recognize(res);
            
            blink();
            OLED_Clear();
            
            sprintf((char*)disp, "Number: %d", num);
            OLED_ShowString (25,  6, (u8*)disp, 16);
            sprintf((char*)disp, "res: %d", res);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            
            OLED_Refresh();
            LED_OFF();
        }
        else
        {
            OLED_Clear();
            sprintf((char*)disp, "Number: %d", num);
            OLED_ShowString (25,  6, (u8*)disp, 16);
            sprintf((char*)disp, "res: %d", res);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            OLED_Refresh();
        }

        flag = Key_Read();
        delay_ms(100);
    }
}

// Led blink
void blink(void)
{
    uint8_t i = 3;
    do{
        delay_ms(200);  LED_ON();
        delay_ms(200);  LED_OFF();
    } while(i--);
}

void sortout(void)
{
    u8 index = 0;

    for(index = 1; index < MAXSIZE - 1; index++)
    {
        data0[index] = (data0[index] + data0[index + 1]) / 2;
    }
}

// Recognize paper number
// data: raw data
// return: paper number
u8 recognize(u32 data)
{
    u8 i = 0;

    if(data > data0[MAXSIZE - 1] << 1)
    {
        return 0;
    }
    
    if(data < data0[1])
    {
        return 1;
    }

    for(i = 1; i < MAXSIZE; i++)
    {
        if(data < data0[i])
        {
            return i;
        }
    }

    return MAXSIZE;
}
