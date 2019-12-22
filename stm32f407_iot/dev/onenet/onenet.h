#ifndef _ONENET_H_
#define _ONENET_H_


#include "dstream.h"




typedef struct
{

    char devID[15];
    char apiKey[35];
	
	char ip[16];
	char port[8];
	
	const unsigned char protocol;	//Э�����ͺ�		1-edp	2-nwx	3-jtext		4-Hiscmd
									//				5-jt808			6-modbus	7-mqtt
									//				8-gr20			9-reg		10-HTTP(�Զ���)
	
	unsigned char *cmd_ptr;			//ƽ̨�·�������
	
	unsigned char netWork : 1;		//1-OneNET����ɹ�		0-OneNET����ʧ��
	unsigned char sendData : 2;
	unsigned char errCount : 3;		//�������
	unsigned char heartBeat : 1;	//����
	unsigned char reverse : 1;

} ONETNET_INFO;

extern ONETNET_INFO oneNetInfo;





#define CHECK_CONNECTED			0	//������
#define CHECK_CLOSED			1	//�ѶϿ�
#define CHECK_GOT_IP			2	//�ѻ�ȡ��IP
#define CHECK_NO_DEVICE			3	//���豸
#define CHECK_INITIAL			4	//��ʼ��״̬
#define CHECK_NO_CARD			5	//û��sim��
#define CHECK_NO_ERR			255 //

#define SEND_TYPE_OK			0	//
#define SEND_TYPE_DATA			1	//
#define SEND_TYPE_HEART			2	//
#define SEND_TYPE_PICTURE		3	//




void OneNet_DevLink(const char* devid, const char* auth_key);

unsigned char OneNet_SendData(FORMAT_TYPE type, char *devid, char *apikey, DATA_STREAM *streamArray, unsigned short streamArrayCnt);

_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len);

void OneNet_SendData_Picture(char *devid, const char* picture, unsigned int pic_len);

unsigned char OneNet_SendData_Heart(void);

_Bool OneNet_Check_Heart(void);

void OneNET_CmdHandle(void);

void OneNet_RevPro(unsigned char *dataPtr);

#endif
