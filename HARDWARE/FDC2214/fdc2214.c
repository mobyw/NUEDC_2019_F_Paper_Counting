#include "fdc2214.h"
#include "delay.h"
#include "usart.h"

void FDC_IIC_Delay(void)
{
    delay_us(8);
}

void FDC_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(    RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);
}

void FDC_IIC_Start(void)
{
    FDC_SDA_OUT();
    FDC_IIC_SDA = 1;
    FDC_IIC_SCL = 1;
    FDC_IIC_Delay();
     FDC_IIC_SDA = 0;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 0;
}      

void FDC_IIC_Stop(void)
{
    FDC_SDA_OUT();
    FDC_IIC_SCL = 0;
    FDC_IIC_SDA = 0;
     FDC_IIC_Delay();
    FDC_IIC_SCL = 1;
    FDC_IIC_SDA = 1;
    FDC_IIC_Delay();
}

u8 FDC_IIC_Wait_Ack(void)
{
    u16 ucErrTime = 0;
    FDC_SDA_IN();
    FDC_IIC_SDA = 1;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 1;
    FDC_IIC_Delay();

    while(FDC_READ_SDA)
    {
        ucErrTime++;

        if(ucErrTime > 10)
        {
            FDC_IIC_Stop();

            return 1;
        }
    }

    FDC_IIC_SCL = 0;

    return 0;  
}

void FDC_IIC_Ack(void)
{
    FDC_IIC_SCL = 0;
    FDC_IIC_SDA = 0;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 1;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 0;
    FDC_IIC_Delay();
    FDC_IIC_SDA = 1;
}
    
void FDC_IIC_NAck(void)
{
    FDC_IIC_SCL = 0;
    FDC_SDA_OUT();
    FDC_IIC_SDA = 1;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 1;
    FDC_IIC_Delay();
    FDC_IIC_SCL = 0;
}

void FDC_IIC_Send_Byte(u8 txd)
{
    u8 t;
    
    FDC_SDA_OUT();
    FDC_IIC_SCL = 0;
    
    for(t = 0; t < 8; t++)
    {
        FDC_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        FDC_IIC_SCL = 1;
        FDC_IIC_Delay();
        FDC_IIC_SCL = 0;
        FDC_IIC_Delay();
    }
}

u8 FDC_IIC_Read_Byte(u8 ack)
{
    u8 i, receive = 0;
    FDC_SDA_IN();

    for(i = 0; i < 8; i++)
    {
        FDC_IIC_SCL = 0;
        FDC_IIC_Delay();
        FDC_IIC_SCL = 1;
        receive <<= 1;

        if(FDC_READ_SDA)
        {
            receive++;
        }

        FDC_IIC_Delay();
    }

    if (!ack)
    {
        FDC_IIC_NAck();
    }
    else
    {
        FDC_IIC_Ack();
    }

    return receive;
}

u8 Set_FDC2214(u8 reg,u8 MSB,u8 LSB)
{
    FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|0);

    if(FDC_IIC_Wait_Ack())
    {
        FDC_IIC_Stop();
        
        return 1;
    }

    FDC_IIC_Send_Byte(reg);
    FDC_IIC_Wait_Ack();
    FDC_IIC_Send_Byte(MSB);

    if(FDC_IIC_Wait_Ack())
    {
        FDC_IIC_Stop();
        
        return 1;
    }

    FDC_IIC_Send_Byte(LSB);

    if(FDC_IIC_Wait_Ack())
    {
        FDC_IIC_Stop();
        
        return 1;         
    }

    FDC_IIC_Stop();
    
    return 0;
}

u16 FDC_Read(u8 reg)
{
    u16 res;

    FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|0);
    FDC_IIC_Wait_Ack();
    FDC_IIC_Send_Byte(reg);
    FDC_IIC_Wait_Ack();

    FDC_IIC_Start();
    FDC_IIC_Send_Byte((FDC2214_ADDR<<1)|1);
    FDC_IIC_Wait_Ack();
    res = FDC_IIC_Read_Byte(1) << 8;
    res |= FDC_IIC_Read_Byte(0);
    FDC_IIC_Stop();

    return res;        
}

u32 FCD2214_ReadCH(u8 index) 
{
    u32 result;

    switch(index)
    {
        case 0:
            result = FDC_Read(DATA_CH0) & 0x0FFF;
            result = (result<<16) | (FDC_Read(DATA_LSB_CH0));
            break;
        case 1:
            result = FDC_Read(DATA_CH1) & 0x0FFF;
            result = (result<<16) | (FDC_Read(DATA_LSB_CH1));
            break;
        case 2:
            result = FDC_Read(DATA_CH2) & 0x0FFF;
            result = (result<<16) | (FDC_Read(DATA_LSB_CH2));
            break;
        case 3:
            result = FDC_Read(DATA_CH3) & 0x0FFF;
            result = (result<<16) | (FDC_Read(DATA_LSB_CH3));
            break;
        default:
            break;
    }

    result = result & 0x0FFFFFFF;

    return result;
}

u8 FDC2214_Init(void)
{
    u16 res, ID_FDC2214;

    FDC_IIC_Init();
    res = FDC_Read(MANUFACTURER_ID);
    ID_FDC2214 = FDC_Read(DEVICE_ID);

    printf("ID: %x \r\n", ID_FDC2214);
    
    if(res == 0x5449)
    {
        Set_FDC2214(RCOUNT_CH0, 0x34, 0xFB);
        Set_FDC2214(RCOUNT_CH1, 0x34, 0xFB);
        Set_FDC2214(RCOUNT_CH2, 0x34, 0xFB);
        Set_FDC2214(RCOUNT_CH3, 0x34, 0xFB);
        
        Set_FDC2214(SETTLECOUNT_CH0, 0x00, 0x1B);
        Set_FDC2214(SETTLECOUNT_CH1, 0x00, 0x1B);
        Set_FDC2214(SETTLECOUNT_CH2, 0x00, 0x1B);
        Set_FDC2214(SETTLECOUNT_CH3, 0x00, 0x1B);
        
        Set_FDC2214(CLOCK_DIVIDERS_C_CH0, 0x20, 0x02);
        Set_FDC2214(CLOCK_DIVIDERS_C_CH1, 0x20, 0x02);
        Set_FDC2214(CLOCK_DIVIDERS_C_CH2, 0x20, 0x02);
        Set_FDC2214(CLOCK_DIVIDERS_C_CH3, 0x20, 0x02);
        
        Set_FDC2214(DRIVE_CURRENT_CH0, 0x78, 0x00);
        Set_FDC2214(DRIVE_CURRENT_CH1, 0x78, 0x00);
        Set_FDC2214(DRIVE_CURRENT_CH2, 0x78, 0x00);
        Set_FDC2214(DRIVE_CURRENT_CH3, 0x78, 0x00);
        
        Set_FDC2214(ERROR_CONFIG, 0x00, 0x00);
        Set_FDC2214(MUX_CONFIG, 0xC2, 0x0D);
        Set_FDC2214(CONFIG, 0x14, 0x01);
    }
    else return 1;
    
    return 0;
}

float Cap_Calculate(u8 chx)
{
    u32 Data_FDC;
    float Cap;

    Data_FDC = FCD2214_ReadCH(chx);
    Cap = 232021045.248 / (Data_FDC);

    return (Cap * Cap);
}
