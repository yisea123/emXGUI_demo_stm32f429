#include <emXGUI.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "ff.h"
#include "x_libc.h"
#include "GUI_AppDef.h"
#include "emXGUI_JPEG.h"
#include "emxgui_png.h"
#include  "GUI_ADC_CollectVoltage_Dialog.h"
#include "./pic_load/gui_pic_load.h"

/* ͼƬ��Դ */
//#define BMP    1    // 1��ʹ��png 0��ʹ��bmp

//#if BMP
//  #define Slider_Button_Name        "slider_button.png"    // 90 * 90
//  #define Slider_Name               "slider.png"           // 600 * 45 
//  #define Adc_Circle_Name           "adc_circle.png"       // 230 * 230
//  #define F429_RP_Name              "F429_RP.png"          // 350 * 340
//#else
//  #define Slider_Button_Name        "slider_button.bmp"    // 90 * 90
//  #define Slider_Name               "slider.bmp"           // 600 * 45 
//  #define Adc_Circle_Name           "adc_circle.bmp"       // 230 * 230
//  #define F429_RP_Name              "F429_RP.bmp"          // 350 * 340
//#endif
//  #define GUI_ADC_BACKGROUNG_PIC    "adc_desktop.jpg"


/* ���� ID */
#define ID_ADV_WIN         0x01    // �м���ʾ����ID
#define SCROLLBAR_Brigh_ID 0x02    // ��������ID
#define ID_TEXTBOX_Title   0x03    // ������
#define ID_TEXTBOX_Brigh   0x04    // ���Ȱٷֱ�

#define CircleCenter_1    (98)     // ��������ת�뾶
#define CircleCenter_2    (125)    // Բ���������뾶��С��
#define CircleCenter_3    (CircleCenter_2 + 10)    //  ������ CircleSize / 2

/* �ƶ������־ */
#define LeftToRight    0
#define RightToLeft    1
#define MOVE_WIN       1

#define CircleSize    285    // Բ����ʾ����Ĵ�С
#define Circle_X      460    // Բ����ʾ�����λ��
#define Circle_Y      (27)   // Բ����ʾ�����λ��

#define TitleHeight    70    // �������ĸ߶�

#define TriangleLen    20    // �����εı߳�
 
uint8_t AovingDirection = 0;

HWND MAIN_Handle;
HWND Title_Handle;
HWND Brigh_Handle;
HWND ADC_Handle;
HWND Brigh_TEXTBOX_Handle;

HDC BacklightFont_hdc;
HDC TrianglePointer_DC;
HDC hdc_mem;

static BITMAP bm_Triangle;

static WCHAR Backlightwbuf[50] __EXRAM;

static COLORREF color_bg;       // ͸���ؼ��ı�����ɫ

static uint8_t res_prep = 1;    // ��Դ׼����־(��ǰ���ص�SDRAM��������Ϊ1��ʾ�������)

// �ֲ����������ڱ���ת�������ĵ�ѹֵ 	 
double ADC_Vol; 

static void	X_MeterPointer(HDC hdc, int cx, int cy, int r, u32 color, double dat_val)
{
  double angle;
  int midpoint_x,midpoint_y;
  POINT pt[4];

  angle = (dat_val * 1.427 );    // ����Ƕ� + 0.785 = 45��
  
  /* ����������ƽ�е�һ�ߵ��е����� */
  midpoint_x =cx - sin(angle) * (r - TriangleLen * 0.866);    // 0.866 = sqrt(3) / 2
  midpoint_y =cy + cos(angle) * (r - TriangleLen * 0.866);

  /* ��Զ��һ���� */
  pt[0].x = cx - r * sin(angle);
  pt[0].y = cy + r * cos(angle);

  /* ��Զһ������һ�� */
  pt[1].x = midpoint_x - (TriangleLen / 2) * sin(angle - 1.57);    // 1.57 = 3.14/2 = ��/2 = 90��
  pt[1].y = midpoint_y + (TriangleLen / 2) * cos(angle - 1.57);

  /* ��Զһ����ұ�һ�� */
  pt[2].x = midpoint_x - (TriangleLen / 2) * sin(angle + 1.57);
  pt[2].y = midpoint_y + (TriangleLen / 2) * cos(angle + 1.57);

  pt[3].x = pt[0].x;
  pt[3].y = pt[0].y;


  /* �������� */
  SetBrushColor(hdc,color);
  EnableAntiAlias(hdc, TRUE);
  FillPolygon(hdc,0,0,pt,4);
  EnableAntiAlias(hdc, FALSE);
}

//�˳���ť�ػ���
static void CollectVoltage_ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
{
  HDC hdc;
  RECT rc;
//  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
//  hwnd = ds->hwnd;

//  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
//  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

//  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
		SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));
	}
	else
	{ //��ť�ǵ���״̬

		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));      //���û���ɫ
	}

  SetPenSize(hdc, 2);

  InflateRect(&rc, 0, -1);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 9;
  }

}

