/************************Copyright (c)********************************
**                          PP-DIY         
**------------------------File Info------------------------------------
** File name:		si702x.c
** Created by：　　　　 P.P　
** Last modified Date:  2014-10-3
** Descriptions:　
***************************************************************************/
#include "SI7021.h"
#include "stdio.h"

float Temp_buf;
float Humi_buf;
struct SI70XX_TYPE SI70XX;

//========================================
//FUNCTION:ROUTES TO PROVIDE A START SIGNAL
//========================================
void SI_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(SI7021_RCC,ENABLE);
    
    GPIO_InitStructure.GPIO_Pin=SI7021_PIN;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
    GPIO_Init(SI7021_GPIO, &GPIO_InitStructure);
	
	SI7021_CLK_H;
	SI7021_SDA_H;
	SI7021_POWER_ON;
}

void SI_start(void) 
{ 
    SI7021_SDA_OUT;
    SI7021_CLK_L;
    SI7021_IIC_Delay();
    SI7021_SDA_H;
    SI7021_IIC_Delay();
    SI7021_CLK_H;
    SI7021_IIC_Delay();
    SI7021_SDA_L;
    SI7021_IIC_Delay();
    SI7021_CLK_L;
    SI7021_IIC_Delay();  
    return;
}

//=======================================
//FUNCTION:ROUTES TO PROVIDE A STOP SIGNAL
//========================================
void SI_stop(void)
{
    SI7021_SDA_OUT;
    SI7021_SDA_L;
    SI7021_CLK_L;
    SI7021_IIC_Delay();
    SI7021_CLK_H;
    SI7021_IIC_Delay();
    SI7021_SDA_H;
    SI7021_IIC_Delay();
    SI7021_CLK_L;
    SI7021_IIC_Delay();
    SI7021_SDA_L;

    return;
}

//-----------------------------------------------------------------------------
// check the acknowledge
//
// Inputs:
//		wrdata:  Byte to be written
//
// Pin drive status upon exit:
//      SDIO = high (open-drain input)
//      SCLK = high
//-----------------------------------------------------------------------------
uchar Slave_acknowledge(void)
{
    //check the acknowledge    
    SI7021_SDA_IN; 
    SI7021_SDA_H;  
    SI7021_CLK_L;
    SI7021_IIC_Delay();  
    SI7021_CLK_H;
    SI7021_IIC_Delay(); 
    //while (SI7021_Data);//长时间等待退出
    for(uchar i=0;i<254;i++)//容错处理，防止死等
    {
        if(SI7021_Data)
        {
            SI7021_IIC_Delay();
        }
        else
        {
            break;//直接退出
        }

        if(i==200)
        {
            //超时出错
            return(1);
        }
    }
    SI7021_CLK_L;
    SI7021_IIC_Delay();
    return(0);
}

void Master_acknowledge(void)
{   
    SI7021_SDA_OUT; 
    SI7021_SDA_L;
    SI7021_IIC_Delay(); 
    SI7021_CLK_L;
    SI7021_IIC_Delay(); 
    SI7021_CLK_H;
    SI7021_IIC_Delay();
    SI7021_CLK_L;
    SI7021_IIC_Delay(); 
    return;  
}

void No_ack(void)
{
    SI7021_SDA_IN;
    SI7021_IIC_Delay();
    SI7021_CLK_L; 
    SI7021_IIC_Delay();
    SI7021_CLK_H;
    SI7021_IIC_Delay();
    SI7021_CLK_L; 
    SI7021_IIC_Delay();
    return;
}
/*********************************************************************
*函数名称: char SI_R_Byte(uchar ack)
*说    明：reads a byte form the Sensibus and gives an acknowledge in case of "ack=1" 
*********************************************************************/
uchar SI_R_Byte(void)
{
    uchar val=0;
    SI7021_SDA_IN;                     //release DATA-line
    SI7021_CLK_L;
    for(uchar i=8;i>0;i--)
    {
        SI7021_CLK_L;    
        SI7021_IIC_Delay();
        SI7021_CLK_H; 
        SI7021_IIC_Delay();
        if(SI7021_Data) 
        {
            val |= 0x01;
        }
        if(i!=1)
        {
            val<<=1;
        }
    }
    SI7021_CLK_L;
    return(val);
}

