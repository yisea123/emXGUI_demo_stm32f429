/*******************************************************************************
***************************** ��MP3�е�ר��ͼƬ ********************************
*******************************************************************************/

#include "MP3_GET_Image.h"
#include <string.h>

/**
  * @brief  ��ȡMP3ID3V2�ļ�ͷ�Ĵ�С
  * @param  ����MP3�ļ���ͷ�����ݣ�����10���ֽ�
  * @retval ID3V2�Ĵ�С
  */
static uint32_t mp3_Header_Size(unsigned char *buf)
{
 uint32_t ID3V2_size;
	
 if(buf[0] == 'I' && buf[1] == 'D' && buf[2] =='3')//����ID3V2
 {
 	 ID3V2_size = (buf[6]<<21) | (buf[7]<<14) | (buf[8]<<7) | buf[9];
 }
 else//������ID3V2
	 ID3V2_size = 0;

 return ID3V2_size;
}

/**
  * @brief  ��ȡ MP3ID3V2 �ļ�ͷ�Ĵ�С
* @param  buf����һ��֡��ǩ string��֡��ʶ
  * @retval ����֡��ʶ��ָ��λ��
  */
uint8_t *MP3_FrameId_Get(unsigned char *buf, uint32_t File_Header,char *string)
{
  uint8_t *FrameId;
  uint8_t databuff;
  uint8_t *start = buf;

  while(1)
  {
//    databuff = *buf;
//    *(buf+4) = '\0';
    
    if (strstr((char *)buf, string))
    {
      FrameId = buf;
      
      return FrameId;
    }
    
//    printf("%s\n", buf);
//    *(buf+4) = databuff;
    
    buf += (*(buf+4) << 24) + (*(buf+5) << 16) + (*(buf+6) << 8) + (*(buf+7)) + 10;    // ������һ֡�ı�ǩ
    
    if (buf-start > File_Header-10)
    {
      break;
    }
  }

  return NULL;
}

/**
  * @brief  ��ȡMP3 ר��ͼƬ����Ϣ
  * @param  ����MP3�ļ���ͷ�����ݣ������ļ�ͷ
  * @retval ��
  */
void Get_Imare_Info(uint8_t *data, MP3_Image_t *Image_Info)
{
  uint32_t File_Header;
  uint8_t buff;
  uint8_t *APIC;
  
  File_Header = mp3_Header_Size(data);    // ��ȡ�ļ�ͷ�Ĵ�С
  
  APIC = MP3_FrameId_Get((uint8_t *)(data+10), File_Header, "APIC");
  
  if (APIC != NULL)    // ��ͼƬ
  {
    Image_Info->Flag = 1;    // û���ҵ�ͼƬ
    Image_Info->Size = (APIC[4] << 24 | APIC[5] << 24 | APIC[6] << 24 | APIC[7]) - 14;    // ����ͼƬ���ݵĴ�С
    Image_Info->Offset = (APIC + 24);                                   // ����ͼƬ��ƫ��λ��
  }
  else
  {
    Image_Info->Flag = NULL;    // û���ҵ�ͼƬ
    
    return;
  }
}

/*****************************File End*****************************************/
