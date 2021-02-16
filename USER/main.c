#include "fdc2214.h"
#include "usart.h"
#include "delay.h"
#include "math.h"
#include "oled.h"
#include "bmp.h"
#include "sys.h"
#include "key.h"

#define MAXSIZE 41

// Calibration data
u32     data0[MAXSIZE];
u32     org [20];          // Original data
u32     res;
u8      num;
u8      i;
u8      flag = 0;         // Key press flag
char    disp [20];        // Display buffer
double  k;
double  b;

void    blink(u8 t);
void    sortout(void);
u8      recognize(u32 data);
void    Calibration(void);

int main(void)
{
    delay_init();
    Key_Init();
    uart_init(115200);      // Serial port 1 @ 115200
    NVIC_Configuration();
    
	OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Refresh();

    while(FDC2214_Init())
    {
        delay_ms(50);
    }

    blink(5);
    delay_ms(100);

    Calibration();

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

                for (i = 0; i < 20; i += 2)
                {
                    org[i] = FCD2214_ReadCH(0);
                    org[i + 1] = FCD2214_ReadCH(0);
                    org[i] = (org[i] + org[i + 1]) / 2;
                }

                for (i = 0; i < 20; i += 4)
                {
                    org[i] = (org[i] + org[i + 2]) / 2;
                }

                res = (org[0] + org[4] + org[8] + org[12] + org[16]) / 5;
            } while (res == 0);

            num = recognize(res);
            
            OLED_Clear();

            if(num)
            {
                sprintf((char*)disp, "Number: %d", num);
                OLED_ShowString (25,  16, (u8*)disp, 16);
            }
            else
            {
                sprintf((char*)disp, "Short circuit");
                OLED_ShowString (8,  16, (u8*)disp, 16);
            }

            sprintf((char*)disp, "res: %d", res);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            OLED_Refresh();

            blink(3);
        }
        else
        {
            OLED_Clear();

            if(num)
            {
                sprintf((char*)disp, "Number: %d", num);
                OLED_ShowString (25,  16, (u8*)disp, 16);
            }
            else
            {
                sprintf((char*)disp, "Short circuit");
                OLED_ShowString (8,  16, (u8*)disp, 16);
            }

            sprintf((char*)disp, "res: %d", res);
            OLED_ShowString (30, 50, (u8*)disp, 12);
            OLED_Refresh();
        }

        flag = Key_Read();
        delay_ms(100);
    }
}

// Led blink
void blink(u8 t)
{
    do{
        delay_ms(100);  LED_ON();
        delay_ms(100);  LED_OFF();
    } while(t--);
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
    if(data > data0[0] >> 1)
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

    return (MAXSIZE - 1);
}

// Calibration
void Calibration(void)
{
    u8 index = 0;        // Array index

    for (index = 0; index < MAXSIZE; index++)
    {
        sprintf((char*)disp, "Put on %d paper(s)", index);
        OLED_ShowString (12, 2, (u8*)disp, 12);
        OLED_ShowString (12, 25, (u8*)"Then press button", 12);

        OLED_Refresh();

        while (Key_Read())
        {
            delay_ms(10);
        }

        while (FDC2214_Init())
        {
            delay_ms(50);
        }

        delay_ms(500);
        
        for (i = 0; i < 20; i += 2)
        {
            org[i] = FCD2214_ReadCH(0);
            org[i + 1] = FCD2214_ReadCH(0);
            org[i] = (org[i] + org[i + 1]) / 2;
        }

        for (i = 0; i < 20; i += 4)
        {
            org[i] = (org[i] + org[i + 2]) / 2;
        }

        res = (org[0] + org[4] + org[8] + org[12] + org[16]) / 5;
        data0[index] = res;

        OLED_Clear();
        sprintf((char*)disp, "num%d: %d", index, res);
        OLED_ShowString (20, 50, (u8*)disp, 12);
        OLED_Refresh();

        blink(1);
        printf("Paper: %d, value: %d \r\n", index, res);

        if (res == 0)
        {
            index--;
        }
        else if (index > 1 && res < data0[index - 1])
        {
            index -= 2;
        }
        else {}
    }
    
    sortout();
}
