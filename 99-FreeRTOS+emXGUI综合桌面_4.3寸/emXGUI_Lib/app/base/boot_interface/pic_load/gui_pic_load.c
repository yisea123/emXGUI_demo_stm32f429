/**
  *********************************************************************
  * @file    gui_pic_load.c
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   ͼƬ����ӿ�
  **********************************************************************
  */ 
  
#include "board.h"
#include "gui_drv.h"
#include "gui_drv_cfg.h"
#include "./pic_load/gui_pic_load.h"
#include "GUI_AppDef.h"
#include "emXGUI_JPEG.h"
#include "emxgui_png.h"
#include <string.h>

/*============================================================================*/
static HDC Load_jpg_to_hdc(char *file_name, int w, int h);
static HDC Load_png_to_hdc(char *file_name, int w, int h);
/****************************** ʱ�� App ͼƬ ********************************/
const clock_hdc_t clock_png_info[hdc_clock_end] = 
{
  {GUI_CLOCK_BTN_PIC,       100, 40,     hdc_clock_btn},
  {GUI_CLOCK_BTN_PRESS_PIC, 100, 40,     hdc_clock_btn_press},
  {GUI_CLOCK_CALENDAR_PIC,  132, 136,    hdc_clock_calendar},
  {GUI_CLOCK_00BACK_PIC,    133, 139,    hdc_clock_back_00},
  {GUI_CLOCK_00H_PIC,         3, 26+23,  hdc_clock_h_00},
  {GUI_CLOCK_00M_PIC,         3, 37+33,  hdc_clock_m_00},
  {GUI_CLOCK_00S_PIC,         5, 59+31,  hdc_clock_s_00},
  {GUI_CLOCK_01BACK_PIC,    133, 139,    hdc_clock_back_01},
  {GUI_CLOCK_01H_PIC,         3, 11+97,  hdc_clock_h_01},
  {GUI_CLOCK_01M_PIC,         9, 49+35,  hdc_clock_m_01},
  {GUI_CLOCK_01S_PIC,         3, 58+42,  hdc_clock_s_01},
  {GUI_CLOCK_02BACK_PIC,    133, 139,    hdc_clock_back_02},
  {GUI_CLOCK_02H_PIC,         9, 35+23,  hdc_clock_h_02},
  {GUI_CLOCK_02M_PIC,         9, 44+34,  hdc_clock_m_02},
  {GUI_CLOCK_02S_PIC,         4, 49+27,  hdc_clock_s_02},
  {GUI_CLOCK_CHCKED_PIC,    154, 154,    hdc_clock_chcked},

};

/* HDC */
HDC hdc_clock_bk;
HDC hdc_clock_png[hdc_clock_end];
/****************************** ʱ�� App ͼƬ END ********************************/