/*********************************************************************
*函数名称: uchar SI_W_Byte(uchar value)
*说    明：
*********************************************************************/
void SI_W_Byte(uchar value)
{ 
    SI7021_SDA_OUT;
    SI7021_CLK_L;
    for(uchar i=0x80;i>0;i>>=1)           //shift bit for masking
    { 
        if (i & value) 
        {
            SI7021_SDA_H;   //masking value with i , write to SENSI-BUS
        }                 
        else 
        {
            SI7021_SDA_L; 
        }                       
        SI7021_IIC_Delay();              //observe setup time
        SI7021_CLK_H;                    //clk for SENSI-BUS
        SI7021_IIC_Delay();              //pulswith approx. 5 us  	
        SI7021_CLK_L;
        SI7021_IIC_Delay();
        SI7021_SDA_L;
    } 

    return;                   //error=1 in case of no acknowledge
}


/******************************************************************************
*函数名称:ucahr s_measure(uchar *p_value, uchar *p_checksum, uchar mode)
*说    明:makes a measurement (humidity/temperature) with checksum
*******************************************************************************/
uchar SI_Read_Register(uchar Reg_add)
{ 
    uchar temp=0;
//====================================================//
    SI_start();
    SI_W_Byte(0x80); 
    Slave_acknowledge();
    SI_W_Byte(Reg_add); 
    Slave_acknowledge();
    SI_start();
    SI_W_Byte(0x81);
    Slave_acknowledge();
    temp=SI_R_Byte();
    Master_acknowledge();
    SI_stop();
    return(temp);
}

/******************************************************************************
*函数名称:ucahr s_measure(uchar *p_value, uchar *p_checksum, uchar mode)
*说    明:makes a measurement (humidity/temperature) with checksum
*******************************************************************************/
uchar SI_Write_Register(uchar Reg_add,uchar Code)
{ 
    uchar temp=0;
//====================================================//
    SI_start();
    SI_W_Byte(0x80); 
    Slave_acknowledge(); 
    SI_W_Byte(Reg_add); 
    Slave_acknowledge();
    SI_W_Byte(Code); 
    Slave_acknowledge();
    SI_stop();
    return(temp);
}

/*****************************************************************************/
/* SI70XX_Read_UserRegister                                                  */
/*****************************************************************************/

uchar SI70XX_Read_UserRegister(void)
{
   uchar  Data;

    SI_start();	
    SI_W_Byte(0x80);//  si7013
    Slave_acknowledge();
    SI_W_Byte(CMD_READ_REGISTER_1);
    Slave_acknowledge();
    
    delay_ms(20);//>=20ms等待转换
    SI_start();
    SI_W_Byte(0x81);//地址读
    Slave_acknowledge();
    Data=SI_R_Byte();
    No_ack();
    SI_stop();
   return(Data);
}
#ifndef I2C_SIM
unsigned char I2C_Err=0;
uint8_t I2C_ReadOneByte(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t Reg_addr)
{  
	RCC_ClocksTypeDef   rcc_clocks;
    uint8_t readout;
    u32 tmr;
	u32 ulTimeOut_Time;
	
	RCC_GetClocksFreq(&rcc_clocks);
    ulTimeOut_Time = (rcc_clocks.SYSCLK_Frequency /10000); 
	
    tmr = ulTimeOut_Time;
    while((--tmr)&&I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    if(tmr==0) I2C_Err = 1;

    I2C_GenerateSTART(I2Cx, ENABLE);
    //??I2C?START??,?????????????
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)));
    if(tmr==0) I2C_Err = 1;

    I2C_Send7bitAddress(I2Cx,I2C_Addr,I2C_Direction_Transmitter);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)));
    if(tmr==0) I2C_Err = 1;

    I2C_SendData(I2Cx, Reg_addr);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)));
    if(tmr==0) I2C_Err = 1;

    I2C_GenerateSTART(I2Cx, ENABLE);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)));
    if(tmr==0) I2C_Err = 1;

    I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Receiver);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)));
    if(tmr==0) I2C_Err = 1; 

    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))));  /* EV7 */
    if(tmr==0) I2C_Err = 1;

    readout = I2C_ReceiveData(I2Cx);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    return readout;
}
#endif

