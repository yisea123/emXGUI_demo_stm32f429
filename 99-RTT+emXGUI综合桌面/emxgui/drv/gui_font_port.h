#ifndef __GUI_FONT_PORT_H
#define	__GUI_FONT_PORT_H

#ifdef __cplusplus
extern "C" {
#endif 
  
#include	"emXGUI.h"
#include "stm32f4xx.h"
#include	"gui_drv_cfg.h"

/* Ĭ������ */
extern HFONT defaultFont;
/* Ĭ��Ӣ������ */
extern HFONT defaultFontEn;

/* logo���� */
extern HFONT logoFont;
/* ͼ������ */
extern HFONT iconFont_100;
extern HFONT iconFont_252;
/* ����ͼ������ */
extern HFONT controlFont_48;
extern HFONT controlFont_64;
extern HFONT controlFont_72;
extern HFONT controlFont_32;
extern HFONT controlFont_100;
/* ���� */
extern HFONT GB2312_32_Font;
extern HFONT iconFont_200;
extern HFONT logoFont_200;

HFONT GUI_Default_FontInit(void);
HFONT GUI_Init_Extern_Font(const char* res_name);
HFONT GUI_Init_Extern2RAM_Font(const char* res_name,u8** buf);
HFONT GUI_Extern_FontInit(void);
#ifdef	__cplusplus
}
#endif

#endif /* __GUI_FONT_PORT_H */
