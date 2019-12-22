/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	dStream.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-09-11
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		cJson��ʽ������ͨ�÷�װ
	*
	*	�޸ļ�¼��	V1.1���޸���������flagȫΪ0ʱ��װ�����bug��
	************************************************************
	************************************************************
	************************************************************
**/

//Э���װ�ļ�
#include "dStream.h"

//C��
#include <string.h>
#include <stdio.h>


//==========================================================
//	�������ƣ�	DSTREAM_toString
//
//	�������ܣ�	����ֵתΪ�ַ���
//
//	��ڲ�����	StreamArray��������
//				buf��ת����Ļ���
//				pos���������е��ĸ�����
//				bufLen�����泤��
//
//	���ز�����	��
//
//	˵����		
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
//	�������ƣ�	 DSTREAM_GetDataStream_Body
//
//	�������ܣ�	��ȡ��������ʽ��Ϣ��
//
//	��ڲ�����	type����ʽ����
//				streamArray���������ṹ
//				streamArrayCnt������������
//				buffer������
//				maxLen����󻺴泤��
//				offset��ƫ��
//
//	���ز�����	Body�ĳ��ȣ�0-ʧ��
//
//	˵����		
//==========================================================
short DSTREAM_GetDataStream_Body(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, unsigned char *buffer, short maxLen, short offset)
{
	
	short count = 0, numBytes = 0;								//count-ѭ��������numBytes-��¼����װ�س���
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
	
	maxLen -= 1;												//Ԥ��������λ��

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
				if(streamArray[count].flag)						//���ʹ�ܷ��ͱ�־λ
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
					if(numBytes > maxLen)						//�ڴ泤���ж�
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//������','�滻Ϊ������
			
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
				if(streamArray[count].flag) 						//���ʹ�ܷ��ͱ�־λ
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
					if(numBytes > maxLen)						//�ڴ泤���ж�
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//������','�滻Ϊ������

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
				if(streamArray[count].flag) 						//���ʹ�ܷ��ͱ�־λ
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
					if(numBytes > maxLen)						//�ڴ泤���ж�
						return 0;
				}
			}
			
			dataPtr[numBytes] = '\0';							//������','�滻Ϊ������

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
				if(streamArray[count].flag && streamArray[count].dataType != TYPE_GPS)	//���ʹ�ܷ��ͱ�־λ ��ʽ5��֧��GPS
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
					if(numBytes > maxLen)						//�ڴ泤���ж�
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
//	�������ƣ�	 DSTREAM_GetDataStream_Body_Measure
//
//	�������ܣ�	������ǰʹ�ܵ�����������
//
//	��ڲ�����	type����ʽ����
//				streamArray���������ṹ
//				streamArrayCnt������������
//				flag��1-����ȫ������������	0-������ǰ��Ҫ���͵�����������
//
//	���ز�����	Body�ĳ���
//
//	˵����		
//==========================================================
short DSTREAM_GetDataStream_Body_Measure(unsigned char type, DATA_STREAM *streamArray, unsigned short streamArrayCnt, _Bool flag)
{

	short count = 0, numBytes = 0;						//count-ѭ��������numBytes-��¼����װ�س���
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
