#include "emXGUI.h"
#include "./clock/GUI_CLOCK_DIALOG.h"
#include "x_libc.h"
#include "string.h"
#include "ff.h"
#include "GUI_AppDef.h"
#include "emXGUI_JPEG.h"
#include "emxgui_png.h"
#include "./clock/RTC/bsp_rtc.h"
#include "stm32f4xx_rtc.h"
#include <stdlib.h>
#include "./pic_load/gui_pic_load.h"

#define ICON_BTN_NUM     2     // ��ť����
#define ICON_TEXT_NUM   (5 + ICON_BTN_NUM)    // �ı�����

struct
{
	uint8_t page;    // ��ǰ����ҳ��
	uint8_t dial;    // ѡ�еı���
}Set_Start;

uint8_t clock_dial = 0;    // ����
static HWND clock_hwnd;

const WCHAR Week_List[][4] = {{L"����һ"}, {L"���ڶ�"}, {L"������"}, {L"������"}, {L"������"}, {L"������"}, {L"������"}};
const uint8_t month_list[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    // ��¼ÿ�µ��������

//ͼ���������
const clock_icon_t clock_icon[] = {

  /* ��ť */
  {L"����",           {318, 390, 166,  70},  ID_CLOCK_SET},             // 0. ����
  {L"O",              {740,   5,  36,  72},  ID_CLOCK_EXIT},            // 1. �˳�

  /* �ı� */
  {L"1",              {551, 210, 102,  93},  ID_CLOCK_DAY},             // 2. ��
  {L"����һ",         {569, 176,  77,  30},  ID_CLOCK_WEEK},            // 3. ��
  {L"1��",            {581, 303,  54,  30},  ID_CLOCK_MONTH},           // 4. ��
  {L" ",              {83,  131, 233, 243},  ID_CLOCK_TIME},            // 5. ������ʾ����
  {L"ʱ��&����",      {345,  23, 110,  33},  ID_CLOCK_TITLE},           // 6. ����
  
  /************ ���ô��ڿؼ� **************/
  /* ��ť */
  {L"��һ��",         {52,  390, 166,  70},  ID_CLOCK_BACK},            // 7. ��һ��
  {L"��һ��",         {587, 390, 166,  70},  ID_CLOCK_NEXT},            // 8. ��һ��
  {L"F",              {  0,  16,  48,  48},  ID_CLOCK_WAIVE},           // 9. ��������
  {L"���",           {317, 390, 166,  70},  ID_CLOCK_OK},              // 10. �������

  /* �ı� */
  {L"����ѡ��",       {352,  23,  96,  33},  ID_CLOCK_SETTITLE},        // 11. ���ô��ڱ���
  {L"00:00",          {314, 105, 173,  30},  ID_CLOCK_SETTIME},         // 12. ���ô��ڱ���
  {L"2000��01��01��", {314, 105, 173,  30},  ID_CLOCK_SETDATE},         // 13. ���ô��ڱ���

  /* ��ѡ��ť */
  {L" ",              {0,   102, 266, 272},  ID_CLOCK_Background00},    // 14. ���� 1
  {L" ",              {266, 102, 266, 272},  ID_CLOCK_Background01},    // 15. ���� 2
  {L" ",              {532, 102, 266, 272},  ID_CLOCK_Background02},    // 16. ���� 3

  /* ʱ��&����ѡ���б� */
  {L" ",              {230, 138, 170, 220},  ID_CLOCK_SetHour},         // 17. ����Сʱ���б�
  {L" ",              {400, 138, 169, 220},  ID_CLOCK_SetMinute},       // 18. ���÷��ӵ��б�

  {L" ",              {230, 138, 113, 220},  ID_CLOCK_SetYear},         // 19. ��������б�
  {L" ",              {343, 138, 113, 220},  ID_CLOCK_SetMonth},        // 20. �����µ��б�
  {L" ",              {456, 138, 113, 220},  ID_CLOCK_SetDate},         // 21. �����յ��б�
 
};

/* ͼƬ�������顪��HDC */
//const clock_hdc_t clock_png_info[hdc_clock_end] = 
//{
//  {GUI_CLOCK_BTN_PIC,       166, 70,     hdc_clock_btn},
//  {GUI_CLOCK_BTN_PRESS_PIC, 166, 70,     hdc_clock_btn_press},
//  {GUI_CLOCK_CALENDAR_PIC,  220, 240,    hdc_clock_calendar},
//  {GUI_CLOCK_00BACK_PIC,    233, 243,    hdc_clock_back_00},
//  {GUI_CLOCK_00H_PIC,         5, 45+39,  hdc_clock_h_00},
//  {GUI_CLOCK_00M_PIC,         5, 65+59,  hdc_clock_m_00},
//  {GUI_CLOCK_00S_PIC,         9, 103+53, hdc_clock_s_00},
//  {GUI_CLOCK_01BACK_PIC,    233, 243,    hdc_clock_back_01},
//  {GUI_CLOCK_01H_PIC,         6, 20+166, hdc_clock_h_01},
//  {GUI_CLOCK_01M_PIC,        16, 85+63,  hdc_clock_m_01},
//  {GUI_CLOCK_01S_PIC,         6, 102+72, hdc_clock_s_01},
//  {GUI_CLOCK_02BACK_PIC,    233, 243,    hdc_clock_back_02},
//  {GUI_CLOCK_02H_PIC,        15, 61+41,  hdc_clock_h_02},
//  {GUI_CLOCK_02M_PIC,        15, 77+57,  hdc_clock_m_02},
//  {GUI_CLOCK_02S_PIC,         7, 86+46,  hdc_clock_s_02},
//  {GUI_CLOCK_CHCKED_PIC,    272, 272,    hdc_clock_chcked},

//};

///* HDC */
//static HDC hdc_clock_bk;
//static HDC hdc_clock_png[hdc_clock_end];

/* ʱ��λͼ */
static BITMAP bm_clock_s_00;
static BITMAP bm_clock_m_00;
static BITMAP bm_clock_h_00;
static BITMAP bm_clock_s_01;
static BITMAP bm_clock_m_01;
static BITMAP bm_clock_h_01;
static BITMAP bm_clock_s_02;
static BITMAP bm_clock_m_02;
static BITMAP bm_clock_h_02;

static void exit_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
  HDC hdc;
  RECT rc, rc_tmp;
  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
  hwnd = ds->hwnd;

  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
		SetPenColor(hdc, MapRGB(hdc, 120, 120, 120));      //��������ɫ
	}
	else
	{ //��ť�ǵ���״̬

		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));
	}
  
  // SetBrushColor(hdc, MapRGB(hdc, 242, 242, 242));
  // FillRect(hdc, &rc);

  SetPenSize(hdc, 2);

  InflateRect(&rc, 0, -23);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 9;
  }

}

