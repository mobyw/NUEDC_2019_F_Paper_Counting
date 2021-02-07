#include "fdc2214.h"
#include "usart.h"
#include "delay.h"
#include "math.h"
#include "oled.h"
#include "bmp.h"
#include "sys.h"
#include "key.h"

#define MAXSIZE 20
// #define GLS
#ifdef  GLS
#define STEP    3
#endif

// Calibration data
u32     data0[MAXSIZE];
u32     org [4];          // Original data
u32     res;
u8      num;
u8      flag = 0;         // Key press flag
char    disp[20];         // Display buffer
double  k;
double  b;

void    blink(void);
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

    blink();
    delay_ms(500);

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

                org[0] = FCD2214_ReadCH(0);
                org[1] = FCD2214_ReadCH(1);
                org[2] = FCD2214_ReadCH(0);
                org[3] = FCD2214_ReadCH(1);
                printf("DATA: \r\n ch0: %d, ch1: %d \r\n", org[0], org[1]);

                res = (org[0] + org[1] + org[2] + org[3]) / 4;
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

            blink();
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
void blink(void)
{
    uint8_t i = 2;
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

#ifndef GLS

// Recognize paper number
// data: raw data
// return: paper number
u8 recognize(u32 data)
{
    u8 i = 0;

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

    return MAXSIZE;
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
        
        org[0] = FCD2214_ReadCH(0);
        org[1] = FCD2214_ReadCH(1);
        org[2] = FCD2214_ReadCH(0);
        org[3] = FCD2214_ReadCH(1);

        res = (org[0] + org[1] + org[2] + org[3]) / 4;
        data0[index] = res;

        OLED_Clear();
        sprintf((char*)disp, "num%d: %d", index, res);
        OLED_ShowString (20, 50, (u8*)disp, 12);
        OLED_Refresh();

        blink();

        if (res == 0)
        {
            index--;
        }
        else if (index > 1 && res < data0[index - 1])
        {
            index -= 2;
        }
        else
        {
            printf("Paper: %d, value: %d \r\n", index, res);
        }
    }
    
    sortout();
}

#else

u8 recognize(u32 data)
{
    double tmp;
    u8 res;

    if(data > data0[0] >> 1)
    {
        return 0;
    }
    
    if(data < data0[1])
    {
        return 1;
    }

    tmp = exp((data - b) / k);
    res = (u8)(tmp + 0.5);

    return res;
}

// Calibration
void Calibration(void)
{
    double A = 0.0; 
    double B = 0.0; 
    double C = 0.0; 
    double D = 0.0;
    double tmp = 0;
    double data_x, data_y;
    u8 index = 0;

    do {
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
        
        org[0] = FCD2214_ReadCH(0);
        org[1] = FCD2214_ReadCH(1);

        res = (org[0] + org[1]) / 2;
        data_y = res;
        data_x = log(index);

        OLED_Clear();
        sprintf((char*)disp, "num%d: %d", index, res);
        OLED_ShowString (20, 50, (u8*)disp, 12);
        OLED_Refresh();

        blink();

        printf("Paper: %d, value: %d \r\n", index, res);
    } while (res == 0);

    for (index = 1; index < MAXSIZE; index += STEP)
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
        
        org[0] = FCD2214_ReadCH(0);
        org[1] = FCD2214_ReadCH(1);

        res = (org[0] + org[1]) / 2;
        data_y = res;
        data_x = log(index);

        OLED_Clear();
        sprintf((char*)disp, "num%d: %d", index, res);
        OLED_ShowString (20, 50, (u8*)disp, 12);
        OLED_Refresh();

        blink();

        if (res == 0)
        {
            index -= STEP;
        }
        else
        {
            printf("Paper: %d, value: %d \r\n", index, res);
            A += data_x * data_x;
            B += data_x;
            C += data_x * data_y;
            D += data_y;
        }
    }

    tmp = (A * MAXSIZE - B * B);
    k = (C * MAXSIZE - B * D) / tmp; 
    b = (A * D - C * B) / tmp;
}

#endif
