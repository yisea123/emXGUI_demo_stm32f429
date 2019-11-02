/**
  *********************************************************************
  * @file    gui_font_port.c
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   �������ӿ�
  *********************************************************************
  * @attention
  * ����    :www.emXGUI.com
  *
  **********************************************************************
  */ 

#include	"emXGUI.h"
#include  "gui_drv_cfg.h"

#include "gui_mem_port.h"
#include "gui_font_port.h"
#include "gui_resource_port.h"

/*===================================================================================*/
/*===================================================================================*/


#include <GUI_Font_XFT.h>

/**
  * @note  ��Ҫ��������뵽�ڲ�FLASH��
  * ����Ӧ�����������ļ���ӵ����̣�Ȼ���ڴ˴���������
  * ��ʹ��XFT_CreateFont��������
  */
extern const char GB2312_16_1BPP[];
extern const char GB2312_16_2BPP[];
extern const char GB2312_20_2BPP[];

extern const char GB2312_16_4BPP[];
extern const char GB2312_20_4BPP[];

extern const char ASCII_16_4BPP[];
extern const char ASCII_20_4BPP[];
extern const char ASCII_24_4BPP[];
extern const char ASCII_32_4BPP[];
extern const char LOGO_50_4BPP[];
extern const char ICON_100_4BPP[];
extern const char CONTROL_50_4BPP[];
extern const char CONTROL_60_8BPP[];
extern const char CONTROL_70_8BPP[];
extern const char CONTROL_80_8BPP[];
extern const char app_icon_200_200_4BPP[];
extern HWND Boot_progbar;

/* Ĭ������ */
HFONT defaultFont =NULL;

/* Ĭ��Ӣ������ */
HFONT defaultFontEn = NULL;

#if(GUI_ICON_LOGO_EN)  
/* logo���� */
HFONT logoFont =NULL;
HFONT logoFont_200 =NULL;
/* ͼ������ */
HFONT iconFont_100 =NULL;
HFONT iconFont_200 =NULL;
HFONT iconFont_252 =NULL;
/* ����ͼ������ */
HFONT controlFont_32 =NULL;
HFONT controlFont_48 =NULL;
HFONT controlFont_64 =NULL;
HFONT controlFont_72 =NULL;
HFONT controlFont_100 =NULL;
#endif

/* ���� */
HFONT GB2312_32_Font =NULL;

/* ���ڱ���Ƿ�����Դ�ļ��޷��ҵ� */
BOOL res_not_found_flag = FALSE;


extern HWND GUI_Boot_hwnd;

/*===================================================================================*/
#if (GUI_USE_EXTERN_FONT)

/**
  * @brief  ����ý��������ݵĻص�����
  * @param  buf[out] �洢��ȡ�������ݻ�����
  * @param  offset Ҫ��ȡ��λ��
  * @param  size Ҫ��ȡ�����ݴ�С
  * @param  lParam ���ú���ʱ���Զ���������û�������
  * @retval ��ȡ�������ݴ�С
  */
static int font_read_data_exFlash(void *buf,int offset,int size,LONG lParam)
{
  /* ��������offset�Ǿ����ַ������������ļ��е�ƫ�� 
   * lParam �������ļ���FLASH�еĻ���ַ
  */
	offset += lParam;
  
  /* ��ȡ�������ģ�������� */
	RES_DevRead(buf,offset,size);
	return size;
}


/**
  * @brief  ��ʼ���ⲿFLASH����(���豸��ʽ)
  * @param  res_name ������Դ����
  * @retval ����Ĭ������ľ��
  */
HFONT GUI_Init_Extern_Font(const char* res_name)
{
  /* ʹ�����豸�������壬����Ҫ��ȡ */
  int font_base;
  HFONT hFont = NULL;
  CatalogTypeDef dir;


  if(Boot_progbar != NULL)
  {  
    int count = 0;

    /* ������������Ľ����� */
    count = SendMessage(Boot_progbar,PBM_GET_VALUE,TRUE,NULL); 
    count++;
    SendMessage(Boot_progbar,PBM_SET_VALUE,TRUE,count); 
  }
  
  font_base =RES_GetInfo_AbsAddr(res_name, &dir);
  if(font_base > 0)
  {
    hFont =XFT_CreateFontEx(font_read_data_exFlash,font_base);
  }
  else
  {
    res_not_found_flag = TRUE;
    GUI_ERROR("Can not find RES:%s",res_name);
  }
  
  if(hFont==NULL)
  {
    res_not_found_flag = TRUE;    
    GUI_ERROR("%s font create failed",res_name);
  }
  return hFont;
}

#endif


/**
  * @brief  ��ʼ���ⲿFLASH���壨������ص�SDRAM��
  * @param  res_name[in] ������Դ����
  * @param  buf[out]��������Դ���Ƶ��Ļ�������
  *         ����ͨ����free��ռ�õĿռ䣬��ע���ͷź�ʹ������Ļ������
  * @retval ����Ĭ������ľ��
  */