/*
 * @brief  ���ƹ�����
 * @param  hwnd:   �������ľ��ֵ
 * @param  hdc:    ��ͼ������
 * @param  back_c��������ɫ
 * @param  Page_c: ������Page������ɫ
 * @param  fore_c���������������ɫ
 * @retval NONE
*/
static void draw_scrollbar(HWND hwnd, HDC hdc, COLOR_RGB32 back_c, COLOR_RGB32 Page_c, COLOR_RGB32 fore_c)
{
	RECT rc,rc_tmp;
  RECT rc_scrollbar;

	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
  SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc_tmp);    // �õ���ť��λ��

  BitBlt(hdc, rc_tmp.x, rc.y+45/2, rc.w - rc_tmp.x, rc.h/2, hdc_adc_png[hdc_adc_slider], rc_tmp.x, 0, SRCCOPY);

  rc_scrollbar.x = rc_tmp.x;
  rc_scrollbar.y = rc.h/2-4;
  rc_scrollbar.w = rc.w - rc_tmp.x - 15;
  rc_scrollbar.h = 10;
	SetBrushColor(hdc, MapRGB888(hdc, Page_c));
  FillRoundRect(hdc, &rc_scrollbar, 4);
}

/*
 * @brief  ���ƹ�����
 * @param  hwnd:   �������ľ��ֵ
 * @param  hdc:    ��ͼ������
 * @param  back_c��������ɫ
 * @param  Page_c: ������Page������ɫ
 * @param  fore_c���������������ɫ
 * @retval NONE
*/
static void draw_gradient_scrollbar(HWND hwnd, HDC hdc, COLOR_RGB32 back_c, COLOR_RGB32 Page_c, COLOR_RGB32 fore_c)
{
	RECT rc,rc_tmp;
   RECT rc_scrollbar;
	GetClientRect(hwnd, &rc);
	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc_tmp);    // �õ���ť��λ��
  
  BitBlt(hdc, rc.x, rc.y+45/2, rc_tmp.x, rc.h/2, hdc_adc_png[hdc_adc_slider], 0, 0, SRCCOPY);

  rc_scrollbar.x = rc.x+15;
  rc_scrollbar.y = rc.h/2-4;
  rc_scrollbar.w = rc_tmp.x;
  rc_scrollbar.h = 8;
   
	SetBrushColor(hdc, MapRGB888(hdc, fore_c));
  FillRoundRect(hdc, &rc_scrollbar, 3);
}

