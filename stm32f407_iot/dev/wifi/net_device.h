#ifndef _NET_DEVICE_H_
#define _NET_DEVICE_H_


//=============================配置==============================
//===========可以提供RTOS的内存管理方案，也可以使用C库的=========
#include <stdlib.h>

#define NET_MallocBuffer	malloc

#define NET_FreeBuffer		free
//==========================================================


struct NET_SEND_LIST
{

	unsigned short dataLen;			//数据长度
	unsigned char *buf;				//数据指针
	
	struct NET_SEND_LIST *next;		//下一个

};


typedef struct
{
	
	unsigned int net_time;
	
	char *cmd_resp;					//命令回复指针。比如获取命令返回的数据，可以提取此指针内的数据
	char *cmd_ipd;					//在接入onenet前通过网络获取的数据，比如网络时间、获取接入ip等
	char *cmd_hdl;					//命令处理。在发送命令后，会在返回内容里搜索用户指定的返回数据
	
/*************************发送队列*************************/
	struct NET_SEND_LIST *head, *end;
	
	unsigned short err : 4; 		//错误类型
	unsigned short initStep : 4;	//初始化步骤
	unsigned short dataType : 4;	//设定数据返回类型--16种
	unsigned short reboot : 1;		//死机重启标志
	unsigned short netWork : 1;		//网络访问OK
	unsigned short send_ok : 1;		//网络设备层的发送成功
	unsigned short reverse : 1;		//预留

} NET_DEVICE_INFO;

extern NET_DEVICE_INFO netDeviceInfo;


typedef struct
{

	char lon[16];
	char lat[16];
	
	_Bool flag;

} GPS_INFO;

extern GPS_INFO gps;

/*************************需要修改的地方*************************/
#define NET_DEVICE_RST_ON		GPIO_ResetBits(GPIOE, GPIO_Pin_12)
#define NET_DEVICE_RST_OFF		GPIO_SetBits(GPIOE, GPIO_Pin_12)

#define RCC_RESET				RCC_AHB1Periph_GPIOE
#define PIN_RESET				GPIO_Pin_12
#define GPIO_RESET				GPIOE

#define RCC_MODE				RCC_AHB1Periph_GPIOB
#define PIN_MODE				GPIO_Pin_9
#define GPIO_MODE				GPIOB

#define WIFI_NAME				"jinhao"
#define WIFI_PASSWORD			"361750389"

#define PHONE_AP_MODE			1	//1-使用手机热点模式。当周围无路由时，AirKiss无法使用，则使用固定方式配置
									//0-使用AirKiss的方式配网

#define NET_TIME_EN				0	//1-获取网络时间		0-不获取
/*************************需要修改的地方*************************/

void NET_DEVICE_IO_Init(void);

_Bool NET_DEVICE_Exist(void);

_Bool NET_DEVICE_Init(unsigned char protocol, char *ip, char *port);

void NET_DEVICE_Reset(void);

_Bool NET_DEVICE_Close(void);

_Bool NET_DEVICE_ReLink(char *ip, char *port);

void NET_DEVICE_CmdHandle(char *cmd);

_Bool NET_DEVICE_SendCmd(char *cmd, char *res);

_Bool NET_DEVICE_SendData(unsigned char *data, unsigned short len);

unsigned char *NET_DEVICE_Read(void);

unsigned char *NET_DEVICE_GetIPD(unsigned char *dataPtr);

unsigned char NET_DEVICE_Check(void);

void NET_DEVICE_ReConfig(unsigned char step);

_Bool NET_DEVICE_CheckListHead(void);

unsigned char *NET_DEVICE_GetListHeadBuf(void);

unsigned short NET_DEVICE_GetListHeadLen(void);

unsigned char NET_DEVICE_AddDataSendList(unsigned char *buf ,unsigned short dataLen);

_Bool NET_DEVICE_DeleteDataSendList(void);

_Bool NET_DEVICE_GetWifiIP(void);

#endif