static void waive_btn_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc, rc_tmp;
  WCHAR wbuf[128];
  HWND hwnd;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  hwnd = ds->hwnd;

  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  if (ds->State & BST_PUSHED)
  { //��ť�ǰ���״̬
    SetTextColor(hdc, MapRGB(hdc, 120, 120, 120));
  }
  else
  { //��ť�ǵ���״̬
    SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
  }
  
  GetWindowText(ds->hwnd, wbuf, 128); //��ð�ť�ؼ�������
  SetFont(hdc, controlFont_48);
  /* ��ʾ�ı� */
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
}

static void btn_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc, rc_tmp;
  WCHAR wbuf[128];
  HWND hwnd;
  
  hwnd = ds->hwnd;
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  if (ds->State & BST_PUSHED)
  { //��ť�ǰ���״̬
    BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_png[hdc_clock_btn_press], 0, 0, SRCCOPY);
    SetTextColor(hdc, MapRGB(hdc, 200, 200, 200));
  }
  else
  { //��ť�ǵ���״̬
    BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_png[hdc_clock_btn], 0, 0, SRCCOPY);
    SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
  }
  
  GetWindowText(ds->hwnd, wbuf, 128); //��ð�ť�ؼ�������
  
  /* ��ʾ�ı� */
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
}

static void radiobox_owner_draw(DRAWITEM_HDR *ds, int ID) // ��ѡ��ť���
{
	HDC hdc;
	RECT rc, rc_tmp;
  HWND hwnd;
  
  hwnd = ds->hwnd;
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  switch(ID)
  {
    case ID_CLOCK_Background00:
          BitBlt(hdc, rc.w/2.0-233/2.0, rc.h/2.0-243/2.0+7, 233, 243, hdc_clock_png[hdc_clock_back_00], 0, 0, SRCCOPY);
    break;

    case ID_CLOCK_Background01:
          BitBlt(hdc, rc.w/2.0-233/2.0, rc.h/2.0-243/2.0+7, 233, 243, hdc_clock_png[hdc_clock_back_01], 0, 0, SRCCOPY);
    break;

    case ID_CLOCK_Background02:
          BitBlt(hdc, rc.w/2.0-233/2.0, rc.h/2.0-243/2.0+7, 233, 243, hdc_clock_png[hdc_clock_back_02], 0, 0, SRCCOPY);
    break;
  }

  if (ds->State & BN_CHECKED)
  { 
    // ��ť��ѡ��״̬
    BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_png[hdc_clock_chcked], 3, 0, SRCCOPY);
  }
}

/*
 * @brief  �ػ��б�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void _draw_listbox(HDC hdc,HWND hwnd,COLOR_RGB32 text_c,COLOR_RGB32 back_c,COLOR_RGB32 sel_c, int fontsize)
{

	RECT rc,rc_cli;
	int i,count;
	WCHAR wbuf[128];


	GetClientRect(hwnd,&rc_cli);

	SetBrushColor(hdc,MapRGB888(hdc,back_c));
	FillRect(hdc,&rc_cli);

	SetTextColor(hdc,MapRGB888(hdc,text_c));

	i=SendMessage(hwnd,LB_GETTOPINDEX,0,0);
	count=SendMessage(hwnd,LB_GETCOUNT,0,0);

  if (fontsize == 32)
  {
    SetFont(hdc,controlFont_32);
  }
  else
  {
    SetFont(hdc,defaultFont);
  }
	

	while(i<count)
	{
		SendMessage(hwnd,LB_GETITEMRECT,i,(LPARAM)&rc);
		if(rc.y > rc_cli.h)
		{
			break;
		}

		SendMessage(hwnd,LB_GETTEXT,i,(LPARAM)wbuf);
		DrawText(hdc,wbuf,-1,&rc,DT_SINGLELINE|DT_CENTER|DT_VCENTER);

		i++;
	}



}

static void listbox_owner_draw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc,hdc0,hdc1;
	RECT rc_m;
	int x,y,w,h;

	hwnd =ds->hwnd;
	hdc =ds->hDC;

	//����һ���м�ľ��Σ�
	rc_m.w =ds->rc.w;
	rc_m.h =44;
	rc_m.x =0;
	rc_m.y =(ds->rc.h-rc_m.h)>>1;

	//��������һ����С��DC,��listbox�ֱ���ƽ�ȥ������ɫ������ͬ��.
	hdc0 =CreateMemoryDC(SURF_SCREEN,ds->rc.w,ds->rc.h);
	hdc1 =CreateMemoryDC(SURF_SCREEN,ds->rc.w,ds->rc.h);

	//һ��listbox�浽hdc0�У�
	_draw_listbox(hdc0,hwnd,RGB888(168,168,168),RGB888(250,250,250),RGB888(10,100,100), 24);

	//һ��listbox�浽hdc1�У�
	_draw_listbox(hdc1,hwnd,RGB888(236,126,65),RGB888(232,232,232),RGB888(150,200,220), 32);

	//�м����Ƶ�hdc1�У�
	SetPenColor(hdc1,MapRGB(hdc1,212,212,212));
	// DrawRect(hdc1,&rc_m);

  HLine(hdc1, rc_m.x, rc_m.y, rc_m.x + rc_m.w);
  HLine(hdc1, rc_m.x, rc_m.y+rc_m.h-1, rc_m.x + rc_m.w);

	//����ľ��β��ִ�hdc0�︴�Ƴ���.
	x =0;
	y =0;
	w =rc_m.w;
	h =rc_m.y-ds->rc.y;
	BitBlt(hdc,x,y,w,h,hdc0,x,y,SRCCOPY);

	//�м���β��ִ�hdc1�︴�Ƴ���.
	BitBlt(hdc,rc_m.x,rc_m.y,rc_m.w,rc_m.h,hdc1,rc_m.x,rc_m.y,SRCCOPY);

	//����ľ��β��ִ�hdc0�︴�Ƴ���.
	x =0;
	y =rc_m.y+rc_m.h;
	w =rc_m.w;
	h =ds->rc.h-(rc_m.y+rc_m.h);
	BitBlt(hdc,x,y,w,h,hdc0,x,y,SRCCOPY);


	DeleteDC(hdc0);
	DeleteDC(hdc1);

}

uint8_t Sec = 0;
uint8_t Min = 0;
uint8_t Hour = 0;

static void Dial_OwnerDraw(DRAWITEM_HDR *ds)  // ���Ʊ���
{
  HDC hdc;
	RECT rc, rc_tmp;
  RTC_TIME rtc_time;
  HWND hwnd;
  int clock_back;
  BITMAP clock_s, clock_m, clock_h;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  hwnd = ds->hwnd;

  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);
  
  RTC_GetTime(RTC_Format_BIN, &rtc_time.RTC_Time);
//  RTC_GetDate(RTC_Format_BIN, &rtc_time.RTC_Date);
//  drv_clock.Time_flag=1;
//  drv_clock.Date_flag=1;
  Hour=rtc_time.RTC_Time.RTC_Hours;
  Min=rtc_time.RTC_Time.RTC_Minutes;
  Sec=rtc_time.RTC_Time.RTC_Seconds;
//  drv_clock.Date_day=rtc_time.RTC_Date.RTC_Date;
//  drv_clock.Date_month=rtc_time.RTC_Date.RTC_Month;
//  drv_clock.Date_year=rtc_time.RTC_Date.RTC_Year;
  /* Unfreeze the RTC DR Register */
  (void)RTC->DR;

  if (clock_dial == 0)    
  {
    clock_back = hdc_clock_back_00;
    clock_h = bm_clock_h_00;
    clock_m = bm_clock_m_00;
    clock_s = bm_clock_s_00;
  }
  else if (clock_dial == 1)
  {
    clock_back = hdc_clock_back_01;
    clock_h = bm_clock_h_01;
    clock_m = bm_clock_m_01;
    clock_s = bm_clock_s_01;
  }
  else if (clock_dial == 2)
  {
    clock_back = hdc_clock_back_02;
    clock_h = bm_clock_h_02;
    clock_m = bm_clock_m_02;
    clock_s = bm_clock_s_02;
  }

  BitBlt(hdc, 0, 0, 233, 243, hdc_clock_png[clock_back], 0, 0, SRCCOPY);
  EnableAntiAlias(hdc, TRUE);
  Hour = Hour>12 ? Hour-12 : Hour;
  RotateBitmap(hdc, 116, 117, &clock_h, Hour / 12.0 * 360 + 360.0 / 12.0 * Min / 60.0);
  RotateBitmap(hdc, 116, 117, &clock_m, Min / 60.0 * 360 + 360.0 / 60. * Sec / 60.0);
  RotateBitmap(hdc, 116, 117, &clock_s, Sec / 60.0 * 360);
  
  EnableAntiAlias(hdc, FALSE);
}