/*
 * @brief  �Զ��廬�������ƺ���
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void scrollbar_owner_draw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
	HDC hdc_mem;
	RECT rc;
	RECT rc_cli,rc_tmp;;

	hwnd = ds->hwnd;
	hdc = ds->hDC;
	GetClientRect(hwnd, &rc_cli);

	hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);   
         
  EnableAntiAlias(hdc, TRUE);

  /* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
  BitBlt(hdc_mem, rc.x, rc.y, rc.w, rc.h, hdc_adc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

	//���ư�ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem, color_bg, RGB888(50, 50, 50), RGB888(255, 255, 255));
	//���ƽ������͵Ĺ�����
	draw_gradient_scrollbar(hwnd, hdc_mem, color_bg, RGB888(50, 50, 50), RGB888(2, 100, 253));
  EnableAntiAlias(hdc, FALSE);
  SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);   
  
	//���ƻ���
  BitBlt(hdc_mem, rc.x, rc.y, rc.w, rc.h, hdc_adc_png[hdc_adc_slider_btn], 0, 0, SRCCOPY);

  BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);

	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem);
}

/*
 * @brief  �ػ�͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Textbox_OwnerDraw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc, rc_tmp;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  if (res_prep == 0)
  {
    SetBrushColor(hdc, MapRGB(hdc, 5, 5, 5));
    FillRect(hdc, &rc);
  }
  else
    BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_adc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);
  SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));

  GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
}

/*
 * @brief  �ػ���ʾ���ȵ�͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Brigh_Textbox_OwnerDraw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc, rc_tmp;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_adc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);
  SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
  rc.w -= 45;
  GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
  SetFont(hdc, controlFont_100);
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_RIGHT);//��������(���ж��뷽ʽ)
  rc.w += 45;
  rc.y += 17;
  SetFont(hdc, controlFont_48);
  DrawText(hdc, L"H", -1, &rc, DT_VCENTER|DT_RIGHT);//��������(���ж��뷽ʽ)
  SetFont(hdc, defaultFont);
}

/* �ػ�Բ����ʾ���� */
void Circle_Paint(HWND hwnd, HDC hdc)
{
  char  cbuf[128];
  WCHAR wbuf[128];
  int vertex_x,vertex_y;
  RECT rc = {0, 0, CircleSize, CircleSize};
  int angle = ((225 - (-45))) * ADC_Vol / 3.3;

  EnableAntiAlias(hdc, TRUE);

  BitBlt(hdc, CircleSize/2-270/2, CircleSize/2-270/2, 270, 270, hdc_adc_png[hdc_adc_circle], 0, 0, SRCCOPY);     // 270 ��ͼƬ�Ĵ�С
  RotateBitmap(hdc, CircleSize/2, CircleSize/2, &bm_Triangle, angle + 45);
  
  if (ADC_Vol > 0.03)    // ��ѹ̫С������ɫСԲȦ
  {
    SetBrushColor(hdc, MapARGB(hdc, 255, 200, 200, 200));
    FillArc(hdc, CircleSize/2, CircleSize/2, CircleCenter_2+1, CircleCenter_3-1, -45, angle  - 45);
    /* �������СԲȦ������ */
    vertex_x =  CircleSize / 2 - ((CircleCenter_3 - CircleCenter_2) / 2 + CircleCenter_2) * sin(3.14/4); 
    vertex_y =  CircleSize / 2 + ((CircleCenter_3 - CircleCenter_2) / 2 + CircleCenter_2) * cos(3.14/4); 
    FillCircle(hdc, vertex_x, vertex_y, (CircleCenter_3 - CircleCenter_2) / 2 - 1);     // ���СԲȦ
  }

  EnableAntiAlias(hdc, FALSE);

  /* ʹ��Ĭ������ */
	SetFont(hdc, defaultFont);

  rc.w = 24*4;
  rc.h = 48;
  rc.x = CircleSize/2 - rc.w/2;
  rc.y = CircleSize/2 - rc.h/2;

  /* ��ʾ��ѹ�ٷֱ� */
  SetTextColor(hdc, MapARGB(hdc, 255, 0, 0, 0));
  SetFont(hdc, controlFont_48);
  x_sprintf(cbuf, "%d", (int)(ADC_Vol/3.3*100));    // H -> % x_wsprintf(Backlightwbuf, L"%d", i);
  x_mbstowcs_cp936(wbuf, cbuf, 128);
  rc.w -= 25;
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_RIGHT);    // ��������(���ж��뷽ʽ)DT_CENTER
  rc.w += 25;
  rc.y += 4;
  SetFont(hdc, controlFont_32);
  DrawText(hdc, L"H", -1, &rc, DT_VCENTER|DT_RIGHT);    // ��������(���ж��뷽ʽ)DT_CENTER

  /* ��ʾ�ɼ����ĵ�ѹֵ */
  SetFont(hdc, defaultFont);
  rc.w = 24*4;
  rc.h = 30;
  rc.y = CircleSize/2 - rc.h/2 + CircleCenter_3;
  x_sprintf(cbuf, "%.2fV", ADC_Vol);
  x_mbstowcs_cp936(wbuf, cbuf, 128);
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
}

static BOOL Update_Circle_Flag = TRUE;

