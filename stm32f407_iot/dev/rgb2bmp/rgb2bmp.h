//rgb2bmp.h�ļ�
#include <stdio.h>

#define BMP_BITS 16

typedef unsigned char  BYTE;
typedef unsigned short WORD;
// BMPͼ�������˵������
/***********
    ��һ����    λͼ�ļ�ͷ
�ýṹ�ĳ����ǹ̶��ģ�Ϊ14���ֽڣ���������������£�
    2byte   ���ļ����ͣ�������0x4d42�����ַ���"BM"��
    4byte   �������ļ���С
    4byte   �������֣�Ϊ0
    4byte   �����ļ�ͷ��ʵ�ʵ�λͼͼ�����ݵ�ƫ���ֽ�����
*************/
typedef struct
{    long imageSize;
    long blank;
    long startPosition;
}BmpHead;

/*********************
�ڶ�����    λͼ��Ϣͷ
�ýṹ�ĳ���Ҳ�ǹ̶��ģ�Ϊ40���ֽڣ������������˵�����£�
    4byte   �����ṹ�ĳ��ȣ�ֵΪ40
    4byte   ��ͼ��Ŀ���Ƕ������ء�
    4byte   ��ͼ��ĸ߶��Ƕ������ء�
    2Byte   ��������1��
    2Byte   ����ʾ��ɫʱ�õ���λ�������õ�ֵΪ1(�ڰ׶�ɫͼ)��4(16ɫͼ)��8(256ɫͼ)��24(���ɫͼ)��
    4byte   ��ָ��λͼ�Ƿ�ѹ������ЧֵΪBI_RGB��BI_RLE8��BI_RLE4��BI_BITFIELDS��Windowsλͼ�ɲ���RLE4��RLE8��ѹ����ʽ��BI_RGB��ʾ��ѹ����
    4byte   ��ָ��ʵ�ʵ�λͼͼ������ռ�õ��ֽ������������µĹ�ʽ���������
     ͼ������ = Width' * Height * ��ʾÿ��������ɫռ�õ�byte��(����ɫλ��/8,24bitͼΪ3��256ɫΪ1��
     Ҫע����ǣ�������ʽ�е�biWidth'������4��������(����biWidth�����Ǵ��ڻ����biWidth����С4��������)��
     ���biCompressionΪBI_RGB����������Ϊ0��
    4byte   ��Ŀ���豸��ˮƽ�ֱ��ʡ�
    4byte   ��Ŀ���豸�Ĵ�ֱ�ֱ��ʡ�
    4byte   ����ͼ��ʵ���õ�����ɫ���������ֵΪ0�����õ�����ɫ��Ϊ2��(��ɫλ��)����,����ɫλ��Ϊ8��2^8=256,��256ɫ��λͼ
    4byte   ��ָ����ͼ������Ҫ����ɫ���������ֵΪ0������Ϊ���е���ɫ������Ҫ�ġ�
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
    ��������    ��ɫ�̽ṹ  ��ɫ��
    ����256ɫBMPλͼ����ɫλ��Ϊ8����Ҫ2^8 = 256����ɫ�̣�
    ����24bitBMPλͼ��������RGBֱֵ�ӱ�����ͼ��������������Ҫ��ɫ�̣������ڵ�ɫ����
    rgbBlue��   ����ɫ����ɫ������
    rgbGreen��  ����ɫ����ɫ������
    rgbRed��    ����ɫ�ĺ�ɫ������
    rgbReserved������ֵ��
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