/****************************** �����Ǳ� App ͼƬ ********************************/
const am_hdc_t am_png_info[hdc_am_end] = 
{
  {GUI_AM_AUTOPOINTER_PIC,         23, 104+80, hdc_autopointer},
  {GUI_ABS_CHECKED_PIC,            24,  17,     hdc_ABS_checked},
  {GUI_ABS_UNCHECKED_PIC,          24,  17,     hdc_ABS_unchecked},
  {GUI_ALARM_CHECKED_PIC,          24,  17,     hdc_alarm_checked},
  {GUI_ALARM_UNCHECKED_PIC,        24,  17,     hdc_alarm_unchecked},
  {GUI_BRAKING_OPENED_PIC,         24,  17,     hdc_braking_opened},
  {GUI_BRAKING_SHUT_PIC,           24,  17,     hdc_braking_shut},
  {GUI_DOOR_OPENED_PIC,            25,  22,     hdc_door_opened},
  {GUI_DOOR_SHUT_PIC,              25,  22,     hdc_door_shut},
  {GUI_ENGINE_ALARM_PIC,           30,  23,     hdc_engine_alarm},
  {GUI_ENGINE_NORMAL_PIC,          30,  23,     hdc_engine_normal},
  {GUI_ENGINE_SHUT_PIC,            30,  23,     hdc_engine_shut},
  {GUI_ENGINE_OIL_CHECKED_PIC,     30,  23,     hdc_engine_oil_checked},
  {GUI_ENGINE_OIL_UNCHECKED_PIC,   30,  23,     hdc_engine_oil_unchecked},
  {GUI_HARNESS_CHECKED_PIC,        24,  17,     hdc_harness_checked},
  {GUI_HARNESS_UNCHECKED_PIC,      24,  17,     hdc_harness_unchecked},
  {GUI_HEADLIGHT_DISTANCE_PIC,     24,  17,     hdc_headlight_distance},
  {GUI_HEADLIGHT_OPENED_PIC,       24,  17,     hdc_headlight_opened},
  {GUI_HEADLIGHT_SHUT_PIC,         24,  17,     hdc_headlight_shut},
  {GUI_LEFT_OPENED_PIC,            24,  17,     hdc_left_opened},
  {GUI_LEFT_SHUT_PIC,              24,  17,     hdc_left_shut},
  {GUI_RIGHT_OPENED_PIC,           24,  17,     hdc_right_opened},
  {GUI_RIGHT_SHUT_PIC,             24,  17,     hdc_right_shut},
  {GUI_FOG_LAMP_OPENED,            24,  17,     hdc_fog_lamp_opened},
  {GUI_FOG_LAMP_SHUT,              24,  17,     hdc_fog_lamp_shut},
  {GUI_LEFT1_PIC,                  34,  13,     hdc_left1},
  {GUI_LEFT2_PIC,                  33,  13,     hdc_left2},
  {GUI_LEFT3_PIC,                  32,  13,     hdc_left3},
  {GUI_LEFT4_PIC,                  33,  13,     hdc_left4},
  {GUI_LEFT5_PIC,                  34,  13,     hdc_left5},
  {GUI_LEFT6_PIC,                  34,  13,     hdc_left6},
  {GUI_LEFT7_PIC,                  36,  13,     hdc_left7},
  {GUI_LEFT8_PIC,                  38,  13,     hdc_left8},
  {GUI_LEFT9_PIC,                  41,  12,     hdc_left9},
  {GUI_LEFT10_PIC,                 46,  14,     hdc_left10},
  {GUI_LEFT11_PIC,                 52,  12,     hdc_left11}, 
  {GUI_RIGHT1_PIC,                 33,  13,     hdc_right1},
  {GUI_RIGHT2_PIC,                 33,  13,     hdc_right2},
  {GUI_RIGHT3_PIC,                 33,  13,     hdc_right3},
  {GUI_RIGHT4_PIC,                 33,  13,     hdc_right4},           
  {GUI_RIGHT5_PIC,                 33,  13,     hdc_right5},
  {GUI_RIGHT6_PIC,                 34,  13,     hdc_right6},
  {GUI_RIGHT7_PIC,                 36,  13,     hdc_right7},
  {GUI_RIGHT8_PIC,                 38,  13,     hdc_right8},
  {GUI_RIGHT9_PIC,                 41,  12,     hdc_right9},
  {GUI_RIGHT10_PIC,                45,  13,     hdc_right10},
  {GUI_RIGHT11_PIC,                51,  12,     hdc_right11},
};

HDC hdc_am_bk;
HDC hdc_am_png[hdc_am_end];
/****************************** �����Ǳ� App ͼƬ END ********************************/

/********************************************************************************************
*                                ��ѹ�� App ͼƬ                                            *
********************************************************************************************/
const adc_hdc_t adc_png_info[hdc_adc_end] = 
{
  {GUI_ADC_SLIDER_BTN_PIC,     53,  53,     hdc_adc_slider_btn},
  {GUI_ADC_SLIDER_PIC,         360, 27,     hdc_adc_slider},
  {GUI_ADC_CIRCLE_PIC,         270, 270,    hdc_adc_circle},
  {GUI_ADC_F429_RP_PIC,        350, 340,    hdc_adc_F429_RP},
};

HDC hdc_adc_bk;
HDC hdc_adc_png[hdc_adc_end];
/********************************************************************************************
*                             ��ѹ�� App ͼƬ END                                           *
********************************************************************************************/

/********************************************************************************************
*                                     ��ҳ��ͼ��                                             *
********************************************************************************************/

const icon_info_t bmp_icon_info[bmp_icon_end] = 
{
  {GUI_ADC_ICON_PIC,        80,  80,     bmp_adc_icon},
  {GUI_MUSIC_ICON_PIC,      80,  80,     bmp_music_icon},
  {GUI_PHOTO_ICON_PIC,      80,  80,     bmp_photo_icon},
  {GUI_CLOCK_ICON_PIC,      80,  80,     bmp_clock_icon},
  {GUI_CAMERA_ICON_PIC,     80,  80,     bmp_camera_icon},
  {GUI_GYRO_ICON_PIC,       80,  80,     bmp_gyro_icon},
  {GUI_HUMITURE_ICON_PIC,   80,  80,     bmp_humiture_icon},
  {GUI_VIDEO_ICON_PIC,      80,  80,     bmp_video_icon},
  {GUI_RGBLEN_ICON_PIC,     80,  80,     bmp_rgbled_icon},
  
  {GUI_GUIUSE_ICON_PIC,     80,  80,     bmp_guiuse_icon},
  {GUI_SUDISH_ICON_PIC,     80,  80,     bmp_sudish_icon},
  {GUI_NETWORK_ICON_PIC,    80,  80,     bmp_entwork_icon},
  {GUI_WIFI_ICON_PIC,       80,  80,     bmp_wifi_icon},
  {GUI_PHONE_ICON_PIC,      80,  80,     bmp_phone_icon},
  {GUI_NOTE_ICON_PIC,       80,  80,     bmp_note_icon},
  {GUI_QRCODE_ICON_PIC,     80,  80,     bmp_QRcode_icon},
  {GUI_RECORD_ICON_PIC,     80,  80,     bmp_record_icon},
  {GUI_WIDGET_ICON_PIC,     80,  80,     bmp_widget_icon},
  {GUI_FLASH_ICON_PIC,      80,  80,     bmp_flash_icon},

};