static LRESULT	ADCWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  
  switch(msg)
  {
    case WM_CREATE:
    {
      RECT rc;
      HWND hwnd_scrolbar;
      SCROLLINFO sif;/*���û������Ĳ���*/
      GetClientRect(hwnd, &rc);
      Rheostat_Init();    // ��ʼ�� ADC

      /*********************���ȵ��ڻ�����******************/
      sif.cbSize = sizeof(sif);
      sif.fMask = SIF_ALL;
      sif.nMin = 0;
      sif.nMax = 100;
      sif.nValue = 50;//��ʼֵ
      sif.TrackSize = 90;//����ֵ
      sif.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������          
      hwnd_scrolbar = CreateWindow(SCROLLBAR, L"SCROLLBAR_Brigh", WS_OWNERDRAW | WS_VISIBLE,//  
                      GUI_XSIZE + 100, (GUI_YSIZE - TitleHeight * 2) / 2, 600, 90, hwnd, SCROLLBAR_Brigh_ID, NULL, NULL);
      SendMessage(hwnd_scrolbar, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif); 

      // rc.w = 50*3+35;
      // rc.h = 100;
      // rc.x = GUI_XSIZE + GUI_XSIZE / 2 - rc.w / 2;
      // rc.y = TitleHeight-20;

      // Brigh_TEXTBOX_Handle = CreateWindow(TEXTBOX, L"50", WS_OWNERDRAW | WS_VISIBLE | WS_DISABLED, 
      //                           rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Brigh, NULL, NULL);  
      
      // SendMessage(Brigh_TEXTBOX_Handle,TBM_SET_TEXTFLAG,0,DT_VCENTER|DT_RIGHT|DT_BKGND);  


//      BOOL res;
//      u8 *pic_buf;
//      u32 pic_size;
//      
//      #if BMP
//      PNG_DEC *png_dec;
//      BITMAP png_bm;
//      #endif 
//      
//      /* ������λ����ʾ HDC */
//      hdc_adc_png[hdc_adc_F429_RP] = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, 350, 340);
//      ClrDisplay(hdc_adc_png[hdc_adc_F429_RP], NULL, 0);
//      res = RES_Load_Content(F429_RP_Name, (char**)&pic_buf, &pic_size);
//      if(res)
//      {
//        #if BMP
//          png_dec = PNG_Open(pic_buf);
//          PNG_GetBitmap(png_dec, &png_bm);
//          DrawBitmap(hdc_adc_png[hdc_adc_F429_RP], 0,0, &png_bm, NULL);
//          PNG_Close(png_dec);
//        #else
//          BMP_Draw(hdc_adc_png[hdc_adc_F429_RP], 0, 0, pic_buf, NULL);
//        #endif     
//      }
//      /* �ͷ�ͼƬ���ݿռ� */
//      RES_Release_Content((char **)&pic_buf);


      /* ����Բ������� HDC */
//      hdc_adc_png[hdc_adc_circle] = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, CircleSize, CircleSize);
//      ClrDisplay(hdc_adc_png[hdc_adc_circle],NULL,0);
//      res = RES_Load_Content(Adc_Circle_Name, (char**)&pic_buf, &pic_size);
//      // res = FS_Load_Content("0:/adc_circle.png", (char**)&pic_buf, &pic_size);
//      if(res)
//      {
//        #if BMP 
//          png_dec = PNG_Open(pic_buf);
//          PNG_GetBitmap(png_dec, &png_bm);
//          DrawBitmap(hdc_adc_png[hdc_adc_circle], CircleSize/2-270/2, CircleSize/2-270/2, &png_bm, NULL);
//          PNG_Close(png_dec);
//        #else
//          BMP_Draw(hdc_adc_png[hdc_adc_circle], CircleSize/2-270/2, CircleSize/2-270/2, pic_buf, NULL);
//        #endif
//      }
//      RES_Release_Content((char **)&pic_buf);     // �ͷ�ͼƬ���ݿռ�

      // EnableAntiAlias(hdc_adc_png[hdc_adc_circle], TRUE);

      // SetBrushColor(hdc_adc_png[hdc_adc_circle], MapARGB(hdc_adc_png[hdc_adc_circle], 255, 65, 65, 65));
      
      // /* �����ұ�СԲȦ������ */
      // vertex_x =  CircleSize / 2 - ((CircleCenter_3 - CircleCenter_2) / 2 + CircleCenter_2) * sin(3.14*7/4); 
      // vertex_y =  CircleSize / 2 + ((CircleCenter_3 - CircleCenter_2) / 2 + CircleCenter_2) * cos(3.14*7/4); 
      // FillCircle(hdc_adc_png[hdc_adc_circle], vertex_x, vertex_y, (CircleCenter_3 - CircleCenter_2) / 2);     // �ұ�СԲȦ
      // FillArc(hdc_adc_png[hdc_adc_circle], CircleSize/2, CircleSize/2, CircleCenter_2, CircleCenter_3, -45, 225);    // ������������
      // EnableAntiAlias(hdc_adc_png[hdc_adc_circle], FALSE);

      /* ��������ָ�� */
      TrianglePointer_DC = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, TriangleLen, CircleCenter_1 * 2);    // ����������ָ���ڴ� DC
      ClrDisplay(TrianglePointer_DC, NULL, 0);
      X_MeterPointer(TrianglePointer_DC, TriangleLen/2, CircleCenter_1, CircleCenter_1-2, MapARGB(TrianglePointer_DC, 255, 250, 20, 20), 0);
      /* ת����bitmap */
      DCtoBitmap(TrianglePointer_DC,&bm_Triangle);

//      /* ������������ť�� HDC */
//      hdc_adc_png[hdc_adc_slider_btn] = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, 90, 90);
//      ClrDisplay(hdc_adc_png[hdc_adc_slider_btn],NULL,0);
//      res = RES_Load_Content(Slider_Button_Name, (char**)&pic_buf, &pic_size);
//      if(res)
//      {
//        #if BMP
//          png_dec = PNG_Open(pic_buf);
//          PNG_GetBitmap(png_dec, &png_bm);
//          DrawBitmap(hdc_adc_png[hdc_adc_slider_btn], 0,0, &png_bm, NULL);
//          PNG_Close(png_dec);
//        #else
//          BMP_Draw(hdc_adc_png[hdc_adc_slider_btn], 0, 0, pic_buf, NULL);
//        #endif 
//      }
//      /* �ͷ�ͼƬ���ݿռ� */
//      RES_Release_Content((char **)&pic_buf);

//      /* ������������ HDC */
//      hdc_adc_png[hdc_adc_slider] = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, 600, 45);
//      ClrDisplay(hdc_adc_png[hdc_adc_slider],NULL,0);
//      res = RES_Load_Content(Slider_Name, (char**)&pic_buf, &pic_size);
//      if(res)
//      {
//        #if BMP
//          png_dec = PNG_Open(pic_buf);
//          PNG_GetBitmap(png_dec, &png_bm);
//          DrawBitmap(hdc_adc_png[hdc_adc_slider], 0,0, &png_bm, NULL);
//          PNG_Close(png_dec);
//        #else
//          BMP_Draw(hdc_adc_png[hdc_adc_slider], 0, 0, pic_buf, NULL);
//        #endif 
//      }
//      /* �ͷ�ͼƬ���ݿռ� */
//      RES_Release_Content((char **)&pic_buf);
      
      /* ����Բ����������� DC */
      hdc_mem = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, CircleSize, CircleSize);
      
      ClrDisplay(hdc_mem, NULL, 0);

      SetTimer(hwnd, 2, 50, TMR_START, NULL);
      
      x_wsprintf(Backlightwbuf, L"%d", 50);

      res_prep = 1;    // �����Դ�������
      SetTimer(hwnd, 4, 10, TMR_START | TMR_SINGLE, NULL);

      break;
    }

    case WM_TIMER:
    {
      RECT rc;
      int tmr_id;
       
      tmr_id = wParam;
      GetWindowRect(hwnd, &rc);
      SetForegroundWindow(hwnd);

      if (tmr_id == MOVE_WIN)
      {
        if (AovingDirection == LeftToRight)
        {
          if (rc.x < 0)
          {
            OffsetRect(&rc, (rc.w >> 3), 0);
            rc.x = MIN(rc.x, 0);
            MoveWindow(hwnd, rc.x, rc.y, rc.w, rc.h, TRUE);
          }
          else
          {
            InvalidateRect(MAIN_Handle, NULL, TRUE);
            SetWindowText(Title_Handle,L"ADCһ��λ����ѹ��ʾ");
            KillTimer(hwnd, 1);
          }
        }
        else if (AovingDirection == RightToLeft)
        {
          if (rc.x > -800)
          {
            OffsetRect(&rc, -(rc.w >> 3), 0);
            rc.x = MAX(rc.x, -800);
            MoveWindow(hwnd, rc.x, rc.y, rc.w, rc.h, TRUE);
          }
          else
          {
            // RECT indicate_rc;
            // indicate_rc.x =
            InvalidateRect(MAIN_Handle, NULL, TRUE);
            SetWindowText(Title_Handle,L"�����ؼ���ʾ");
            KillTimer(hwnd, 1);
          }
        }
      }
      else if (tmr_id == 2)
      {
        RECT rc;
        static double ADC_buff = 0.0;
        double vol_buff = 0.0;
        static uint8_t xC = 0;
        static double ADC_Vol_Old;

        vol_buff =(double) ADC_ConvertedValue/4096*(double)3.3; // ��ȡת����ADֵ
//        GUI_DEBUG("��ѹֵǰΪ��%f", ADC_Vol);
        #if 1

          if (xC++ < 3)
          {
            ADC_buff += vol_buff;
            break;
          }
          else
          {
            ADC_Vol = ADC_buff / ((double)(xC-1));
            ADC_buff = 0;
            xC = 0;
          }

        #else
          
        ADC_Vol = vol_buff;//(double)(((int)( * 10)) / 10.0);

        #endif
      //  GUI_DEBUG("��ѹֵ��Ϊ��%f", ADC_Vol);
      //  ADC_Vol = (double)(((int)(vol_buff * 100)) / 100.0);
      //  GUI_DEBUG("��ѹֵ��Ϊ��%f", ADC_Vol);
        if (fabs(ADC_Vol - ADC_Vol_Old) <= 0.01)
        {
//          ADC_Vol_Old = ADC_Vol;    // ���¾ɵ�ֵ
          break;                    // ���βɼ�����һ�����ػ�ֱ�ӷ���
        }
        // GUI_DEBUG(" %fV", ADC_Vol - ADC_Vol_Old);
        ADC_Vol_Old = ADC_Vol;    // ���¾ɵ�ֵ

        rc.x = Circle_X;
        rc.y = Circle_Y;
        rc.w = CircleSize;
        rc.h = CircleSize;
        Update_Circle_Flag = TRUE;

        InvalidateRect(hwnd, &rc, TRUE);
      }
      else if (tmr_id == 4)
      {
        // �ػ游����
        InvalidateRect(MAIN_Handle, NULL, TRUE);
        InvalidateRect(GetDlgItem(MAIN_Handle, ID_TEXTBOX_Title), NULL, TRUE);     // �ػ洰�ڱ���
        InvalidateRect(hwnd, NULL, TRUE);    // �ػ��Ӵ���
      }
      
      break;
    }

    case WM_ERASEBKGND:
    {
      
      HDC hdc =(HDC)wParam;
      RECT rc = {0, TitleHeight, GUI_XSIZE, GUI_YSIZE - TitleHeight};

      ScreenToClient(hwnd, (POINT *)&rc, 1);
      BitBlt(hdc, rc.x, rc.y, GUI_XSIZE, rc.h, hdc_adc_bk, 0, TitleHeight, SRCCOPY);

      BitBlt(hdc, 0, 0, 350, 340, hdc_adc_png[hdc_adc_F429_RP], 0, 0, SRCCOPY);

      /* ��ʾ����ͼ�� */
      SetFont(hdc, controlFont_48);
      SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
      rc.y = TitleHeight;
      rc.x = GUI_XSIZE + 100;
      rc.w = 70;
      rc.h = TitleHeight;
      DrawText(hdc, L"I", -1, &rc, NULL);//��������(���ж��뷽ʽ)

      SetFont(hdc, controlFont_72);
      rc.x = GUI_XSIZE + 630;
      DrawText(hdc, L"I", -1, &rc, NULL);//��������(���ж��뷽ʽ)

      SetFont(hdc, defaultFont);   // �ָ�Ĭ������

      return FALSE;
    }
  
    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      RECT rc = {Circle_X, Circle_Y, 2, 2};

      ClientToScreen(hwnd, (POINT *)&rc, 1);

      hdc = BeginPaint(hwnd, &ps);

      if (Update_Circle_Flag == TRUE)    /* ��Ҫ���� */
      {
        Update_Circle_Flag = FALSE;
        ClrDisplay(hdc_mem, NULL, 0);
        Circle_Paint(hwnd, hdc_mem);    /* ����Բ����ʾ���� */
      }

      BitBlt(hdc, Circle_X, Circle_Y, CircleSize, CircleSize, hdc_mem, 0, 0, SRCCOPY);

      rc.w = 50*3+35;
      rc.h = 100;
      rc.x = GUI_XSIZE + GUI_XSIZE / 2 - rc.w / 2;
      rc.y = TitleHeight-20;

      SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
      rc.w -= 45;
      SetFont(hdc, controlFont_100);
      DrawText(hdc, Backlightwbuf, -1, &rc, DT_VCENTER|DT_RIGHT);//��������(���ж��뷽ʽ)
      rc.w += 45;
      rc.y += 17;
      SetFont(hdc, controlFont_48);
      DrawText(hdc, L"H", -1, &rc, DT_VCENTER|DT_RIGHT);//��������(���ж��뷽ʽ)
      SetFont(hdc, defaultFont);

      EndPaint(hwnd, &ps);

      break;
    }

    case WM_NOTIFY:
    {
      u16 ctr_id;
      NMHDR *nr;
//      code=HIWORD(wParam);//��ȡ��Ϣ������
      ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID.
      nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.      

      if (ctr_id == SCROLLBAR_Brigh_ID)
      {
        NM_SCROLLBAR *sb_nr;
        int i = 0;
        sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
        switch (nr->code)
        {
          case SBN_THUMBTRACK: //R�����ƶ�
          {
            i = sb_nr->nTrackValue; //��û��鵱ǰλ��ֵ                
            SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, i); //���ý���ֵ

            x_wsprintf(Backlightwbuf, L"%d", i);

            RECT rc;
            rc.w = 35*7;
            rc.h = 100;
            rc.x = GUI_XSIZE + GUI_XSIZE / 2 - rc.w / 2;
            rc.y = TitleHeight-20;
            InvalidateRect(hwnd, &rc, TRUE);//FALSE
            // SetWindowText(Brigh_TEXTBOX_Handle, Backlightwbuf);
          }
          break;
        }
      }
      break;
    } 
 
    
    static int x_move;

    case WM_LBUTTONDOWN:
    {
      int x;
      x = LOWORD(lParam);
      RECT rc;
	  	GetWindowRect(hwnd, &rc);
      rc.x = x;
      ClientToScreen(hwnd, (POINT *)&rc, 1);
      x_move = rc.x;
//      GUI_DEBUG("���£�x = %d",x);
      break;
    }

    static int x_old;
    case WM_MOUSEMOVE:
    {
      int x;
      RECT rc;
      RECT Client_rc;
	  	GetWindowRect(hwnd, &rc);
      GetWindowRect(hwnd, &Client_rc);
      x = LOWORD(lParam);
      
      rc.x = x;
      ClientToScreen(hwnd, (POINT *)&rc, 1);
      OffsetRect(&Client_rc, rc.x - x_move, 0);

      Client_rc.x = MIN(Client_rc.x, 30);
      Client_rc.x = MAX(Client_rc.x, -800 - 30);
      
      MoveWindow(hwnd, Client_rc.x, Client_rc.y, Client_rc.w, Client_rc.h, TRUE);

      x_old = x_move;
      x_move = rc.x;

//      GUI_DEBUG("�ƶ���x = %d", x_old);
      break;
    }

    case WM_LBUTTONUP:
    { 
      int x;
      RECT Client_rc;
      RECT rc;
      
      x = LOWORD(lParam);
      rc.x = x;
      GetWindowRect(hwnd, &Client_rc);
      ClientToScreen(hwnd, (POINT *)&rc, 1);
//      GUI_DEBUG("̧��x = %d",rc.x);

      if (rc.x - x_old > 0)    // �������һ�
      {
        if (Client_rc.x > 0)
        {
          MoveWindow(hwnd, 0, Client_rc.y, Client_rc.w, Client_rc.h, TRUE);
        }
        else 
        {
          SetTimer(hwnd, 1, 5, TMR_START, NULL);
          AovingDirection = LeftToRight;
        }
      }
      else    // ��������
      {
        if (Client_rc.x < -800)
        {
          MoveWindow(hwnd, -800, Client_rc.y, Client_rc.w, Client_rc.h, TRUE);
        }
        else 
        {
          SetTimer(hwnd, 1, 5, TMR_START, NULL);
          AovingDirection = RightToLeft;
        }
      }
      
      break;
    } 

    case WM_DRAWITEM:    // ��ť�ػ�
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_ADC_EXIT:
          {
            CollectVoltage_ExitButton_OwnerDraw(ds);
            return TRUE;             
          }   

          case SCROLLBAR_Brigh_ID:
          {
            scrollbar_owner_draw(ds);
            return TRUE;             
          }

          case ID_TEXTBOX_Brigh:
          {
            Brigh_Textbox_OwnerDraw(ds);
            return TRUE;             
          } 
       }
       break;
    }

    case WM_DESTROY:
    {
      

      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
}
 
