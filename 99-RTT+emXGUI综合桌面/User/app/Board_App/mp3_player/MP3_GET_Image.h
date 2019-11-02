#ifndef	__MP3_GET_Image_H__
#define	__MP3_GET_Image_H__

#include "stm32f4xx.h"

typedef struct{
  uint8_t  Flag;      // ��־�Ƿ��ҵ�ͼƬ
  uint32_t Size;      // ͼƬ�Ĵ�С
  uint8_t  Type;      // ͼƬ���� 0:jpeg 1:PNG
  uint8_t *Offset;    // ͼƬ��ƫ����ʼ��ַ
} MP3_Image_t;

void Get_Imare_Info(uint8_t *data, MP3_Image_t *Image_Info);

#endif
/*****************************File End*****************************************/
