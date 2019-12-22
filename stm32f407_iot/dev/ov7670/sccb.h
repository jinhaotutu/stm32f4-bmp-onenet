#ifndef __SCCB_H
#define __SCCB_H
#include "stm32f4xx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//OV系列摄像头 SCCB 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

/*************************需要修改的地方*************************/
#define OV7670_XCLK_RCC		RCC_AHB1Periph_GPIOA
#define OV7670_XCLK_Pin		GPIO_Pin_8
#define OV7670_XCLK_GPIO	GPIOA
#define STM32_MCO1_DIV		RCC_MCO1Div_4

#define OV7670_SCCB_RCC		RCC_AHB1Periph_GPIOF
#define OV7670_SCCB_Pin		GPIO_Pin_9|GPIO_Pin_10
#define OV7670_SCCB_GPIO	GPIOF


//IO方向设置
#define SCCB_SDA_IN()  		{GPIOF->MODER&=~(0x0003<<18);GPIOF->MODER|=(0x0000<<18);}	//PF9 输入 寄存器左移2*9=18位
#define SCCB_SDA_OUT() 		{GPIOF->MODER&=~(0x0003<<18);GPIOF->MODER|=(0x0001<<18);} 	//PF9 输出

//IO操作函数	 
#define SCCB_SCL_H    		GPIO_SetBits(GPIOF,GPIO_Pin_10)	 		//SCL
#define SCCB_SCL_L    		GPIO_ResetBits(GPIOF,GPIO_Pin_10)	 	//SCL

#define SCCB_SDA_H    		GPIO_SetBits(GPIOF,GPIO_Pin_9) 			//SDA	 
#define SCCB_SDA_L    		GPIO_ResetBits(GPIOF,GPIO_Pin_9) 		//SDA	

#define SCCB_READ_SDA   	GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_9) //输入SDA  
/*************************需要修改的地方*************************/

#define SCCB_ID   			0X42  			//OV7670的ID

///////////////////////////////////////////
void SCCB_Init(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
u8 SCCB_WR_Byte(u8 dat);
u8 SCCB_RD_Byte(void);
u8 SCCB_WR_Reg(u8 reg,u8 data);
u8 SCCB_RD_Reg(u8 reg);
#endif