HDC hdc_home_bk;
u8 * bmp_icon[bmp_icon_end];
/********************************************************************************************
 *                             ��ҳ��ͼ��  END                                               *
 ********************************************************************************************/

/********************************************************************************************
*                                ����������ͼƬ                                            *
********************************************************************************************/
const beeper_hdc_t beeper_png_info[hdc_beeper_end] = 
{
  {GUI_BEEPER_HORN_PIC,     62, 115,   hdc_beeper_horn},
  {GUI_BEEPER_HORN1_PIC,    20, 45,    hdc_beeper_horn1},
  {GUI_BEEPER_HORN2_PIC,    32, 90,    hdc_beeper_horn2},
  {GUI_BEEPER_HORN3_PIC,    44, 131,   hdc_beeper_horn3},
  {GUI_BEEPER_BUTTON_PIC,   51, 51,    hdc_beeper_button},
};

HDC hdc_beeper_bk;
HDC hdc_beeper_png[hdc_beeper_end];
/********************************************************************************************
*                             ����������ͼƬ END                                            *
********************************************************************************************/

BOOL PIC_Load_To_SDRAM(void)
{
  BOOL res = TRUE;
  
/****************************** ʱ�� App ͼƬ ********************************/
  hdc_clock_bk = Load_jpg_to_hdc(GUI_CLOCK_BACKGROUNG_PIC, GUI_XSIZE, GUI_YSIZE);
  
  for (uint8_t xC=0; xC<hdc_clock_end; xC++)
  {
    /* ���� HDC */
    hdc_clock_png[clock_png_info[xC].id] = Load_png_to_hdc(
                                            clock_png_info[xC].pic_name,
                                            clock_png_info[xC].w,  
                                            clock_png_info[xC].h);
  }
  
/****************************** ʱ�� App ͼƬ END ********************************/
  
/****************************** �����Ǳ� App ͼƬ ********************************/
  hdc_am_bk = Load_jpg_to_hdc(GUI_AUTOMETER_BACKGROUNG_PIC, GUI_XSIZE, GUI_YSIZE);
  
  for (uint8_t xC=0; xC<hdc_am_end; xC++)
  {
    /* ���� HDC */
    hdc_am_png[am_png_info[xC].id] = Load_png_to_hdc(
                                            am_png_info[xC].pic_name,
                                            am_png_info[xC].w,  
                                            am_png_info[xC].h);
  }
  
/****************************** �����Ǳ� App ͼƬ END ********************************/
  
  
/********************************************************************************************
*                                ��ѹ�� App ͼƬ                                            *
********************************************************************************************/
  hdc_adc_bk = Load_jpg_to_hdc(GUI_ADC_BACKGROUNG_PIC, GUI_XSIZE, GUI_YSIZE);
  
  for (uint8_t xC=0; xC<hdc_adc_end; xC++)
  {
    /* ���� HDC */
    hdc_adc_png[adc_png_info[xC].id] = Load_png_to_hdc(
                                            adc_png_info[xC].pic_name,
                                            adc_png_info[xC].w,  
                                            adc_png_info[xC].h);
  }

/********************************************************************************************
*                              ��ѹ�� App ͼƬ END                                          *
********************************************************************************************/

/********************************************************************************************
*                                     ��ҳ��ͼ��                                             *
********************************************************************************************/
  hdc_home_bk = Load_jpg_to_hdc(GUI_HOME_BACKGROUNG_PIC, GUI_XSIZE, GUI_YSIZE);
  
//  uint32_t pic_size;
//  for (uint8_t xC=0; xC<bmp_icon_end; xC++)
//  {
//    /* ���� HDC */
//    if (strstr(bmp_icon_info[xC].pic_name, "0:/") != NULL)
//    {
//      res = FS_Load_Content(bmp_icon_info[xC].pic_name, (char **)&bmp_icon[xC], &pic_size);    // ��Դ�� SD ��
//    }
//    else
//    {
//      res = RES_Load_Content(bmp_icon_info[xC].pic_name, (char **)&bmp_icon[xC], &pic_size);     // ��Դ���ⲿ FLASH
//    }
//    if (!res)
//    {
//      GUI_ERROR("Can not find RES:%s",bmp_icon_info[xC].pic_name);
//      res_not_found_flag = TRUE;    // ���û���ҵ���Դ�ļ�
//    }
//  }
/********************************************************************************************
*                                  ��ҳ��ͼ��  END                                           *
********************************************************************************************/
  
/********************************************************************************************
*                                ����������ͼƬ                                             *
********************************************************************************************/
  hdc_beeper_bk = Load_jpg_to_hdc(GUI_BEEPER_BACKGROUNG_PIC, GUI_XSIZE, GUI_YSIZE);
  
  for (uint8_t xC=0; xC<hdc_beeper_end; xC++)
  {
    /* ���� HDC */
    hdc_beeper_png[beeper_png_info[xC].id] = Load_png_to_hdc(
                                             beeper_png_info[xC].pic_name,
                                             beeper_png_info[xC].w,  
                                             beeper_png_info[xC].h);
  }

/********************************************************************************************
*                              ����������ͼƬ END                                           *
********************************************************************************************/
  
  return res;
}


