/**
  *********************************************************************
  * @file    gui_drv_cfg.h
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   emXGUI驱动配置文件
  *          包含显示器、输入设备、存储器、字体、资源设备等内容
  *********************************************************************
  * @attention
  * 官网    :www.emXGUI.com
  *
  **********************************************************************
  */ 
#ifndef	__GUI_DRV_CFG_H__
#define	__GUI_DRV_CFG_H__


/*===========显示器配置===gui_lcd_port.c===============================================*/
//野火5.0 / 7.0TFT,800x480

/* 显存基地址 */
#define  LCD_FRAME_BUFFER   LCD_BUFFER

/* 使用的显示格式 */
#define	LCD_FORMAT	  COLOR_FORMAT_RGB565
//#define	LCD_FORMAT	  COLOR_FORMAT_XRGB8888

/* 液晶宽高 */
#define	LCD_XSIZE	    LCD_PIXEL_WIDTH
#define	LCD_YSIZE	    LCD_PIXEL_HEIGHT

//屏幕旋转，默认
#define	LCD_ROTATE      ROTATE_0

/* 是否使用硬件图形加速器（DMA2D之类，驱动需要重新定制） */
#define	DMA2D_EN                   1

/* 是否使用缓冲型控件，使用的话会增加一层液晶像素数据的消耗，显示效果减少闪烁*/
#define  FRAME_BUFFER_EN            1

/*==========输入设备配置===gui_input_port.c==================================================*/
/* 是否使用输入设备 */
#define  GUI_INPUT_DEV_EN        1

/* 是否使能键盘、鼠标、触摸屏 */
#define	GUI_TOUCHSCREEN_EN      1
#define	GUI_KEYBOARD_EN          0
#define	GUI_MOUSE_EN             0


/* 是否需要触摸校准-电阻屏才需要 */
#define	GUI_TOUCHSCREEN_CALIBRATE       0

/*===========存储器配置===gui_mem_port.c===============================================*/
/* 通常GUI内核使用MEM内存堆，VMEM内存堆给用户使用 */

/* GUI内核使用的存储区的基地址 */  
//#define	GUI_CORE_MEM_BASE	  0xD0100000  //本例子使用RTT管理，使用内部sram，不指定地址

/* GUI内核使用的存储区大小，推荐最小值为8KB */
#define  GUI_CORE_MEM_SIZE  (100*1024) //本例子使用RTT管理，在board.c实现
/* 最小分配粒度，单位为字节*/  
#define	GUI_CORE_MEM_ALLOC_UNIT   (64)         


/* 配置vmem的基地址，大小以及分配粒度 */
/* 是否使能VMEM内存堆 */
#define  GUI_VMEM_EN      1

/* 液晶驱动显存使用的空间 */
#if (LCD_FORMAT == COLOR_FORMAT_RGB565)

  #define LCD_FRAME_SIZE  (LCD_XSIZE*LCD_YSIZE*2)

#elif (LCD_FORMAT == COLOR_FORMAT_XRGB8888)

  #define LCD_FRAME_SIZE (LCD_XSIZE*LCD_YSIZE*4)

#endif
/*
SDRAM 基地址：0xD0000000
前 LCD_FRAME_SIZE 大小的空间作为液晶驱动显存
后200KB空间作为EXRAM节区用于全局变量的空间，详见sct分散加载文件
中间剩余的空间作为VMEM动态分配使用
*/
/* 内存堆的基地址，可以为内部SRAM、外扩的SDRAM等 */  
#define	VMEM_BASE	        (0xD0000000 + LCD_FRAME_SIZE)
/* 内存堆的总大小，单位为字节 */ 
#define	VMEM_SIZE	        10*1024*1024     
/* 最小分配粒度，单位为字节*/  
#define	VMEM_ALLOC_UNIT   (64)         //64字节   

//设置变量定义到“EXRAM”节区的宏
#define __EXRAM  __attribute__ ((section ("EXRAM")))


/*===========存储器配置===GUI_Arch.c===============================================*/
/** 
 在静态内存中最大的RECT_LL 链表数，
 使用静态内存链表可以极快速的分配，以提升系统性能
 如果这个静态 RECT_LL 链表用完，就会调用 GUI_MEM_Alloc，
 以动态内存方式分配 RECT_LL 
 这部分在GUI_RectLL_Alloc函数实现

 如果用户窗口不多，并且系统静态 RAM又比较少时，
 就可以把这个表定义小一点
 
 一个RECT_LL元素20字节，总的静态空间占用为 20*RECT_LL_SLOT_NUM 个字节
*/
#define	RECT_LL_SLOT_NUM	        512

