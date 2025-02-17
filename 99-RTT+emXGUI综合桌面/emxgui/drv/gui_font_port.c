/**
  *********************************************************************
  * @file    gui_font_port.c
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   字体管理接口
  *********************************************************************
  * @attention
  * 官网    :www.emXGUI.com
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
  * @note  若要把字体加入到内部FLASH，
  * 把相应的字体数组文件添加到工程，然后在此处加入声明
  * 并使用XFT_CreateFont创建字体
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

/* 默认字体 */
HFONT defaultFont =NULL;

/* 默认英文字体 */
HFONT defaultFontEn = NULL;

#if(GUI_ICON_LOGO_EN)  
/* logo字体 */
HFONT logoFont =NULL;
HFONT logoFont_200 =NULL;
/* 图标字体 */
HFONT iconFont_100 =NULL;
HFONT iconFont_200 =NULL;
HFONT iconFont_252 =NULL;
/* 控制图标字体 */
HFONT controlFont_32 =NULL;
HFONT controlFont_48 =NULL;
HFONT controlFont_64 =NULL;
HFONT controlFont_72 =NULL;
HFONT controlFont_100 =NULL;
#endif

/* 其它 */
HFONT GB2312_32_Font =NULL;

/* 用于标记是否有资源文件无法找到 */
BOOL res_not_found_flag = FALSE;


extern HWND GUI_Boot_hwnd;

/*===================================================================================*/
#if (GUI_USE_EXTERN_FONT)

/**
  * @brief  从流媒体加载内容的回调函数
  * @param  buf[out] 存储读取到的数据缓冲区
  * @param  offset 要读取的位置
  * @param  size 要读取的数据大小
  * @param  lParam 调用函数时的自定义参数（用户参数）
  * @retval 读取到的数据大小
  */
static int font_read_data_exFlash(void *buf,int offset,int size,LONG lParam)
{
  /* 本例子中offset是具体字符数据在字体文件中的偏移 
   * lParam 是字体文件在FLASH中的基地址
  */
	offset += lParam;
  
  /* 读取具体的字模数据内容 */
	RES_DevRead(buf,offset,size);
	return size;
}


/**
  * @brief  初始化外部FLASH字体(流设备方式)
  * @param  res_name 字体资源名字
  * @retval 返回默认字体的句柄
  */
HFONT GUI_Init_Extern_Font(const char* res_name)
{
  /* 使用流设备加载字体，按需要读取 */
  int font_base;
  HFONT hFont = NULL;
  CatalogTypeDef dir;


  if(Boot_progbar != NULL)
  {  
    int count = 0;

    /* 更新启动界面的进度条 */
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
  * @brief  初始化外部FLASH字体（整体加载到SDRAM）
  * @param  res_name[in] 字体资源名字
  * @param  buf[out]：字体资源复制到的缓冲区，
  *         可以通过它free掉占用的空间，但注意释放后使用字体的话会出错
  * @retval 返回默认字体的句柄
  */
HFONT GUI_Init_Extern2RAM_Font(const char* res_name,u8** buf)
{
    /* 整个字体文件加载至RAM */
    int font_base; 
    HFONT hFont = NULL;  
    CatalogTypeDef dir;
        

    if(Boot_progbar != NULL)
    {    
      int count = 0;

      /* 更新启动界面的进度条 */
      count = SendMessage(Boot_progbar,PBM_GET_VALUE,TRUE,NULL); 
      count++;
      SendMessage(Boot_progbar,PBM_SET_VALUE,TRUE,count); 
    }
    
    /* RES_GetInfo读取到的dir.offset是资源的绝对地址 */
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
  * @brief  加载外部字体
  * @retval 返回重新创建的defaultFont
  */
HFONT GUI_Extern_FontInit(void)
{
   /* 整个字体文件加载至RAM */
#if (GUI_FONT_LOAD_TO_RAM  )
  {  
    defaultFont = GUI_Init_Extern2RAM_Font(GUI_DEFAULT_EXTERN_FONT,&default_font_buf);
    //GUI_msleep(10);
  #if(GUI_ICON_LOGO_EN)  
   {
    /* 创建logo字体 */  
    logoFont =  GUI_Init_Extern2RAM_Font(GUI_LOGO_FONT,&logo_font_buf);
    
    logoFont_200 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_200,&logo_font_buf_200);
    /* 创建图标字体 */  
    iconFont_100 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_100,&icon_font_100_buf);
    iconFont_252 =  GUI_Init_Extern2RAM_Font(GUI_ICON_FONT_252,&icon_font_252_buf);   
    /* 创建图标字体 */
    controlFont_32 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_32,&control_font_32_buf);      
    /* 创建控制图标字体 */  
    controlFont_48 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_48,&control_font_48_buf); 
    /* 创建控制图标字体 */  
    controlFont_64 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_64,&control_font_64_buf); 
    /* 创建控制图标字体 */  
    controlFont_72 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_72,&control_font_72_buf); 
    controlFont_100 =  GUI_Init_Extern2RAM_Font(GUI_CONTROL_FONT_100,&control_font_100_buf);
   }
  #endif
  }
  
#else

   /* 使用流设备加载字体，按需要读取 */
  {
    defaultFont =GUI_Init_Extern_Font(GUI_DEFAULT_EXTERN_FONT);
        
  #if(GUI_ICON_LOGO_EN)  
    {
    /* 创建logo字体 */  
    logoFont =  GUI_Init_Extern_Font(GUI_LOGO_FONT);
    logoFont_200 =  GUI_Init_Extern_Font(GUI_ICON_FONT_200);

    /* 创建图标字体 */  
    iconFont_100 =  GUI_Init_Extern_Font(GUI_ICON_FONT_100);      
    iconFont_252 =  GUI_Init_Extern_Font(GUI_ICON_FONT_252); 
    /* 创建控制图标字体 */             
    controlFont_32 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_32); 
    /* 创建控制图标字体 */
    controlFont_48 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_48); 
    /* 创建控制图标字体 */  
    controlFont_64 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_64); 
    /* 创建控制图标字体 */  
    controlFont_72 =  GUI_Init_Extern_Font(GUI_CONTROL_FONT_72); 
    }
  #endif
  }
#endif 
 
  return defaultFont;
}

/**
  * @brief  GUI默认字体初始化
  * @param  无
  * @retval 返回默认字体的句柄
  */
HFONT GUI_Default_FontInit(void)
{
    /* 若前面的字体加载失败，使用内部FLASH中的数据（工程中的C语言数组）
    *  添加字体数据时，把数组文件添加到工程，在本文件头添加相应字体数组的声明，
    *  然后调用XFT_CreateFont函数创建字体即可
    */
  
    /* 从本地加载(本地数组数据) */ 
    /*ASCii字库,24x24,4BPP抗锯齿*/
    defaultFontEn = XFT_CreateFont(GUI_DEFAULT_FONT);
  
    if(defaultFont==NULL)
    { 
      defaultFont = defaultFontEn;  /*ASCii字库,20x20,4BPP抗锯齿*/
      
      /* 中文字库存储占用空间非常大，不推荐放在内部FLASH */
    	//defaultFont =XFT_CreateFont(GB2312_16_2BPP); /*GB2312字库,16x16,2BPP抗锯齿*/
    	//defaultFont =XFT_CreateFont(GB2312_20_4BPP); /*GB2312字库,20x20,4BPP抗锯齿*/
    }
    
	return defaultFont;
}

/********************************END OF FILE****************************/

