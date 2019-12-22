#ifndef __SCCB_H
#define __SCCB_H
#include "stm32f4xx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//OVϵ������ͷ SCCB ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/14
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

/*************************��Ҫ�޸ĵĵط�*************************/
#define OV7670_XCLK_RCC		RCC_AHB1Periph_GPIOA
#define OV7670_XCLK_Pin		GPIO_Pin_8
#define OV7670_XCLK_GPIO	GPIOA
#define STM32_MCO1_DIV		RCC_MCO1Div_4

#define OV7670_SCCB_RCC		RCC_AHB1Periph_GPIOF
#define OV7670_SCCB_Pin		GPIO_Pin_9|GPIO_Pin_10
#define OV7670_SCCB_GPIO	GPIOF


//IO��������
#define SCCB_SDA_IN()  		{GPIOF->MODER&=~(0x0003<<18);GPIOF->MODER|=(0x0000<<18);}	//PF9 ���� �Ĵ�������2*9=18λ
#define SCCB_SDA_OUT() 		{GPIOF->MODER&=~(0x0003<<18);GPIOF->MODER|=(0x0001<<18);} 	//PF9 ���

//IO��������	 
#define SCCB_SCL_H    		GPIO_SetBits(GPIOF,GPIO_Pin_10)	 		//SCL
#define SCCB_SCL_L    		GPIO_ResetBits(GPIOF,GPIO_Pin_10)	 	//SCL

#define SCCB_SDA_H    		GPIO_SetBits(GPIOF,GPIO_Pin_9) 			//SDA	 
#define SCCB_SDA_L    		GPIO_ResetBits(GPIOF,GPIO_Pin_9) 		//SDA	

#define SCCB_READ_SDA   	GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_9) //����SDA  
/*************************��Ҫ�޸ĵĵط�*************************/

#define SCCB_ID   			0X42  			//OV7670��ID

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


