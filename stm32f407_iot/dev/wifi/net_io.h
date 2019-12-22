#ifndef _NET_IO_H_
#define _NET_IO_H_





#define RB_BUFFER_SIZE		256

#define RB_BUFFER2_SIZE		256


typedef struct
{

/*************************循环队列*************************/
	unsigned char buf[RB_BUFFER_SIZE];		//接收缓存
	unsigned char buf_tp[RB_BUFFER2_SIZE];	//二级缓存，一帧数据出现在rb末尾和首的时候，吧数据复制到二级缓存，以方便上层的完整处理
	unsigned short write_pos;				//写
	unsigned short read_pos;				//读
	unsigned short data_packet;				//数据包

} NET_IO_INFO;

/*************************需要修改的地方*************************/
#define RCC_IO_WIFI				RCC_AHB1Periph_GPIOB
#define PIN_WIFI				GPIO_Pin_10 | GPIO_Pin_11
#define GPIO_WIFI				GPIOB
#define AF_WIFI_TX				GPIO_PinSource10
#define AF_WIFI_RX				GPIO_PinSource11

#define WIFI_IRQn				USART3_IRQn

#define RCC_USART_WIFI			RCC_APB1Periph_USART3
#define	BAUDRATE_WIFI			115200
#define NET_IO					USART3
/*************************需要修改的地方*************************/

void NET_USART_Init(void);

void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

unsigned char *NET_IO_Read(void);


#endif
