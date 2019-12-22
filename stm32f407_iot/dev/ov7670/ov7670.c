#include "ov7670.h"
#include "ov7670config.h"	  
#include "sys_delay.h"
#include "stdio.h"
#include "sccb.h"
#include "dcmi.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序参考自网友guanfu_wang代码。
//ALIENTEK战舰STM32开发板
//OV7670 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/14
//版本：V1.0		    							    							  
//////////////////////////////////////////////////////////////////////////////////
		    					
uint16_t camera_buffer[PIC_WIDTH*PIC_HEIGHT+27]={0};

//配置RESET和PWDN引脚
void OV7670_RST_PW_Init(void)
{
	//设置IO     	   
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(OV7670_RST_PW_RCC, ENABLE);
	
	//RESET,PWDN初始化设置
	GPIO_InitStructure.GPIO_Pin = OV7670_RST_PW_Pin;//RESET,PWDN推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(OV7670_RST_PW_GPIO, &GPIO_InitStructure);//初始化
}

//初始化OV7670
//返回0:成功
//返回其他值:错误代码
u8 OV7670_Init(void)
{
	u16 i=0;
	u16 reg=0;

	OV7670_RST_PW_Init();

	OV7670_PWDN_L;	//POWER ON
	delay_ms(100);
	OV7670_RST_L;	//复位OV7670
	delay_ms(100);
	OV7670_RST_H;	//结束复位 
	
	SCCB_Init();        		//初始化SCCB 的IO口	
	SCCB_WR_Reg(0X12, 0x80);	//软复位OV7670
	delay_ms(50); 
	
	reg=SCCB_RD_Reg(0X1c);	//读取厂家ID 高八位
	reg<<=8;
	reg|=SCCB_RD_Reg(0X1d);	//读取厂家ID 低八位
	if(reg!=OV7670_MID)
	{
		printf("\r\nMID:%d\r\n",reg);
		return 1;
	}
	
	reg=SCCB_RD_Reg(0X0a);	//读取厂家ID 高八位
	reg<<=8;
	reg|=SCCB_RD_Reg(0X0b);	//读取厂家ID 低八位
	if(reg!=OV7670_PID)
	{
		printf("HID:%d\r\n\r\n",reg);
		return 2;
	}   
	
	//初始化 OV7670,采用QVGA分辨率(320*240)  
	for(i=0;i<sizeof(ov7670_init_reg_tbl)/sizeof(ov7670_init_reg_tbl[0]);i++)
	{
		SCCB_WR_Reg(ov7670_init_reg_tbl[i][0],ov7670_init_reg_tbl[i][1]);
	} 

	OV7670_Window_Set(PIC_START_X,PIC_START_Y,PIC_WIDTH,PIC_HEIGHT);
	
	OV7670_Light_Mode(0);
	OV7670_Color_Saturation(2);
	OV7670_Brightness(1);
	OV7670_Contrast(2);

	My_DCMI_Init();
	DCMI_DMA_Init((uint32_t)&camera_buffer+54,(sizeof(camera_buffer)-54)/4,DMA_MemoryDataSize_HalfWord,DMA_MemoryInc_Enable);//DCMI DMA 

	return 0x00; 	//ok
} 
////////////////////////////////////////////////////////////////////////////
//OV7670功能设置
//白平衡设置
//0:自动
//1:太阳sunny
//2,阴天cloudy
//3,办公室office
//4,家里home
void OV7670_Light_Mode(u8 mode)
{
	u8 reg13val=0XE7;//默认就是设置为自动白平衡
	u8 reg01val=0;
	u8 reg02val=0;
	switch(mode)
	{
		case 1://sunny
			reg13val=0XE5;
			reg01val=0X5A;
			reg02val=0X5C;
			break;	
		case 2://cloudy
			reg13val=0XE5;
			reg01val=0X58;
			reg02val=0X60;
			break;	
		case 3://office
			reg13val=0XE5;
			reg01val=0X84;
			reg02val=0X4c;
			break;	
		case 4://home
			reg13val=0XE5;
			reg01val=0X96;
			reg02val=0X40;
			break;	
	}
	SCCB_WR_Reg(0X13,reg13val);//COM8设置 
	SCCB_WR_Reg(0X01,reg01val);//AWB蓝色通道增益 
	SCCB_WR_Reg(0X02,reg02val);//AWB红色通道增益 
}				  
//色度设置
//0:-2
//1:-1
//2,0
//3,1
//4,2
void OV7670_Color_Saturation(u8 sat)
{
	u8 reg4f5054val=0X80;//默认就是sat=2,即不调节色度的设置
 	u8 reg52val=0X22;
	u8 reg53val=0X5E;
 	switch(sat)
	{
		case 0://-2
			reg4f5054val=0X40;  	 
			reg52val=0X11;
			reg53val=0X2F;	 	 
			break;	
		case 1://-1
			reg4f5054val=0X66;	    
			reg52val=0X1B;
			reg53val=0X4B;	  
			break;	
		case 3://1
			reg4f5054val=0X99;	   
			reg52val=0X28;
			reg53val=0X71;	   
			break;	
		case 4://2
			reg4f5054val=0XC0;	   
			reg52val=0X33;
			reg53val=0X8D;	   
			break;	
	}
	SCCB_WR_Reg(0X4F,reg4f5054val);	//色彩矩阵系数1
	SCCB_WR_Reg(0X50,reg4f5054val);	//色彩矩阵系数2 
	SCCB_WR_Reg(0X51,0X00);			//色彩矩阵系数3  
	SCCB_WR_Reg(0X52,reg52val);		//色彩矩阵系数4 
	SCCB_WR_Reg(0X53,reg53val);		//色彩矩阵系数5 
	SCCB_WR_Reg(0X54,reg4f5054val);	//色彩矩阵系数6  
	SCCB_WR_Reg(0X58,0X9E);			//MTXS 
}
//亮度设置
//0:-2
//1:-1
//2,0
//3,1
//4,2
void OV7670_Brightness(u8 bright)
{
	u8 reg55val=0X00;//默认就是bright=2
  	switch(bright)
	{
		case 0://-2
			reg55val=0XB0;	 	 
			break;	
		case 1://-1
			reg55val=0X98;	 	 
			break;	
		case 3://1
			reg55val=0X18;	 	 
			break;	
		case 4://2
			reg55val=0X30;	 	 
			break;	
	}
	SCCB_WR_Reg(0X55,reg55val);	//亮度调节 
}
//对比度设置
//0:-2
//1:-1
//2,0
//3,1
//4,2
void OV7670_Contrast(u8 contrast)
{
	u8 reg56val=0X40;//默认就是contrast=2
  	switch(contrast)
	{
		case 0://-2
			reg56val=0X30;	 	 
			break;	
		case 1://-1
			reg56val=0X38;	 	 
			break;	
		case 3://1
			reg56val=0X50;	 	 
			break;	
		case 4://2
			reg56val=0X60;	 	 
			break;	
	}
	SCCB_WR_Reg(0X56,reg56val);	//对比度调节 
}
//特效设置
//0:普通模式    
//1,负片
//2,黑白   
//3,偏红色
//4,偏绿色
//5,偏蓝色
//6,复古	    
void OV7670_Special_Effects(u8 eft)
{
	u8 reg3aval=0X04;//默认为普通模式
	u8 reg67val=0XC0;
	u8 reg68val=0X80;
	switch(eft)
	{
		case 1://负片
			reg3aval=0X24;
			reg67val=0X80;
			reg68val=0X80;
			break;	
		case 2://黑白
			reg3aval=0X14;
			reg67val=0X80;
			reg68val=0X80;
			break;	
		case 3://偏红色
			reg3aval=0X14;
			reg67val=0Xc0;
			reg68val=0X80;
			break;	
		case 4://偏绿色
			reg3aval=0X14;
			reg67val=0X40;
			reg68val=0X40;
			break;	
		case 5://偏蓝色
			reg3aval=0X14;
			reg67val=0X80;
			reg68val=0XC0;
			break;	
		case 6://复古
			reg3aval=0X14;
			reg67val=0XA0;
			reg68val=0X40;
			break;	 
	}
	SCCB_WR_Reg(0X3A,reg3aval);//TSLB设置 
	SCCB_WR_Reg(0X68,reg67val);//MANU,手动U值 
	SCCB_WR_Reg(0X67,reg68val);//MANV,手动V值 
}	
//设置图像输出窗口
//对QVGA设置。
void OV7670_Window_Set(u16 sx,u16 sy,u16 width,u16 height)
{
	u16 endx;
	u16 endy;
	u8 temp; 
	
	if ((sx+width) > 320)
	{
		width = 320 - sx;
	}
	
	if ((sy+height) > 240)
	{
		height = 240 - sy;
	}
	
	sx += 176;
	sy += 12;
	
	endx=sx+width*2;	//HREF
 	endy=sy+height*2;	//VREF
	if(endx>784)
	{
		endx-=784;
	}
	
	temp=SCCB_RD_Reg(0X32);				//读取Href之前的值
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WR_Reg(0X32,temp);
	SCCB_WR_Reg(0X17,sx>>3);			//设置Href的start高8位
	SCCB_WR_Reg(0X18,endx>>3);			//设置Href的end的高8位
	
	temp=SCCB_RD_Reg(0X03);				//读取Vref之前的值
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WR_Reg(0X03,temp);				//设置Vref的start和end的最低2位
	SCCB_WR_Reg(0X19,sy>>2);			//设置Vref的start高8位
	SCCB_WR_Reg(0X1A,endy>>2);			//设置Vref的end的高8位


}