/*===========字体配置===gui_font_port.c===============================================*/
/* 默认字体使用XFT，保留，这个宏不起作用 */
#define	GUI_FONT_XFT_EN		      1
/* 是否支持TTF字体，保留，这个宏不起作用 */
#define	GUI_FONT_TTF_EN		      0

/* 是否支持ICON LOGO字体，使能时需要添加相应的字体数据文件 */
#define  GUI_ICON_LOGO_EN         1

#if 0
  /* LOGO使用的字体 */
  #define  GUI_LOGO_FONT          LOGO_50_4BPP
  /* ICON使用的字体 */
  #define   GUI_ICON_FONT_100         ICON_100_4BPP
  /* 控制ICON使用的字体 */
  #define   GUI_CONTROL_FONT_64      CONTROL_60_8BPP
#else
  /* LOGO使用的字体 */
  #define  GUI_LOGO_FONT               "LOGO_50_4BPP.xft"
  #define  GUI_ICON_FONT_200          "logo_icon_200_200_4BPP.xft"
  /* ICON使用的字体 */
  #define   GUI_ICON_FONT_100         "APP_ICON_100_100_4BPP.xft"
  #define   GUI_ICON_FONT_252         "app_icon_252_252_4BPP.xft"
  /* 控制ICON使用的字体 */
  #define   GUI_CONTROL_FONT_48      "CONTROL_ICON_48_48_4BPP.xft"
  #define   GUI_CONTROL_FONT_64      "CONTROL_ICON_64_64_4BPP.xft"
  #define   GUI_CONTROL_FONT_72      "CONTROL_ICON_72_72_4BPP.xft"
  #define   GUI_CONTROL_FONT_100     "CONTROL_ICON_100_100_4BPP.xft"
  #define   GUI_CONTROL_FONT_32      "CONTROL_ICON_32_32_4BPP.xft"
#endif


/* 是否使用外部FLASH中的字体
*  流设备和整体加载方式都要把这个宏设置为1
*/
#define GUI_USE_EXTERN_FONT       1

/*
* 是否把整个外部字体数据加载至VMEM区域，初始化加载时需要较长时间，
* 加载后可大幅提高字符显示的速度
* 若设置为真，则使用整体加载方式，否则使用流设备方式
*/
#define GUI_FONT_LOAD_TO_RAM    (1 && GUI_USE_EXTERN_FONT)

/* 要使用的外部默认字体文件，USE_EXTERN_FONT为1时生效 */
#define GUI_DEFAULT_EXTERN_FONT   "GB2312_24_4BPP.xft"

/* 默认内部字体数组名，USE_EXTERN_FONT为0或 外部字体加载失败时会采用的字体 */
#define GUI_DEFAULT_FONT          ASCII_24_4BPP



/*===========日志输出设备配置===gui_log_port.c===============================================*/

/* 是否开启调试输出、数组输出、进入函数的输出功能 */
#define GUI_DEBUG_EN         	      1
#define GUI_DEBUG_ARRAY_EN          0
#define GUI_DEBUG_FUNC_EN   	       0



/*===========资源设备配置===gui_resource_port.c===============================================*/
/* 是否使用资源设备 */
#define GUI_RES_DEV_EN         1

/* 是否支持文件系统接口,需要移植fatfs文件系统 */
#define GUI_RES_FS_EN         1

/* 资源所在的基地址 */
#define GUI_RES_BASE             4096

/* 存储在FLASH中的资源目录大小 */
#define GUI_CATALOG_SIZE         (8*1024)

/*===========图片接口配置===gui_picture_port.c===============================================*/
/* 是否支持文件系统图片接口,需要移植fatfs文件系统 */
#define GUI_PIC_FS_EN         1

/* 是否支持显示JPEG图片,需要添加jpeg解码库 */
#define GUI_PIC_JPEG_EN       1

/* 是否支持显示JPEG图片,需要添加png解码库 */
#define GUI_PIC_PNG_EN       0

/* 截图 */
#define GUI_PIC_CAPTURE_SCREEN_EN  ( 1 && GUI_PIC_FS_EN)

/*============================================================================*/

#endif	/*__GUI_DRV_CFG_H__*/
