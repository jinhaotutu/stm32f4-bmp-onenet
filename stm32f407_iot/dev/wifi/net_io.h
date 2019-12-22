#ifndef _NET_IO_H_
#define _NET_IO_H_





#define RB_BUFFER_SIZE		256

#define RB_BUFFER2_SIZE		256


typedef struct
{

/*************************ѭ������*************************/
	unsigned char buf[RB_BUFFER_SIZE];		//���ջ���
	unsigned char buf_tp[RB_BUFFER2_SIZE];	//�������棬һ֡���ݳ�����rbĩβ���׵�ʱ�򣬰����ݸ��Ƶ��������棬�Է����ϲ����������
	unsigned short write_pos;				//д
	unsigned short read_pos;				//��
	unsigned short data_packet;				//���ݰ�

} NET_IO_INFO;

/*************************��Ҫ�޸ĵĵط�*************************/
#define RCC_IO_WIFI				RCC_AHB1Periph_GPIOB
#define PIN_WIFI				GPIO_Pin_10 | GPIO_Pin_11
#define GPIO_WIFI				GPIOB
#define AF_WIFI_TX				GPIO_PinSource10
#define AF_WIFI_RX				GPIO_PinSource11

#define WIFI_IRQn				USART3_IRQn

#define RCC_USART_WIFI			RCC_APB1Periph_USART3
#define	BAUDRATE_WIFI			115200
#define NET_IO					USART3
/*************************��Ҫ�޸ĵĵط�*************************/

void NET_USART_Init(void);

void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

unsigned char *NET_IO_Read(void);


#endif
