//rgb2bmp.h文件
#include <stdio.h>

#define BMP_BITS 16

typedef unsigned char  BYTE;
typedef unsigned short WORD;
// BMP图像各部分说明如下
/***********
    第一部分    位图文件头
该结构的长度是固定的，为14个字节，各个域的依次如下：
    2byte   ：文件类型，必须是0x4d42，即字符串"BM"。
    4byte   ：整个文件大小
    4byte   ：保留字，为0
    4byte   ：从文件头到实际的位图图像数据的偏移字节数。
*************/
typedef struct
{    long imageSize;
    long blank;
    long startPosition;
}BmpHead;

/*********************
第二部分    位图信息头
该结构的长度也是固定的，为40个字节，各个域的依次说明如下：
    4byte   ：本结构的长度，值为40
    4byte   ：图像的宽度是多少象素。
    4byte   ：图像的高度是多少象素。
    2Byte   ：必须是1。
    2Byte   ：表示颜色时用到的位数，常用的值为1(黑白二色图)、4(16色图)、8(256色图)、24(真彩色图)。
    4byte   ：指定位图是否压缩，有效值为BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS。Windows位图可采用RLE4和RLE8的压缩格式，BI_RGB表示不压缩。
    4byte   ：指定实际的位图图像数据占用的字节数，可用以下的公式计算出来：
     图像数据 = Width' * Height * 表示每个象素颜色占用的byte数(即颜色位数/8,24bit图为3，256色为1）
     要注意的是：上述公式中的biWidth'必须是4的整数倍(不是biWidth，而是大于或等于biWidth的最小4的整数倍)。
     如果biCompression为BI_RGB，则该项可能为0。
    4byte   ：目标设备的水平分辨率。
    4byte   ：目标设备的垂直分辨率。
    4byte   ：本图像实际用到的颜色数，如果该值为0，则用到的颜色数为2的(颜色位数)次幂,如颜色位数为8，2^8=256,即256色的位图
    4byte   ：指定本图像中重要的颜色数，如果该值为0，则认为所有的颜色都是重要的。
***********************************/
typedef struct
  
{
    long    Length;
    long    width;
    long    height;
    WORD    colorPlane;
    WORD    bitColor;
    long    zipFormat;
    long    realSize;
    long    xPels;
    long    yPels;
    long    colorUse;
    long    colorImportant;
  /*  void show()
  
    {    
        printf("infoHead Length:%dn",Length);
        printf("width&height:%d*%dn",width,height);
        printf("colorPlane:%dn",colorPlane);
        printf("bitColor:%dn",bitColor);
        printf("Compression Format:%dn",zipFormat);
        printf("Image Real Size:%dn",realSize);
        printf("Pels(X,Y):(%d,%d)n",xPels,yPels);
        printf("colorUse:%dn",colorUse);    
        printf("Important Color:%dn",colorImportant);
    }*/
}InfoHead;
/***************************
    第三部分    调色盘结构  颜色表
    对于256色BMP位图，颜色位数为8，需要2^8 = 256个调色盘；
    对于24bitBMP位图，各象素RGB值直接保存在图像数据区，不需要调色盘，不存在调色盘区
    rgbBlue：   该颜色的蓝色分量。
    rgbGreen：  该颜色的绿色分量。
    rgbRed：    该颜色的红色分量。
    rgbReserved：保留值。
************************/
typedef struct
{         BYTE   rgbBlue;
         BYTE   rgbGreen;
         BYTE   rgbRed;
         BYTE   rgbReserved;
      /*   void show(void)
         {
            printf("Mix Plate B,G,R:%d %d %dn",rgbBlue,rgbGreen,rgbRed);
         }*/
}RGBMixPlate;

void rgb565tobmp(char *rgb_buffer, unsigned short nWidth, unsigned short nHeight);