static HDC Load_png_to_hdc(char *file_name, int w, int h)
{	 
  BOOL res;
  u8 *pic_buf;
  u32 pic_size;
  PNG_DEC *png_dec;
  BITMAP png_bm;
  HDC hdc;
  
#if (GUI_APP_BOOT_INTERFACE_EN)
{
  /* ��������Ľ����� */
  if(Boot_progbar != NULL)
  {  
    int count = 0;

    /* ������������Ľ����� */
    count = SendMessage(Boot_progbar,PBM_GET_VALUE,TRUE,NULL); 
    count++;
    SendMessage(Boot_progbar,PBM_SET_VALUE,TRUE,count); 
  }  
}
#endif
  
  /* ���� HDC */
  hdc = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, w, h);
  ClrDisplay(hdc, NULL, 0);
  if (strstr(file_name, "0:/") != NULL)
  {
    res = FS_Load_Content(file_name, (char**)&pic_buf, &pic_size);    // ��Դ�� SD ��
  }
  else
  {
    res = RES_Load_Content(file_name, (char**)&pic_buf, &pic_size);     // ��Դ���ⲿ FLASH
  }
  
  if(res)
  {
    png_dec = PNG_Open(pic_buf);
    PNG_GetBitmap(png_dec, &png_bm);
    DrawBitmap(hdc, 0, 0, &png_bm, NULL);
    PNG_Close(png_dec);
  }
  else
  {
    GUI_ERROR("Can not find RES:%s",file_name);
    res_not_found_flag = TRUE;    // ���û���ҵ���Դ�ļ�
  }
  
  /* �ͷ�ͼƬ���ݿռ� */
  RES_Release_Content((char **)&pic_buf);
 
  return hdc;
}

static HDC Load_jpg_to_hdc(char *file_name, int w, int h) 
{
  u8 *jpeg_buf;
  u32 jpeg_size;
  JPG_DEC *dec;
  BOOL res = NULL;
  HDC hdc;
  
#if (GUI_APP_BOOT_INTERFACE_EN)
{
  /* ��������Ľ����� */
  if(Boot_progbar != NULL)
  {  
    int count = 0;

    /* ������������Ľ����� */
    count = SendMessage(Boot_progbar,PBM_GET_VALUE,TRUE,NULL); 
    count++;
    SendMessage(Boot_progbar,PBM_SET_VALUE,TRUE,count); 
  }  
}
#endif

  if (strstr(file_name, "0:/") != NULL)
  {
    res = FS_Load_Content(file_name, (char**)&jpeg_buf, &jpeg_size);    // ��Դ�� SD ��
  }
  else
  {
    res = RES_Load_Content(file_name, (char**)&jpeg_buf, &jpeg_size);     // ��Դ���ⲿ FLASH
  }
  
  hdc = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
  if(res)
  {
    /* ����ͼƬ���ݴ���JPG_DEC��� */
    dec = JPG_Open(jpeg_buf, jpeg_size);

    /* �������ڴ���� */
    JPG_Draw(hdc, 0, 0, dec);

    /* �ر�JPG_DEC��� */
    JPG_Close(dec);
  }
  else
  {
    GUI_ERROR("Can not find RES:%s",file_name);
    res_not_found_flag = TRUE;    // ���û���ҵ���Դ�ļ�
  }
  /* �ͷ�ͼƬ���ݿռ� */
  RES_Release_Content((char **)&jpeg_buf);
  
  return hdc;
}

/********************************END OF FILE****************************/

