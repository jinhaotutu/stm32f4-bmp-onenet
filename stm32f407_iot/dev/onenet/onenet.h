#ifndef _ONENET_H_
#define _ONENET_H_


#include "dstream.h"




typedef struct
{

    char devID[15];
    char apiKey[35];
	
	char ip[16];
	char port[8];
	
	const unsigned char protocol;	//协议类型号		1-edp	2-nwx	3-jtext		4-Hiscmd
									//				5-jt808			6-modbus	7-mqtt
									//				8-gr20			9-reg		10-HTTP(自定义)
	
	unsigned char *cmd_ptr;			//平台下发的命令
	
	unsigned char netWork : 1;		//1-OneNET接入成功		0-OneNET接入失败
	unsigned char sendData : 2;
	unsigned char errCount : 3;		//错误计数
	unsigned char heartBeat : 1;	//心跳
	unsigned char reverse : 1;

} ONETNET_INFO;

extern ONETNET_INFO oneNetInfo;





#define CHECK_CONNECTED			0	//已连接
#define CHECK_CLOSED			1	//已断开
#define CHECK_GOT_IP			2	//已获取到IP
#define CHECK_NO_DEVICE			3	//无设备
#define CHECK_INITIAL			4	//初始化状态
#define CHECK_NO_CARD			5	//没有sim卡
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