static LRESULT	CollectVoltage_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
      RECT rc;
      GetClientRect(hwnd, &rc); 
      
      /********************* �����м们������ ***************************/
      WNDCLASS wcex;

      wcex.Tag	 		    = WNDCLASS_TAG;
      wcex.Style			  = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc	= (WNDPROC)ADCWinProc;
      wcex.cbClsExtra		= 0;
      wcex.cbWndExtra		= 0;
      wcex.hInstance		= NULL;
      wcex.hIcon			  = NULL;
      wcex.hCursor		  = NULL;
      
      rc.x = 0;
      rc.y = TitleHeight;
      rc.w = GUI_XSIZE*2;
      rc.h = GUI_YSIZE - TitleHeight * 2;
      // ����" ADC �ɼ�����"�Ŀؼ�.
      ADC_Handle = CreateWindowEx(WS_EX_NOFOCUS, &wcex,L"---",WS_CLIPCHILDREN|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,ID_ADV_WIN,NULL,NULL);
            
      CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY |WS_OWNERDRAW|WS_VISIBLE,
                  740, 17, 36, 36, hwnd, eID_ADC_EXIT, NULL, NULL); 

      rc.w = GUI_XSIZE / 2;
      rc.h = TitleHeight-2;
      rc.x = GUI_XSIZE / 2 - rc.w / 2;
      rc.y = 0;

      Title_Handle = CreateWindow(TEXTBOX, L"ADCһ��λ����ѹ��ʾ", WS_VISIBLE | WS_OWNERDRAW, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Title, NULL, NULL);//
      SendMessage(Title_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER);   

      /************************ ���ر���ͼƬ ************************/
