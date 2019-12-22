//Ö÷º¯Êý
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rgb2bmp.h"


static void RGBtoBMP(char *bmp_buffer, int nWidth, int nHeight, char bits)
{
	BmpHead m_BMPHeader;
	char bfType[2] = { 'B','M' };
	m_BMPHeader.imageSize = bits * nWidth*nHeight + 54;
	m_BMPHeader.blank = 0;
	m_BMPHeader.startPosition = 54;

	memcpy(bmp_buffer, bfType, sizeof(bfType));
	bmp_buffer += sizeof(bfType);
	memcpy(bmp_buffer, &m_BMPHeader.imageSize, sizeof(m_BMPHeader.imageSize));
	bmp_buffer += sizeof(m_BMPHeader.imageSize);
	memcpy(bmp_buffer, &m_BMPHeader.blank, sizeof(m_BMPHeader.blank));
	bmp_buffer += sizeof(m_BMPHeader.blank);
	memcpy(bmp_buffer, &m_BMPHeader.startPosition, sizeof(m_BMPHeader.startPosition));
	bmp_buffer += sizeof(m_BMPHeader.startPosition);

	InfoHead  m_BMPInfoHeader;
	m_BMPInfoHeader.Length = 40;
	m_BMPInfoHeader.width = nWidth;
	m_BMPInfoHeader.height = nHeight;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = BMP_BITS;
	m_BMPInfoHeader.zipFormat = 0;
	m_BMPInfoHeader.realSize = bits * nWidth*nHeight;
	m_BMPInfoHeader.xPels = 2835;
	m_BMPInfoHeader.yPels = 2835;
	m_BMPInfoHeader.colorUse = 0;
	m_BMPInfoHeader.colorImportant = 0;

	memcpy(bmp_buffer, &m_BMPInfoHeader.Length, sizeof(m_BMPInfoHeader.Length));
	bmp_buffer += sizeof(m_BMPInfoHeader.Length);
	memcpy(bmp_buffer, &m_BMPInfoHeader.width, sizeof(m_BMPInfoHeader.width));
	bmp_buffer += sizeof(m_BMPInfoHeader.width);
	memcpy(bmp_buffer, &m_BMPInfoHeader.height, sizeof(m_BMPInfoHeader.height));
	bmp_buffer += sizeof(m_BMPInfoHeader.height);
	memcpy(bmp_buffer, &m_BMPInfoHeader.colorPlane, sizeof(m_BMPInfoHeader.colorPlane));
	bmp_buffer += sizeof(m_BMPInfoHeader.colorPlane);
	memcpy(bmp_buffer, &m_BMPInfoHeader.bitColor, sizeof(m_BMPInfoHeader.bitColor));
	bmp_buffer += sizeof(m_BMPInfoHeader.bitColor);
	memcpy(bmp_buffer, &m_BMPInfoHeader.zipFormat, sizeof(m_BMPInfoHeader.zipFormat));
	bmp_buffer += sizeof(m_BMPInfoHeader.zipFormat);
	memcpy(bmp_buffer, &m_BMPInfoHeader.realSize, sizeof(m_BMPInfoHeader.realSize));
	bmp_buffer += sizeof(m_BMPInfoHeader.realSize);
	memcpy(bmp_buffer, &m_BMPInfoHeader.xPels, sizeof(m_BMPInfoHeader.xPels));
	bmp_buffer += sizeof(m_BMPInfoHeader.xPels);
	memcpy(bmp_buffer, &m_BMPInfoHeader.yPels, sizeof(m_BMPInfoHeader.yPels));
	bmp_buffer += sizeof(m_BMPInfoHeader.yPels);
	memcpy(bmp_buffer, &m_BMPInfoHeader.colorUse, sizeof(m_BMPInfoHeader.colorUse));
	bmp_buffer += sizeof(m_BMPInfoHeader.colorUse);
	memcpy(bmp_buffer, &m_BMPInfoHeader.colorImportant, sizeof(m_BMPInfoHeader.colorImportant));
	bmp_buffer += sizeof(m_BMPInfoHeader.colorImportant);
}


void rgb565tobmp(char *rgb_buffer, unsigned short nWidth, unsigned short nHeight)
{    
	char bits = BMP_BITS / 8;
	int i = 0;
	int j = 0;
	unsigned char R,G,B;
	unsigned short BGR,RGB;
	char *rgb_buff;
	
	rgb_buff = rgb_buffer;
	
	rgb_buffer += 54;
	
	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			RGB = (*(rgb_buffer+1)<<8 | *rgb_buffer);
			R = RGB & 0x001f;
			G = (RGB >> 6) & 0x001f;
			B = (RGB >> 11) & 0x001f;
			BGR = (B << 10) | (G << 5) | (R);
			*rgb_buffer = BGR;
			*(rgb_buffer+1) = BGR >> 8;
			rgb_buffer += 2;
		}
	}

	RGBtoBMP(rgb_buff, nWidth, nHeight, bits);
	
}
   
