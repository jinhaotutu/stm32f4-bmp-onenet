/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	dStream.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-09-11
	*
	*	版本： 		V1.1
	*
	*	说明： 		cJson格式数据流通用封装
	*
	*	修改记录：	V1.1：修复当数据流flag全为0时封装错误的bug。
	************************************************************
	************************************************************
	************************************************************
**/

//协议封装文件
#include "dStream.h"

//C库
#include <string.h>
#include <stdio.h>


//==========================================================
//	函数名称：	DSTREAM_toString
//
//	函数功能：	将数值转为字符串
//
//	入口参数：	StreamArray：数据流
//				buf：转换后的缓存
//				pos：数据流中的哪个数据
//				bufLen：缓存长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void DSTREAM_toString(DATA_STREAM *streamArray, char *buf, unsigned short pos, unsigned short bufLen)
{
	
	memset(buf, 0, bufLen);

	switch((unsigned char)streamArray[pos].dataType)
	{
		case TYPE_BOOL:
			snprintf(buf, bufLen, "%d", *(_Bool *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_CHAR:
			snprintf(buf, bufLen, "%d", *(signed char *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_UCHAR:
			snprintf(buf, bufLen, "%d", *(unsigned char *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_SHORT:
			snprintf(buf, bufLen, "%d", *(signed short *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_USHORT:
			snprintf(buf, bufLen, "%d", *(unsigned short *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_INT:
			snprintf(buf, bufLen, "%d", *(signed int *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_UINT:
			snprintf(buf, bufLen, "%d", *(unsigned int *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_LONG:
			snprintf(buf, bufLen, "%ld", *(signed long *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_ULONG:
			snprintf(buf, bufLen, "%ld", *(unsigned long *)streamArray[pos].dataPoint);
		break;
			
		case TYPE_FLOAT:
			snprintf(buf, bufLen, "%.1f", *(float *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_DOUBLE:
			snprintf(buf, bufLen, "%f", *(double *)streamArray[pos].dataPoint);
		break;
		
		case TYPE_GPS:
			snprintf(buf, bufLen, "{\"lon\":%s,\"lat\":%s}", (char *)streamArray[pos].dataPoint, (char *)(streamArray[pos].dataPoint) + 16);
		break;
		
		case TYPE_STRING:
			snprintf(buf, bufLen, "\"%s\"", (char *)streamArray[pos].dataPoint);
		break;
	}

}

//==========================================================
//	函数名称：	 DSTREAM_GetDataStream_Body
//
//	函数功能：	获取数据流格式消息体
//
//	入口参数：	type：格式类型
//				streamArray：数据流结构
//				streamArrayCnt：数据流个数
//				buffer：缓存
//				maxLen：最大缓存长度
//				offset：偏移
//
//	返回参数：	Body的长度，0-失败
//
//	说明：		
//==========================================================
short DSTREAM_GetDataStream_Body(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, unsigned char *buffer, short maxLen, short offset)
{
	
	short count = 0, numBytes = 0;								//count-循环计数。numBytes-记录数据装载长度
	char stream_buf[96];
	char data_buf[48];
	short cBytes = 0;
	
	unsigned char *dataPtr = buffer + offset;
	
	for(; count < streamArrayCnt; count++)
	{
		if(streamArray[count].flag)
			break;
	}
	
	if(count == streamArrayCnt)
		return -1;
	
	count = 0;
	
	maxLen -= 1;												//预留结束符位置

	switch(type)
	{
		case FORMAT_TYPE1:
		
			if(numBytes + 16 < maxLen)
			{
				memcpy(dataPtr, "{\"datastreams\":[", 16);
				numBytes += 16;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag)						//如果使能发送标志位
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]},", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//printf("dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(dataPtr + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//将最后的','替换为结束符
			
			if(numBytes + 1 <= maxLen)
			{
				memcpy(dataPtr + numBytes - 1, "]}", 2);
				numBytes++;
			}
			else
				return 0;
		
		break;
		
		case FORMAT_TYPE3:
			
			if(numBytes + 1 < maxLen)
			{
				memcpy(dataPtr, "{", 1);
				numBytes++;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) 						//如果使能发送标志位
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":%s,", streamArray[count].name, data_buf);

					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//printf("dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(dataPtr + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//将最后的','替换为结束符

			memcpy(dataPtr + numBytes - 1, "}", 1);
		
		break;
		
		case FORMAT_TYPE4:
			
			if(numBytes + 1 < maxLen)
			{
				memcpy(dataPtr, "{", 1);
				numBytes++;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag) 						//如果使能发送标志位
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":{\"2016-08-10T12:31:17\":%s},", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes)
					{
						//printf("dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(dataPtr + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//将最后的','替换为结束符

			memcpy(dataPtr + numBytes - 1, "}", 1);
		
		break;
		
		case FORMAT_TYPE5:
		
			if(numBytes + 2 < maxLen)
			{
				memcpy(dataPtr, ",;", 2);
				numBytes += 2;
			}
			else
				return 0;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag && streamArray[count].dataType != TYPE_GPS)	//如果使能发送标志位 格式5不支持GPS
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "%s,%s;", streamArray[count].name, data_buf);
					
					cBytes = strlen(stream_buf);
					if(cBytes >= maxLen - numBytes - 2)
					{
						//printf("dStream_Get_dFormatBody Load Failed %d\r\n", numBytes);
						return 0;
					}
					
					memcpy(dataPtr + numBytes, stream_buf, cBytes);
					
					numBytes += cBytes;
					if(numBytes > maxLen)						//内存长度判断
						return 0;
				}
			}
		
		break;
		
		default:
		break;
	}
	
	//printf("Body Len: %d\r\n", numBytes);
	return numBytes;

}

//==========================================================
//	函数名称：	 DSTREAM_GetDataStream_Body_Measure
//
//	函数功能：	测量当前使能的数据流长度
//
//	入口参数：	type：格式类型
//				streamArray：数据流结构
//				streamArrayCnt：数据流个数
//				flag：1-测量全部数据流长度	0-测量当前需要发送的数据流长度
//
//	返回参数：	Body的长度
//
//	说明：		
//==========================================================
short DSTREAM_GetDataStream_Body_Measure(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, _Bool flag)
{

	short count = 0, numBytes = 0;						//count-循环计数。numBytes-记录数据装载长度
	char stream_buf[96];
	char data_buf[48];
	
	for(; count < streamArrayCnt; count++)
	{
		if(streamArray[count].flag)
			break;
	}
	
	if(count == streamArrayCnt)
		return -1;
	
	count = 0;

	switch(type)
	{
		case FORMAT_TYPE1:
			
			numBytes += 16;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag || flag)
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]},", streamArray[count].name, data_buf);
					
					numBytes += strlen(stream_buf);
				}
			}
			
			numBytes += 1;
		
		break;
		
		case FORMAT_TYPE3:
			
			numBytes++;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag || flag)
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":%s,", streamArray[count].name, data_buf);

					numBytes += strlen(stream_buf);
				}
			}
		
		break;
		
		case FORMAT_TYPE4:
			
			numBytes++;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag || flag)
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":{\"2016-08-10T12:31:17\":%s},", streamArray[count].name, data_buf);
					
					numBytes += strlen(stream_buf);
				}
			}
		
		break;
		
		case FORMAT_TYPE5:
			
			numBytes += 2;
			
			for(; count < streamArrayCnt; count++)
			{
				if(streamArray[count].flag || flag)
				{
					DSTREAM_toString(streamArray, data_buf, count, sizeof(data_buf));
					snprintf(stream_buf, sizeof(stream_buf), "%s,%s;", streamArray[count].name, data_buf);
					
					numBytes += strlen(stream_buf);
				}
			}
		
		break;
		
		default:
		break;
	}
	
	return numBytes;

}