HFONT GUI_Init_Extern2RAM_Font(const char* res_name,u8** buf)
{
    /* ���������ļ�������RAM */
    int font_base; 
    HFONT hFont = NULL;  
    CatalogTypeDef dir;
        

    if(Boot_progbar != NULL)
    {    
      int count = 0;

      /* ������������Ľ����� */
      count = SendMessage(Boot_progbar,PBM_GET_VALUE,TRUE,NULL); 
      count++;
      SendMessage(Boot_progbar,PBM_SET_VALUE,TRUE,count); 
    }
    
    /* RES_GetInfo��ȡ����dir.offset����Դ�ľ��Ե�ַ */
    font_base =RES_GetInfo_AbsAddr(res_name, &dir);

    if(font_base > 0)
    {
    	*buf =(u8*)GUI_VMEM_Alloc(dir.size);
      if(*buf!=NULL)
      {
        RES_DevRead((u8 *)*buf, font_base, dir.size);

        hFont = XFT_CreateFont(*buf);
      }
    }
    else
    {
      res_not_found_flag = TRUE;
      GUI_ERROR("Can not find RES:%s",res_name);
    }
    
    if(hFont==NULL)
    {
      res_not_found_flag = TRUE;    
      GUI_ERROR("%s font create failed",res_name);
    }
    
   return hFont;
}

#if (GUI_FONT_LOAD_TO_RAM)

  u8 *default_font_buf;

  #if(GUI_ICON_LOGO_EN)  
    u8 *logo_font_buf;
    u8 *logo_font_buf_200;
    u8 *icon_font_100_buf;
    u8 *icon_font_252_buf;
    u8 *control_font_32_buf;
    u8 *control_font_48_buf;
    u8 *control_font_64_buf;
    u8 *control_font_72_buf;
    u8 *control_font_100_buf;
  #endif
#endif


/**
  * @brief  �����ⲿ����
  * @retval �������´�����defaultFont
  */
HFONT GUI_Extern_FontInit(void)
{
   /* ���������ļ�������RAM */
#if (GUI_FONT_LOAD_TO_RAM  )
  {  
    defaultFont = GUI_Init_Extern2RAM_Font(GUI_DEFAULT_EXTERN_FONT,&default_font_buf);
    //GUI_msleep(10);
  #if(GUI_ICON_LOGO_EN)  
   {
    /* ����logo���� */  
    logoFont =  GUI_Init_Extern2RAM_Font(GUI_LOGO_FONT,&logo_font_buf);
    
    logoFont_200 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_200,&logo_font_buf_200);
    /* ����ͼ������ */  
    iconFont_100 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_100,&icon_font_100_buf);
    iconFont_252 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_252,&icon_font_252_buf);   
    /* ����ͼ������ */
    controlFont_32 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_32,&control_font_32_buf);      
    /* ��������ͼ������ */  
    controlFont_48 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_48,&control_font_48_buf); 
    /* ��������ͼ������ */  
    controlFont_64 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_64,&control_font_64_buf); 
    /* ��������ͼ������ */  
    controlFont_72 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_72,&control_font_72_buf); 
    controlFont_100 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_100,&control_font_100_buf);
   }
  #endif
  }
  
#else

   /* ʹ�����豸�������壬����Ҫ��ȡ */
  {
    defaultFont =GUI_Init_Extern_Font(GUI_DEFAULT_EXTERN_FONT);
        
  #if(GUI_ICON_LOGO_EN)  
    {
    /* ����logo���� */  
    logoFont =  GUI_Init_Extern_Font(GUI_LOGO_FONT);
    logoFont_200 =  GUI_Init_Extern_Font(GUI_ICON_FONT_200);

    /* ����ͼ������ */  
    iconFont_100 =  GUI_Init_Extern_Font(GUI_ICON_FONT_100);      
    iconFont_252 =  GUI_Init_Extern_Font(GUI_ICON_FONT_252); 
    /* ��������ͼ������ */             
    controlFont_32 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_32); 
    /* ��������ͼ������ */
    controlFont_48 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_48); 
    /* ��������ͼ������ */  
    controlFont_64 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_64); 
    /* ��������ͼ������ */  
    controlFont_72 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_72); 
    }
  #endif
  }
#endif 
 
  return defaultFont;
}

/**
  * @brief  GUIĬ�������ʼ��
  * @param  ��
  * @retval ����Ĭ������ľ��
  */
HFONT GUI_Default_FontInit(void)
{
    /* ��ǰ����������ʧ�ܣ�ʹ���ڲ�FLASH�е����ݣ������е�C�������飩
    *  �����������ʱ���������ļ���ӵ����̣��ڱ��ļ�ͷ�����Ӧ���������������
    *  Ȼ�����XFT_CreateFont�����������弴��
    */
  
    /* �ӱ��ؼ���(������������) */ 
    /*ASCii�ֿ�,24x24,4BPP�����*/
    defaultFontEn = XFT_CreateFont(GUI_DEFAULT_FONT);
  
    if(defaultFont==NULL)
    { 
      defaultFont = defaultFontEn;  /*ASCii�ֿ�,20x20,4BPP�����*/
      
      /* �����ֿ�洢ռ�ÿռ�ǳ��󣬲��Ƽ������ڲ�FLASH */
    	//defaultFont =XFT_CreateFont(GB2312_16_2BPP); /*GB2312�ֿ�,16x16,2BPP�����*/
    	//defaultFont =XFT_CreateFont(GB2312_20_4BPP); /*GB2312�ֿ�,20x20,4BPP�����*/
    }
    
	return defaultFont;
}

/********************************END OF FILE****************************/