//        BOOL res;
//        u8 *jpeg_buf;
//        u32 jpeg_size;
//        JPG_DEC *dec;
//        res = RES_Load_Content(GUI_ADC_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
//        hdc_adc_bk = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
//        if(res)
//        {
//          /* ����ͼƬ���ݴ���JPG_DEC��� */
//          dec = JPG_Open(jpeg_buf, jpeg_size);

//          /* �������ڴ���� */
//          JPG_Draw(hdc_adc_bk, 0, 0, dec);

//          /* �ر�JPG_DEC��� */
//          JPG_Close(dec);
//        }
//        /* �ͷ�ͼƬ���ݿռ� */
//        RES_Release_Content((char **)&jpeg_buf);

//      SetTimer(hwnd, 3, 10, TMR_SINGLE | TMR_START, NULL);    // ����һ�ζ�ʱ��������Դ����

      break;
    } 
    case WM_TIMER:
    {

//      RECT rc;
//      int tmr_id;
//       
//      tmr_id = wParam;

//      if (tmr_id == 3)    // ��Դ���ض�ʱ��
//      {
//        /********************* �����м们������ ***************************/
//        WNDCLASS wcex;

//        wcex.Tag	 		    = WNDCLASS_TAG;
//        wcex.Style			  = CS_HREDRAW | CS_VREDRAW;
//        wcex.lpfnWndProc	= (WNDPROC)ADCWinProc;
//        wcex.cbClsExtra		= 0;
//        wcex.cbWndExtra		= 0;
//        wcex.hInstance		= NULL;
//        wcex.hIcon			  = NULL;
//        wcex.hCursor		  = NULL;
//        
//        rc.x = 0;
//        rc.y = TitleHeight;
//        rc.w = GUI_XSIZE*2;
//        rc.h = GUI_YSIZE - TitleHeight * 2;
//        // ����" ADC �ɼ�����"�Ŀؼ�.
//        ADC_Handle = CreateWindowEx(WS_EX_NOFOCUS, &wcex,L"---",WS_CLIPCHILDREN|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,ID_ADV_WIN,NULL,NULL);
//      }
      
      break;
    }

    case WM_ERASEBKGND:
    {
      HDC hdc =(HDC)wParam;
      RECT rc;
      
      GetClientRect(hwnd, &rc);

      if (res_prep == 0)    // ��Դ���ػ�û�����
      {
        SetBrushColor(hdc, MapRGB(hdc, 5, 5, 5));
        FillRect(hdc, &rc);
      }
      else
      {
        BitBlt(hdc, 0, 0, GUI_XSIZE, GUI_YSIZE, hdc_adc_bk, 0, 0, SRCCOPY);                                                   // ���Ʊ��������ֱ���
        //BitBlt(hdc, 0, GUI_YSIZE - TitleHeight, GUI_XSIZE, TitleHeight, hdc_adc_bk, 0, GUI_YSIZE - TitleHeight, SRCCOPY);       // �����°벿�ֱ���
      }

      return FALSE;
    }

    case WM_PAINT:
    {
      HDC hdc;
      RECT rc;
      PAINTSTRUCT ps;
      //  RECT rc = {0,0,800,70};
      //  hdc_mem = CreateMemoryDC(SURF_ARGB4444, 800,70);
       
      hdc = BeginPaint(hwnd, &ps);
      GetClientRect(hwnd, &rc);
      
      if (res_prep == 0)    // ��Դ���ػ�û�����
      {
        SetTextColor(hdc, MapRGB(hdc, 225, 225, 225));
        DrawText(hdc, L"��Դ���������Ե�...", -1, &rc, DT_VCENTER | DT_CENTER);    // ��������(���ж��뷽ʽ)
      }
      else
      {
        /* ��������ֱ�� */
        // HLine(hdc, 0, TitleHeight-1, GUI_XSIZE);
        // HLine(hdc, 0, GUI_YSIZE - TitleHeight, GUI_XSIZE);

        /* ������Ļ���µ�ָʾ�� */

        EnableAntiAlias(hdc, TRUE);
        if(AovingDirection == RightToLeft) 
        {
          /* �����ұ�һ�� */
          SetBrushColor(hdc, MapRGB(hdc, 220, 220, 220));
          FillCircle(hdc, 415+5, 415+5, 5);
        }
        else
        {
          /* �������һ�� */
          SetBrushColor(hdc, MapRGB(hdc, 220, 220, 220));
          FillCircle(hdc, 376+5, 415+5, 5);
        }
        EnableAntiAlias(hdc, FALSE);
      }

      EndPaint(hwnd, &ps);

      break;
    } 
    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_ADC_EXIT:
          {
            CollectVoltage_ExitButton_OwnerDraw(ds);
            return TRUE;             
          }    

          case ID_TEXTBOX_Title:
          {
            Textbox_OwnerDraw(ds);
            return TRUE;             
          } 
       }

       break;
    }
    case WM_NOTIFY:
    {
      u16 code, id;
      id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
      code=HIWORD(wParam);//��ȡ��Ϣ������    
      if(code == BN_CLICKED && id == eID_ADC_EXIT)
      {
        PostCloseMessage(hwnd);
        break;
      }

      break;
    } 

    case WM_LBUTTONUP:
    {
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
    }

    case WM_MOUSEMOVE:
    {
      return	DefWindowProc(hwnd, msg, wParam, lParam);
    }

    case WM_LBUTTONDOWN:
    {
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
    }

    case WM_DESTROY:
    {
      Rheostat_DISABLE();    // ֹͣADC�Ĳɼ�
//      DeleteDC(hdc_adc_bk);
//      DeleteDC(hdc_adc_png[hdc_adc_slider_btn]);
//      DeleteDC(hdc_adc_png[hdc_adc_slider]);
//      DeleteDC(hdc_adc_png[hdc_adc_circle]);
//      DeleteDC(hdc_adc_png[hdc_adc_F429_RP]);
      DeleteDC(TrianglePointer_DC);
      DeleteDC(hdc_mem);
      Update_Circle_Flag = TRUE;
//      res_prep = 0;     // ��λ��Դ������ɱ�־

      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
  
}

void GUI_ADC_CollectVoltage_Dialog(void)
{
	
	WNDCLASS	wcex;
	MSG msg;

	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CollectVoltage_proc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   
	//����������
	MAIN_Handle = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER|WS_OVERLAPPED|WS_CLIPCHILDREN,
                              &wcex,
                              L"GUI_ADC_CollectVoltage_Dialog",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0, 0, GUI_XSIZE, GUI_YSIZE,
                              NULL, NULL, NULL, NULL);
   //��ʾ������
	ShowWindow(MAIN_Handle, SW_SHOW);
	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, MAIN_Handle))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}  
}


