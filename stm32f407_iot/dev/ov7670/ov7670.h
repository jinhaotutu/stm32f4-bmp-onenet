#ifndef _OV7670_H
#define _OV7670_H

#include "stm32f4xx.h"

/*************************需要修改的地方*************************/
#define OV7670_RST_PW_RCC		RCC_AHB1Periph_GPIOG
#define OV7670_RST_PW_Pin		GPIO_Pin_8|GPIO_Pin_9
#define OV7670_RST_PW_GPIO		GPIOG

#define OV7670_PWDN_H  			GPIO_SetBits(GPIOG,GPIO_Pin_9)			//POWER DOWN控制信号 
#define OV7670_PWDN_L  			GPIO_ResetBits(GPIOG,GPIO_Pin_9)		//POWER DOWN控制信号 

#define OV7670_RST_H  			GPIO_SetBits(GPIOG,GPIO_Pin_8)			//复位控制信号 
#define OV7670_RST_L  			GPIO_ResetBits(GPIOG,GPIO_Pin_8)		//复位控制信号 

//320*240裁剪相关定义
#define PIC_START_X				0		//起始坐标x
#define PIC_START_Y				0		//起始坐标y
#define PIC_WIDTH				320		//照片长度
#define PIC_HEIGHT				200		//照片高度

extern uint16_t camera_buffer[PIC_WIDTH*PIC_HEIGHT+27];
/*************************需要修改的地方*************************/

////////////////////////////////////////////////////////////////////////////////// 
#define OV7670_MID				0X7FA2    
#define OV7670_PID				0X7673
/////////////////////////////////////////
	    				 
u8   OV7670_Init(void);		  	   		 
void OV7670_Light_Mode(u8 mode);
void OV7670_Color_Saturation(u8 sat);
void OV7670_Brightness(u8 bright);
void OV7670_Contrast(u8 contrast);
void OV7670_Special_Effects(u8 eft);
void OV7670_Window_Set(u16 sx,u16 sy,u16 width,u16 height);
void set_cif(void);

#endif