/*
 * @brief  �ػ���ͨ͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Common_Textbox_OwnerDraw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
  RECT rc,rc_tmp;
	WCHAR wbuf[128];
  HDC hdc_temp;

  hdc_temp = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, GUI_XSIZE, GUI_YSIZE);
  ClrDisplay(hdc_temp, NULL, 0);
  BitBlt(hdc_temp, 0, 0, GUI_XSIZE, GUI_YSIZE, hdc_clock_bk, 0, 0, SRCCOPY);
  BitBlt(hdc_temp, 497, 134, 220, 240, hdc_clock_png[hdc_clock_calendar], 0, 0, SRCCOPY);

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.

	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_temp, rc_tmp.x, rc_tmp.y, SRCCOPY);

	SetTextColor(hdc, MapRGB(hdc, 0, 0, 0));
	GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)

  DeleteDC(hdc_temp);
}

/*
 * @brief  �ػ����͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Title_Textbox_OwnerDraw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
  RECT rc,rc_tmp;
	WCHAR wbuf[128];

  hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

  /* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

	SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
	GetWindowText(hwnd, wbuf, 128);                        // ��ð�ť�ؼ�������
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
}

/*
 * @brief  �ػ��ɫ�����ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void WhiteBK_Textbox_OwnerDraw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
  RECT rc;
	WCHAR wbuf[128];

  hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

  /* ���� */
  SetBrushColor(hdc, MapRGB(hdc, 250, 250, 250));
  FillRect(hdc, &rc);

	SetTextColor(hdc, MapRGB(hdc, 10, 10, 10));
	GetWindowText(hwnd, wbuf, 128);                        // ��ð�ť�ؼ�������
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
}

/*
 * @brief  �ػ�����͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Num_Textbox_OwnerDraw(DRAWITEM_HDR *ds)
{
  HWND hwnd;
	HDC hdc;
  RECT rc,rc_tmp;
	WCHAR wbuf[128];

  HDC hdc_temp;

  hdc_temp = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, GUI_XSIZE, GUI_YSIZE);
  ClrDisplay(hdc_temp, NULL, 0);
  BitBlt(hdc_temp, 0, 0, GUI_XSIZE, GUI_YSIZE, hdc_clock_bk, 0, 0, SRCCOPY);
  BitBlt(hdc_temp, 497, 134, 220, 240, hdc_clock_png[hdc_clock_calendar], 0, 0, SRCCOPY);

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.

	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_temp, rc_tmp.x, rc_tmp.y, SRCCOPY);

  SetFont(hdc, controlFont_72);
	SetTextColor(hdc, MapRGB(hdc, 0, 0, 0));
	GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
  SetFont(hdc, defaultFont);

  DeleteDC(hdc_temp);
}

/*
 * @brief  ����б�ѡ�������ֵ���ַ���ת���ͣ�
 * @param  hwnd:	���ھ��
 * @param  ID��   �б� ID
 * @retval NONE
*/
static uint16_t GetListCurselVal(HWND hwnd, uint32_t ID)
{
  HWND wnd;
  uint8_t csl;    // ѡ����
  WCHAR wbuf[10];
  char cbuf[10];

  wnd = GetDlgItem(hwnd, ID);
  csl = SendMessage(wnd, LB_GETCURSEL, 0, 0);    // ���ѡ����
  SendMessage(wnd, LB_GETTEXT, csl, (LPARAM)wbuf);       // ���ѡ������ı�
  x_wcstombs_cp936(cbuf, wbuf, 9);

  return x_atoi(cbuf);                           // ����ѡ�����ֵ
}

/*
 * @brief  ����б�ָ�������ֵ���ַ���ת���ͣ�
 * @param  hwnd:	���ھ��
 * @param  ID��   �б� ID
 * @param  csl:   �б����
 * @retval NONE
*/
//static uint16_t GetListItemVal(HWND hwnd, uint32_t ID, uint32_t csl)
//{
//  HWND wnd;
//  WCHAR wbuf[10];
//  char cbuf[10];

//  wnd = GetDlgItem(hwnd, ID);
//  SendMessage(wnd, LB_GETTEXT, csl, (LPARAM)wbuf);       // ���ѡ������ı�
//  x_wcstombs_cp936(cbuf, wbuf, 9);

//  return x_atoi(cbuf);                           // ����ѡ�����ֵ
//}