/*****************************************************************************/
/* SI70XX_Read_UserRegister                                                  */
/*****************************************************************************/

void SI70XX_Write_UserRegister(uchar Data)
{
    SI_start();	
    SI_W_Byte(0x80);//  si7013
    Slave_acknowledge();
    SI_W_Byte(CMD_WRITE_REGISTER_1);
    Slave_acknowledge();
    SI_W_Byte(Data);
    Master_acknowledge();
    SI_stop();
    return;
}

#ifndef I2C_SIM
void I2C_WriteOneByte(I2C_TypeDef *I2Cx,uint8_t I2C_Addr,uint8_t Reg_addr,uint8_t value)
{
	RCC_ClocksTypeDef   rcc_clocks;
    u32 tmr;
	u32 ulTimeOut_Time;
	
	RCC_GetClocksFreq(&rcc_clocks);
    ulTimeOut_Time = (rcc_clocks.SYSCLK_Frequency /10000); 

    tmr = ulTimeOut_Time;
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    while((--tmr)&&I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    if(tmr==0) I2C_Err = 1;

    I2C_GenerateSTART(I2Cx, ENABLE);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))); 
    if(tmr==0) I2C_Err = 1;

    I2C_Send7bitAddress(I2Cx, I2C_Addr, I2C_Direction_Transmitter);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)));
    if(tmr==0) I2C_Err = 1;

    I2C_SendData(I2Cx, Reg_addr);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)));
    if(tmr==0) I2C_Err = 1;

    I2C_SendData(I2Cx, value);
    tmr = ulTimeOut_Time;
    while((--tmr)&&(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)));
    if(tmr==0) I2C_Err = 1;

    I2C_GenerateSTOP(I2Cx, ENABLE);
    //I2C_AcknowledgeConfig(I2Cx, DISABLE);
}
#endif

/*****************************************************************************/
/* Sensor_Measure                                                            */
/*****************************************************************************/

uchar SI70XX_Sensor_Measure( uchar Config, uint *Value )
{
    uchar  Data[2];

    SI_start();	
    SI_W_Byte(0x80);//  si7013

    if(Slave_acknowledge())//无反应直接退出
    {//容错处理，防止死等
        return(0);
    }
    SI_W_Byte(Config);
    Slave_acknowledge();

    delay_ms(20);//>=20ms等待转换
    SI_start();
    SI_W_Byte(0x81);//地址读
    Slave_acknowledge();
    Data[0]=SI_R_Byte();
    Master_acknowledge();
    Data[1]=SI_R_Byte();
    No_ack();
    SI_stop();

    /* Swap the bytes and clear the status bits */   
    *Value = (Data[0]*256) + Data[1];

    return(1);
}

/******************************************************************************
*函数名称:ucahr s_measure(uchar *p_value, uchar *p_checksum, uchar mode)
*说    明:makes a measurement (humidity/temperature) with checksum
*******************************************************************************/
uchar SI7021_measure(void)
{ 
    uint Value;
//    uchar temp;
    uchar ucConversion_OK=0;//转换成功标记

    //开启测试
    SI_stop();//以防万一，先弄条停止位
    
    ucConversion_OK=SI70XX_Sensor_Measure(CMD_MEASURE_TEMPERATURE_NO_HOLD, &Value );
    
    if(!ucConversion_OK)
    {
        //转换失败，直接退出，方便模块热插拔
        SI70XX.inTemp = 0;
        SI70XX.unHumi = 0;
        return(0);//错误=0
    }
    
    SI70XX.inTemp = ((((long)Value)*1757)>>16) - 469;
    Temp_buf = (((((float)Value)*175.72f)/65536.0f) - 46.85f);

    SI70XX_Sensor_Measure(CMD_MEASURE_HUMIDITY_NO_HOLD, &Value );
    
    SI70XX.unHumi = ((((long)Value)*625)>>15) - 60;
    Humi_buf = (((((float)Value)*125.0f)/65535.0f) - 6.0f);

    printf("\r\nTemp:%.2f\r\n",Temp_buf);
    printf("Humi:%.2f\r\n",Humi_buf);

    return(1);//正常=1
}