static LRESULT setting_win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg){
      case WM_CREATE:
      {
        Set_Start.dial = 0;
        Set_Start.page = 0;

        for (uint8_t xC=7; xC<11; xC++)     //  ��ť
        {
          /* ѭ��������ť */
          CreateWindow(BUTTON, clock_icon[xC].icon_name, WS_OWNERDRAW | WS_VISIBLE,
                        clock_icon[xC].rc.x, clock_icon[xC].rc.y,
                        clock_icon[xC].rc.w, clock_icon[xC].rc.h,
                        hwnd, clock_icon[xC].id, NULL, NULL); 
        }

        ShowWindow(GetDlgItem(hwnd, ID_CLOCK_BACK), SW_HIDE);    // ������һ����ť

        for (uint8_t xC=11; xC<14; xC++)
        {
          /* ѭ�������ı��� */
          CreateWindow(TEXTBOX, clock_icon[xC].icon_name, WS_OWNERDRAW,
                        clock_icon[xC].rc.x, clock_icon[xC].rc.y,
                        clock_icon[xC].rc.w,clock_icon[xC].rc.h,
                        hwnd, clock_icon[xC].id, NULL, NULL);
        }
        ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETTITLE), SW_SHOW);    // ������һ����ť
         
        for (uint8_t xC=14; xC<17; xC++)
        {
          /* ѭ����ѡ��ť */
          CreateWindow(BUTTON, clock_icon[xC].icon_name, WS_OWNERDRAW | WS_VISIBLE | BS_RADIOBOX,
                        clock_icon[xC].rc.x, clock_icon[xC].rc.y,
                        clock_icon[xC].rc.w, clock_icon[xC].rc.h,
                        hwnd, clock_icon[xC].id | (1 << 16), NULL, NULL); 
        }

        SendMessage(GetDlgItem(hwnd, ID_CLOCK_Background00), BM_SETSTATE, BST_CHECKED, 0);    // Ĭ��ѡ�е�һ��

        for (uint8_t xC=17; xC<22; xC++)
        {
          /* ���������б�����Ϊ�������ɼ��� */
          CreateWindow(LISTBOX, clock_icon[xC].icon_name, WS_OWNERDRAW | LBS_NOTIFY,
                            clock_icon[xC].rc.x, clock_icon[xC].rc.y, clock_icon[xC].rc.w,
                            clock_icon[xC].rc.h, hwnd, clock_icon[xC].id, NULL, NULL);
        }

        for (uint8_t xC=0; xC<24; xC++)
        {
          WCHAR wbuf[10];
          HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetHour);

          /* ���б�������Сʱ */
          x_wsprintf(wbuf, L"%02d", xC);
          SendMessage(ListHwnd, LB_ADDSTRING, xC, (LPARAM)wbuf);
          SendMessage(ListHwnd, LB_SETITEMHEIGHT, xC, (LPARAM)44);
        }
        
        for (uint8_t xC=0; xC<60; xC++)
        {
          WCHAR wbuf[10];
          HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetMinute);

          /* ���б������ӷ��� */
          x_wsprintf(wbuf, L"%02d", xC);
          SendMessage(ListHwnd, LB_ADDSTRING, xC, (LPARAM)wbuf);
          SendMessage(ListHwnd, LB_SETITEMHEIGHT, xC, (LPARAM)44);
        }

        uint8_t i=0;
        for (uint16_t xC=2000; xC<2099; xC++)
        {
          WCHAR wbuf[10];
          
          HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetYear);

          /* ���б��������� */
          x_wsprintf(wbuf, L"%d", xC);
          SendMessage(ListHwnd, LB_ADDSTRING, i, (LPARAM)wbuf);
          SendMessage(ListHwnd, LB_SETITEMHEIGHT, i++, (LPARAM)44);
        }

        i=0;
        for (uint8_t xC=1; xC<13; xC++)
        {
          WCHAR wbuf[10];
          HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetMonth);

          /* ���б��������� */
          x_wsprintf(wbuf, L"%d", xC);
          SendMessage(ListHwnd, LB_ADDSTRING, i, (LPARAM)wbuf);
          SendMessage(ListHwnd, LB_SETITEMHEIGHT, i++, (LPARAM)44);
        }

        i=0;
        for (uint8_t xC=1; xC<32; xC++)
        {
          WCHAR wbuf[10];
          HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetDate);

          /* ���б����������� */
          x_wsprintf(wbuf, L"%d", xC);
          SendMessage(ListHwnd, LB_ADDSTRING, i, (LPARAM)wbuf);
          SendMessage(ListHwnd, LB_SETITEMHEIGHT, i++, (LPARAM)44);
        }

        SetTimer(hwnd, 1, 50, TMR_START, NULL);
        
        break;
      }

      case WM_TIMER:
      {
        int cur_sel, i, y;
        HWND wnd;
        RECT rc0, rc;

        if(GetKeyState(VK_LBUTTON)==0) //����������������û�а���ʱ.
        {
          for (uint32_t xC=ID_CLOCK_SetYear; xC<ID_CLOCK_SetMinute+1; xC++)
          {
            wnd =GetDlgItem(hwnd, xC); //���LISTBOX ����ʱ��HWND.

            cur_sel=SendMessage(wnd,LB_GETCURSEL,0,0); //���LISTBOX����.

            GetClientRect(wnd,&rc0);
            SendMessage(wnd,LB_GETITEMRECT,cur_sel,(LPARAM)&rc); //��ö���ľ���λ��.

            y = rc.y+(rc.h/2);

            if(y < (rc0.h/2)) //���ѡ�е�����Listbox�����ߵ�����,�������ƶ�.
            {
              if(abs(y-(rc0.h/2)) > (rc.h>>2))
              {//�����߸���Զ����ÿ�ζ���һ�����(����ȥ�ƶ��ٶȿ�)��
                //i =rc.h>>2;
                i = 4;
              }
              else
              {//�����߸��úܽ��ˣ���ֻ��1��offset(�����ƶ�)��
                i = 1;
              }

              SendMessage(wnd,LB_OFFSETPOS,TRUE,i);
            }

            if(y > (rc0.h/2)) //���ѡ�е�����Listbox�����ߵ�����,�������ƶ�.
            {
              if(abs(y-(rc0.h/2)) > (rc.h>>2))
              {//�����߸���Զ����ÿ�ζ���һ�����(����ȥ�ƶ��ٶȿ�)��
                //i =rc.h>>2;
                i = 4;
              }
              else
              {//�����߸��úܽ��ˣ���ֻ��1��offset(�����ƶ�)��
                i = 1;
              }

              SendMessage(wnd,LB_OFFSETPOS,TRUE,-i);
            }
          }
        }
        
      }  
			break;     
      
      case WM_NOTIFY:
      {
         u16 code, id;
         id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
         code=HIWORD(wParam);//��ȡ��Ϣ������
         
         NMHDR *nr;
         nr =(NMHDR*)lParam;      

         //���͵���
        if(code == BN_CLICKED)
        {
          switch (id)
          {
            /* �˳���ť���� */
            case ID_CLOCK_WAIVE:
            {
              PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
            }
            break;

            /* ������ð�ť���� */
            case ID_CLOCK_OK:
            {
              if (Set_Start.page == 0)    // ֻ���ñ���
              {
                clock_dial = Set_Start.dial;    // ���ñ���
              }
              else if (Set_Start.page == 1)    // �޸ı��̺�ʱ��
              {
                RTC_TimeTypeDef RTC_Time;

                clock_dial = Set_Start.dial;    // ���ñ���
                RTC_Time.RTC_Hours = GetListCurselVal(hwnd, ID_CLOCK_SetHour);        // ��ȡ�б��ʱ
                RTC_Time.RTC_Minutes = GetListCurselVal(hwnd, ID_CLOCK_SetMinute);    // ��ȡ�б�ķ�
                RTC_Time.RTC_Seconds = 0;

                RTC_SetTime(RTC_Format_BIN, &RTC_Time);    // ����ʱ��
              }
              else if (Set_Start.page == 2)    // �޸ı��̡�ʱ�������
              {
                RTC_DateTypeDef RTC_Date;
                RTC_TimeTypeDef RTC_Time;

                clock_dial = Set_Start.dial;    // ���ñ���

                RTC_Time.RTC_Hours = GetListCurselVal(hwnd, ID_CLOCK_SetHour);        // ��ȡ�б��ʱ
                RTC_Time.RTC_Minutes = GetListCurselVal(hwnd, ID_CLOCK_SetMinute);    // ��ȡ�б�ķ�
                RTC_Time.RTC_Seconds = 0;
                RTC_SetTime(RTC_Format_BIN, &RTC_Time);    // ����ʱ��

                RTC_Date.RTC_Year = GetListCurselVal(hwnd, ID_CLOCK_SetYear) - 2000;    // ��ȡ�б����
                RTC_Date.RTC_Month = GetListCurselVal(hwnd, ID_CLOCK_SetMonth);         // ��ȡ�б����
                RTC_Date.RTC_Date = GetListCurselVal(hwnd, ID_CLOCK_SetDate);           // ��ȡ�б����
                /* ��ķ����ɭ�ܼ��㹫ʽ */
                RTC_Date.RTC_WeekDay = (RTC_Date.RTC_Date + 2 * RTC_Date.RTC_Month + 3     \
                                        * (RTC_Date.RTC_Month + 1) / 5 + RTC_Date.RTC_Year \
                                        + RTC_Date.RTC_Year / 4 - RTC_Date.RTC_Year / 100  \
                                        + RTC_Date.RTC_Year / 400) % 7 + 1;
                RTC_SetDate(RTC_Format_BIN, &RTC_Date);                                 // ��������

                /* ���õ�ǰ��ʾ���� */
                WCHAR wbuf[5];
                
                x_wsprintf(wbuf, L"%d", RTC_Date.RTC_Date);
                SetWindowText(GetDlgItem(clock_hwnd, ID_CLOCK_DAY), wbuf);    // ��������
                
                x_wsprintf(wbuf, L"%d��", RTC_Date.RTC_Month);
                SetWindowText(GetDlgItem(clock_hwnd, ID_CLOCK_MONTH), wbuf);    // ������
                
                SetWindowText(GetDlgItem(clock_hwnd, ID_CLOCK_WEEK), Week_List[RTC_Date.RTC_WeekDay - 1]);    // ��������
              }

              PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
            }
            break;

            /* ����ѡ�� */
            case ID_CLOCK_Background00:
            {
              Set_Start.dial = 0;    // ѡ����� 1
            }
            break;

            /* ����ѡ�� */
            case ID_CLOCK_Background01:
            {
              Set_Start.dial = 1;    // ѡ����� 2
            }
            break;

            /* ����ѡ�� */
            case ID_CLOCK_Background02:
            {
              Set_Start.dial = 2;    // ѡ����� 3
            }
            break;

            /* ��һ�� */
            case ID_CLOCK_NEXT:
            {
              if (Set_Start.page == 0)
              {
                Set_Start.page = 1;    // ���Ϊʱ�����ý���
                SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETTITLE), L"����ʱ��");

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background00), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background01), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background02), SW_HIDE);

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetHour),      SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMinute),    SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_BACK),         SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETTIME),      SW_SHOW);
              }
              else if (Set_Start.page == 1)
              {
                Set_Start.page = 2;    // ���Ϊ�������ý���
                SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETTITLE), L"��������");

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetHour),   SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMinute), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_NEXT),      SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETTIME),   SW_HIDE);

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetDate),  SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetYear),  SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMonth), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETDATE),  SW_SHOW);
              }
              
            }
            break;

            /* ��һ�� */
            case ID_CLOCK_BACK:
            {
              if (Set_Start.page == 2)
              {
                Set_Start.page = 1;    // ���Ϊʱ�����ý���
                SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETTITLE), L"����ʱ��");

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetHour),   SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMinute), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_NEXT),      SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETTIME),  SW_SHOW);

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetDate),  SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetYear),  SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMonth), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETDATE),  SW_HIDE);
              }
              else if (Set_Start.page == 1)
              {
                Set_Start.page = 0;    // ���Ϊ����ѡ�����
                SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETTITLE), L"����ѡ��");

                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background00), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background01), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_Background02), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetHour),      SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SetMinute),    SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_BACK),         SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, ID_CLOCK_SETTIME),      SW_HIDE);
              }
            }
            break;
          }
        }

        /* �б��� */
        if (id >= ID_CLOCK_SetYear && id <= ID_CLOCK_SetMinute)
        {
          if(nr->code == LBN_SELCHANGE)    // �б�ѡ����ı�
          {
            
          }
          
          if(nr->code == LBN_POSCHANGE)    // ���б�λ�ñ��ı�
          {
            RECT rc0,rc;
            HWND wnd;
            int i;

            wnd = GetDlgItem(hwnd, id);     // ��ñ��ı��б�� HWND

            GetClientRect(wnd,&rc0);
            /* ʹ�б�ѭ�� */
            i = 0;
            SendMessage(wnd,LB_GETITEMRECT,i,(LPARAM)&rc);
            if(rc.y > 2)
            {
              i =SendMessage(wnd,LB_GETCOUNT,0,0);
              i-=1;
              SendMessage(wnd,LB_MOVEINDEX,i,0);
            }
            else
            {
              i =SendMessage(wnd,LB_GETCOUNT,0,0);
              i-=1;
              SendMessage(wnd,LB_GETITEMRECT,i,(LPARAM)&rc);
              if((rc.y+rc.h) < (rc0.h-8))
              {
                SendMessage(wnd,LB_MOVEINDEX,0,0xFFFF);
              }
            }

            /* ���ÿ����м���Ϊѡ���� */
            int min = 0;    // ��¼����ģ�����ѡ���
            int len = 0xFFFFFFF;
            int count = SendMessage(wnd,LB_GETCOUNT,0,0);
            i = SendMessage(wnd, LB_GETTOPINDEX, 0, 0);            // �׸���ʾ������Ŀ
            
            while (1)
            {
              SendMessage(wnd, LB_GETITEMRECT, i, (LPARAM)&rc);    // �������Ŀ�ľ��β���

              if (rc.y >= rc0.h || i >= count - 1)          // �ж��ǲ�������ʾ������
              {
                SendMessage(wnd, LB_SETCURSEL, min, 0);     // ���õ�ǰѡ�е�����Ŀ
                break;                                      // ��������ʾ���򣬽���ѭ��
              }

              rc.y += rc.h/2;                // ����������м�λ��
              rc.y = abs(rc.y - rc0.h/2);    // ���㵱ǰ��м�ľ���

              if (len > rc.y)
              {
                len = rc.y;    // ��¼��С����
                min = i;       // ��¼��С��
              }
              i++;         // ������һ��
              if (i>=count) 
              {
                i = 0;
              }
            }

            WCHAR wbuf[5];
            if (id >= ID_CLOCK_SetYear && id <= ID_CLOCK_SetDate)    // ������������
            {
              x_wsprintf(wbuf, L"%d��%02d��%02d��", GetListCurselVal(hwnd, ID_CLOCK_SetYear),
                                                    GetListCurselVal(hwnd, ID_CLOCK_SetMonth),
                                                    GetListCurselVal(hwnd, ID_CLOCK_SetDate));

              SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETDATE), wbuf);    // ��������
            }
            else    // ��������ʱ��
            {
              x_wsprintf(wbuf, L"%02d:%02d", GetListCurselVal(hwnd, ID_CLOCK_SetHour),
                                             GetListCurselVal(hwnd, ID_CLOCK_SetMinute));

              SetWindowText(GetDlgItem(hwnd, ID_CLOCK_SETTIME), wbuf);    // ��������
            }
      
            uint8_t month = 0;
            month = GetListCurselVal(hwnd, ID_CLOCK_SetMonth);    // ���ѡ�е���
            if (id == ID_CLOCK_SetMonth || (id == ID_CLOCK_SetYear && month == 2))    // �����·ݣ���ͬ�·����������
            {
              uint8_t date_max = 0;

              date_max = month_list[month - 1];

              if (month == 2)    // ����ƽ�� ����
              {
                uint16_t year = GetListCurselVal(hwnd, ID_CLOCK_SetYear);    // ���ѡ�е���

                if( year % 400 == 0 || (year % 4 ==  0 && year % 100 != 0))
                {
                  date_max = 29;    // ����
                }
              }

              HWND ListHwnd = GetDlgItem(hwnd, ID_CLOCK_SetDate);

              SendMessage(ListHwnd, LB_RESETCONTENT, 0, 0);

              for (uint8_t xC=1; xC<date_max+1; xC++)
              {
                WCHAR wbuf[10];

                /* ���б����������� */
                x_wsprintf(wbuf, L"%d", xC);
                SendMessage(ListHwnd, LB_ADDSTRING, xC-1, (LPARAM)wbuf);
                SendMessage(ListHwnd, LB_SETITEMHEIGHT, xC-1, (LPARAM)44);
              }
              SendMessage(ListHwnd, LB_SETCURSEL, 2, 0);

              // uint8_t list_conut = SendMessage(ListHwnd, LB_GETCOUNT, 0, 0);
              // uint8_t DateCursel = GetListCurselVal(hwnd, ID_CLOCK_SetDate);    // ���ѡ�е�����
              // uint8_t max = 0;
              // uint8_t item = 0;

              // if (list_conut > date_max)
              // {
              //   /* �б���Ŀ��ʵ��Ҫ����Ҫɾ�� */
              //   for (uint8_t xC=0; xC<list_conut; xC++)
              //   {
              //     if (GetListItemVal(hwnd, ID_CLOCK_SetDate, xC) > date_max)
              //     {
              //       SendMessage(ListHwnd, LB_DELETESTRING, xC, 0);
              //     }
              //   }
              // }
              // else if (list_conut < date_max)
              // {
              //   /* �б���Ŀ��ʵ��Ҫ����Ҫ���� */
              //   for (uint8_t xC=0; xC<list_conut; xC++)    // �ҵ������Ŀ
              //   {
              //     if (GetListItemVal(hwnd, ID_CLOCK_SetDate, xC) > max)
              //     {
              //       item = xC;    // ��¼�ϴ���Ŀ
              //       max = GetListItemVal(hwnd, ID_CLOCK_SetDate, item);       // �õ�����������
              //     }
              //   }

                
              //   list_conut = SendMessage(ListHwnd, LB_GETCOUNT, 0, 0);    // �õ����е���Ŀ��
              //   uint8_t ii = max;    // ��Ŀ

              //   for (uint8_t xY=item; xY<list_conut+date_max-max+1; xY++)    // ��������Ŀ����������Ҫ���ӵ���Ŀ
              //   {
              //     x_wsprintf(wbuf, L"%d", ii+1);
              //     ii++;
              //     SendMessage(ListHwnd, LB_ADDSTRING, xY, (LPARAM)wbuf);      // ������Ŀ
              //     SendMessage(ListHwnd, LB_SETITEMHEIGHT, xY, (LPARAM)45);    // ������Ŀ�߶�
              //   }
              // }

              // /* �����б��ѡ����Ŀ */
              // list_conut = SendMessage(ListHwnd, LB_GETCOUNT, 0, 0);
              // max = 0;
              // uint8_t xC=0;
              // for (xC=0; xC<list_conut; xC++)    // �ҵ������Ŀ
              // {
              //   if (GetListItemVal(hwnd, ID_CLOCK_SetDate, xC) == DateCursel)
              //   {
              //     SendMessage(ListHwnd, LB_SETCURSEL, xC, 0);    // ����ѡ�е���Ŀ
              //     break;
              //   }

              //   if (GetListItemVal(hwnd, ID_CLOCK_SetDate, xC) > max)
              //   {
              //     item = xC;    // ��¼�ϴ���Ŀ
              //     max = GetListItemVal(hwnd, ID_CLOCK_SetDate, xC);
              //   }
              // }

              // if (xC >= list_conut)    // û�ҵ�ԭ��������������Ϊѡ����
              // {
              //   SendMessage(ListHwnd, LB_SETCURSEL, item, 0);    // ����ѡ�е���Ŀ
              // }
              // printf("xC=%d,item=%d\n", xC, item);
            }
          }
        }

      break;
    }

      // �ػ��ƺ�����Ϣ
      case WM_DRAWITEM:
      {
        DRAWITEM_HDR *ds;
        ds = (DRAWITEM_HDR*)lParam;        
        if (ds->ID >= ID_CLOCK_BACK && ds->ID <= ID_CLOCK_OK)
        {
          btn_owner_draw(ds);             // �ػ水ť
          return TRUE;
        }
        else if (ds->ID == ID_CLOCK_SETTITLE)
        {
          Title_Textbox_OwnerDraw(ds);
          return TRUE;
        }
        else if (ds->ID == ID_CLOCK_SETTIME || ds->ID == ID_CLOCK_SETDATE)
        {
          WhiteBK_Textbox_OwnerDraw(ds);
          return TRUE;
        }
        else if (ds->ID == ID_CLOCK_WAIVE)
        {
          waive_btn_owner_draw(ds);
          return TRUE;
        }
        else if (ds->ID >= ID_CLOCK_Background00 && ds->ID <= ID_CLOCK_Background02)
        {
          radiobox_owner_draw(ds, ds->ID);
          return TRUE;
        }
        else if (ds->ID >= ID_CLOCK_SetYear && ds->ID <= ID_CLOCK_SetMinute)
        {
          listbox_owner_draw(ds);
          return TRUE;
        }

        return FALSE;
      }     
      
      //���ƴ��ڽ�����Ϣ
      case WM_PAINT:
      {
        PAINTSTRUCT ps;
//        HDC hdc;//��Ļhdc

        //��ʼ����
        BeginPaint(hwnd, &ps); 
        
        EndPaint(hwnd, &ps);
      }
      break;

      case WM_ERASEBKGND:
      {
        HDC hdc =(HDC)wParam;
        RECT rc =*(RECT*)lParam;
        RECT rc1 = {230, 102, 339, 272};

        BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc.x, rc.y, SRCCOPY);

        if (Set_Start.page != 0)     // ���Ǳ���ѡ��
        {
          SetBrushColor(hdc, MapRGB(hdc, 250, 250, 250));
          EnableAntiAlias(hdc, TRUE);
          FillRoundRect(hdc, &rc1, 10);    // ���б���
          EnableAntiAlias(hdc, FALSE);
        }

        return TRUE;
      }

      //�رմ�����Ϣ����case
      case WM_CLOSE:
      {   
        DestroyWindow(hwnd);
        return TRUE;	
      }
    
      //�رմ�����Ϣ����case
      case WM_DESTROY:
      {
        SetTimer(clock_hwnd, 1, 400, TMR_START, NULL);

        return PostQuitMessage(hwnd);		
      }
      
      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
     
   return WM_NULL;
}

static LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
  static uint8_t Load_Flag = 1;
   switch(msg){
      case WM_CREATE:
      {
        for (uint8_t xC=0; xC<ICON_BTN_NUM; xC++)     //  ��ť
        {
          /* ѭ��������ť */
          CreateWindow(BUTTON, clock_icon[xC].icon_name, WS_OWNERDRAW|WS_VISIBLE,
                        clock_icon[xC].rc.x, clock_icon[xC].rc.y,
                        clock_icon[xC].rc.w, clock_icon[xC].rc.h,
                        hwnd, clock_icon[xC].id, NULL, NULL); 
        }

        for (uint8_t xC=ICON_BTN_NUM; xC<ICON_TEXT_NUM; xC++)    // �ı���
        {
          /* ѭ�������ı��� */
          CreateWindow(TEXTBOX, clock_icon[xC].icon_name, WS_OWNERDRAW|WS_VISIBLE,
                        clock_icon[xC].rc.x, clock_icon[xC].rc.y,
                        clock_icon[xC].rc.w,clock_icon[xC].rc.h,
                        hwnd, clock_icon[xC].id, NULL, NULL); 
        }

        /* ��ʼ������ */
        WCHAR wbuf[5];
        RTC_TIME rtc_time;
        RTC_GetDate(RTC_Format_BIN, &rtc_time.RTC_Date);
        
        x_wsprintf(wbuf, L"%d", rtc_time.RTC_Date.RTC_Date);
        SetWindowText(GetDlgItem(hwnd, ID_CLOCK_DAY), wbuf);    // ��������
        
        x_wsprintf(wbuf, L"%d��", rtc_time.RTC_Date.RTC_Month);
        SetWindowText(GetDlgItem(hwnd, ID_CLOCK_MONTH), wbuf);    // ������
        
        SetWindowText(GetDlgItem(hwnd, ID_CLOCK_WEEK), Week_List[rtc_time.RTC_Date.RTC_WeekDay - 1]);    // ��������

        SetTimer(hwnd, 1, 400, TMR_START, NULL);
        
        /* ת����bitmap */
        DCtoBitmap(hdc_clock_png[hdc_clock_s_00], &bm_clock_s_00);
        DCtoBitmap(hdc_clock_png[hdc_clock_h_00], &bm_clock_h_00);
        DCtoBitmap(hdc_clock_png[hdc_clock_m_00], &bm_clock_m_00);
        DCtoBitmap(hdc_clock_png[hdc_clock_s_01], &bm_clock_s_01);
        DCtoBitmap(hdc_clock_png[hdc_clock_h_01], &bm_clock_h_01);
        DCtoBitmap(hdc_clock_png[hdc_clock_m_01], &bm_clock_m_01);
        DCtoBitmap(hdc_clock_png[hdc_clock_s_02], &bm_clock_s_02);
        DCtoBitmap(hdc_clock_png[hdc_clock_h_02], &bm_clock_h_02);
        DCtoBitmap(hdc_clock_png[hdc_clock_m_02], &bm_clock_m_02);
        
        // SetTimer(hwnd, 2, 10, TMR_START|TMR_SINGLE, NULL);    // ��Դ���ض�ʱ��

        break;
      }

      case WM_TIMER:
      {
        int tmr_id;

				tmr_id = wParam;    // ��ʱ�� ID

				if (tmr_id == 1)
        {
          WCHAR wbuf[5];
          
          RTC_TIME rtc_time;
          RTC_GetDate(RTC_Format_BIN, &rtc_time.RTC_Date);
          
          x_wsprintf(wbuf, L"%d", rtc_time.RTC_Date.RTC_Date);
          SetWindowText(GetDlgItem(hwnd, ID_CLOCK_DAY), wbuf);    // ��������
          
          x_wsprintf(wbuf, L"%d��", rtc_time.RTC_Date.RTC_Month);
          SetWindowText(GetDlgItem(hwnd, ID_CLOCK_MONTH), wbuf);    // ������
          
          SetWindowText(GetDlgItem(hwnd, ID_CLOCK_WEEK), Week_List[rtc_time.RTC_Date.RTC_WeekDay - 1]);    // ��������
          /* Unfreeze the RTC DR Register */
          (void)RTC->DR;
          
          InvalidateRect(GetDlgItem(hwnd, ID_CLOCK_TIME), NULL, TRUE);
        }
//        else if (tmr_id == 2)
//        {
//          u8 *jpeg_buf;
//          u32 jpeg_size;
//          JPG_DEC *dec;
//          BOOL res = NULL;

//          res = RES_Load_Content(GUI_CLOCK_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
//          // res = FS_Load_Content(GUI_CLOCK_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
//          hdc_clock_bk = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
//          if(res)
//          {
//            /* ����ͼƬ���ݴ���JPG_DEC��� */
//            dec = JPG_Open(jpeg_buf, jpeg_size);

//            /* �������ڴ���� */
//            JPG_Draw(hdc_clock_bk, 0, 0, dec);

//            /* �ر�JPG_DEC��� */
//            JPG_Close(dec);
//          }
//          /* �ͷ�ͼƬ���ݿռ� */
//          RES_Release_Content((char **)&jpeg_buf);

//          u8 *pic_buf;
//          u32 pic_size;
//          PNG_DEC *png_dec;
//          BITMAP png_bm;
//          
//          for (uint8_t xC=0; xC<hdc_clock_end; xC++)
//          {
//            /* ���� HDC */
//            hdc_clock_png[clock_png_info[xC].id] = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, clock_png_info[xC].w, clock_png_info[xC].h);
//            ClrDisplay(hdc_clock_png[clock_png_info[xC].id], NULL, 0);
//            res = RES_Load_Content(clock_png_info[xC].pic_name, (char**)&pic_buf, &pic_size);
//            // res = FS_Load_Content(clock_png_info[xC].pic_name, (char**)&pic_buf, &pic_size);
//            if(res)
//            {
//              png_dec = PNG_Open(pic_buf);
//              PNG_GetBitmap(png_dec, &png_bm);
//              DrawBitmap(hdc_clock_png[clock_png_info[xC].id], 0, 0, &png_bm, NULL);
//              PNG_Close(png_dec);
//            }
//            /* �ͷ�ͼƬ���ݿռ� */
//            RES_Release_Content((char **)&pic_buf);
//          }

//          /* ת����bitmap */
//          DCtoBitmap(hdc_clock_png[hdc_clock_s_00], &bm_clock_s_00);
//          DCtoBitmap(hdc_clock_png[hdc_clock_h_00], &bm_clock_h_00);
//          DCtoBitmap(hdc_clock_png[hdc_clock_m_00], &bm_clock_m_00);
//          DCtoBitmap(hdc_clock_png[hdc_clock_s_01], &bm_clock_s_01);
//          DCtoBitmap(hdc_clock_png[hdc_clock_h_01], &bm_clock_h_01);
//          DCtoBitmap(hdc_clock_png[hdc_clock_m_01], &bm_clock_m_01);
//          DCtoBitmap(hdc_clock_png[hdc_clock_s_02], &bm_clock_s_02);
//          DCtoBitmap(hdc_clock_png[hdc_clock_h_02], &bm_clock_h_02);
//          DCtoBitmap(hdc_clock_png[hdc_clock_m_02], &bm_clock_m_02);

//          Load_Flag = 1;    // ��־��Դ�������
//          for (uint32_t xC=0; xC<7; xC++)
//          {
//            ShowWindow(GetDlgItem(hwnd, clock_icon[xC].id), SW_SHOW);    // ��Դ������ɣ���ʾ��ҳ��ȫ���ؼ�
//          }
//          InvalidateRect(hwnd, NULL, TRUE);    // �ػ洰��
//        }
      }  
			break;     
      
      case WM_NOTIFY:
      {
         u16 code,  id;
         id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
         code=HIWORD(wParam);//��ȡ��Ϣ������   

         //���͵���
        if(code == BN_CLICKED)
        {
          switch (id)
          {
            /* �˳���ť���� */
            case ID_CLOCK_EXIT:
            {
              PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
            }
            break;

            /* ���ð�ť���� */
            case ID_CLOCK_SET:
            {
              KillTimer(hwnd, 1);
              /* �������ô��� */
              WNDCLASS wcex;
              RECT rc;

              wcex.Tag	 		= WNDCLASS_TAG;

              wcex.Style			= CS_HREDRAW | CS_VREDRAW;
              wcex.lpfnWndProc	= (WNDPROC)setting_win_proc;
              wcex.cbClsExtra		= 0;
              wcex.cbWndExtra		= 0;
              wcex.hInstance		= NULL;
              wcex.hIcon			= NULL;
              wcex.hCursor		= NULL;
              
              rc.x = 0;
              rc.y = 0;
              rc.w = GUI_XSIZE;
              rc.h = GUI_YSIZE;
              
              // ����"����"����.
              CreateWindow(&wcex, L"---", WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS,
                           rc.x, rc.y, rc.w, rc.h, hwnd, ID_CLOCK_SetWin, NULL, NULL);
            }
            break;
          }
        }
        break;
      }

      //�ػ��ƺ�����Ϣ
      case WM_DRAWITEM:
      {
         DRAWITEM_HDR *ds;
         ds = (DRAWITEM_HDR*)lParam;        
         if(ds->ID == ID_CLOCK_EXIT)
         {
            exit_owner_draw(ds);
            return TRUE;
         }
         else if (ds->ID == ID_CLOCK_SET)
         {
            btn_owner_draw(ds);
            return TRUE;
         }
         else if (ds->ID == ID_CLOCK_MONTH || ds->ID == ID_CLOCK_WEEK)
         {
           Common_Textbox_OwnerDraw(ds);
           return TRUE;
         }
         else if (ds->ID == ID_CLOCK_DAY)
         {
            Num_Textbox_OwnerDraw(ds);
            return TRUE;
         }
         else if (ds->ID == ID_CLOCK_TITLE)
         {
            Title_Textbox_OwnerDraw(ds);
            return TRUE;
         }
         else if (ds->ID == ID_CLOCK_TIME)
         {
            Dial_OwnerDraw(ds);
            return TRUE;
         }

         return FALSE;
      }     
      
      //���ƴ��ڽ�����Ϣ
      case WM_PAINT:
      {
        PAINTSTRUCT ps;

        //��ʼ����
        BeginPaint(hwnd, &ps); 
        
        EndPaint(hwnd, &ps);
        break;
      }
      
      case WM_ERASEBKGND:
      {
        HDC hdc =(HDC)wParam;
        RECT rc =*(RECT*)lParam;
        RECT rc2 = {0, 0, GUI_XSIZE, GUI_YSIZE};
        
        if (Load_Flag)     // ��Դ�������
        {
          BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc.x, rc.y, SRCCOPY);
          BitBlt(hdc, 497, 134, 220, 240, hdc_clock_png[hdc_clock_calendar], 0, 0, SRCCOPY);
        }
        else
        {    /* ��Դ����δ��� */ 
          SetBrushColor(hdc, MapRGB(hdc, 10, 10, 10));
          FillRect(hdc, &rc2);
          SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
          DrawText(hdc, L"��Դ�����У����Ե�...", -1, &rc2, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
        }

        return TRUE;
      }

      //�رմ�����Ϣ����case
      case WM_CLOSE:
      {   
        DestroyWindow(hwnd);
        return TRUE;	
      }
    
      //�رմ�����Ϣ����case
      case WM_DESTROY:
      {        
//        Load_Flag = 0;
//        DeleteDC(hdc_clock_bk);
//        for (int i=0; i<hdc_clock_end; i++)
//        {
//          DeleteDC(hdc_clock_png[i]);
//        }
        return PostQuitMessage(hwnd);		
      }
      
      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
     
   return WM_NULL;
}


void GUI_CLOCK_DIALOG(void)
{ 	
	WNDCLASS	wcex;
	MSG msg;

	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = win_proc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);

	//����������
	clock_hwnd = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,//
                                    &wcex,
                                    L"GUI CLOCK DIALOG",
                                    WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                                    0, 0, GUI_XSIZE, GUI_YSIZE,
                                    NULL, NULL, NULL, NULL);

	//��ʾ������
	ShowWindow(clock_hwnd, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, clock_hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


