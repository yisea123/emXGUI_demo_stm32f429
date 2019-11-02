#include <emXGUI.h>
#include <string.h>
#include "Widget.h"
#include "./camera/bsp_ov5640.h"
#include "x_libc.h"
#include "./camera/ov5640_AF.h"


static HDC hdc_bk = NULL;//����ͼ�㣬����͸���ؼ�
extern BOOL g_dma2d_en;//DMA2Dʹ�ܱ�־λ������ͷDMEO�����ֹ
TaskHandle_t h_autofocus;//�Զ��Խ��߳�
BOOL update_flag = 0;//֡�ʸ��±�־
static RECT win_rc;//�����˵�λ����Ϣ
static int b_close=FALSE;//���ڹرձ�־λ
uint8_t fps=0;//֡��
OV5640_IDTypeDef OV5640_Camera_ID;
RECT rc_fps = {0,400,800,72};//֡����ʾ�Ӵ���
HWND Cam_hwnd;//�����ھ��
HWND SetWIN=NULL;//�������ô���
int state = 0;//��ʼ������ͷ״̬��
//uint16_t *cam_buff0;
//uint16_t *cam_buff1;
GUI_SEM *cam_sem = NULL;//����ͼ��ͬ���ź�������ֵ�ͣ�
GUI_SEM *set_sem = NULL;//�ȴ��Խ�ͬ���ź�������ֵ�ͣ�
int focus_status = 1;//�Զ��Խ���Ĭ�Ͽ���
//����ؼ�ID
enum eID
{
	eID_OK =0x1000,
	eID_SET,             //���ü�������ͷ���ڣ�
	eID_SET1,            //�Զ��Խ�
	eID_SET2,            //����
	eID_SET3,            //���Ͷ�
	eID_SET4,            //�Աȶ�
  eID_SET5,            //�ֱ���
  eID_SET6,            //����ģʽ
  eID_SET7,            //����Ч��
  eID_SCROLLBAR,       //���Ȼ�����
  eID_SCROLLBAR1,      //���ͶȻ�����
  eID_SCROLLBAR2,      //�ԱȶȻ�����
  //��ѡ��---�ֱ���
  eID_RB1,             //320*240
  eID_RB2,             //480*272
  eID_RB3,             //800*480��Ĭ�ϣ�
  //��ѡ��---����ģʽ
  eID_RB4,             //�Զ�
  eID_RB5,             //����
  eID_RB6,             //����
  eID_RB7,             //�칫��
  eID_RB8,             //����
  //��ѡ��---����Ч��
   eID_RB9,              //��ɫ
  eID_RB10,             //ůɫ
  eID_RB11,             //�ڰ�
  eID_RB12,             //����
  eID_RB13,             //��ɫ   
  eID_RB14,             //ƫ��
  eID_RB15,             //����
  eID_RB16,             //����  
  eID_TB1,             //��ǰ�ֱ�����ʾ
  eID_TB2,             //��ǰ����ģʽ��ʾ   
  eID_TB3,             //��ǰ����Ч����ʾ
   
  eID_switch,          //�Զ��Խ�����
  eID_Setting1,        //���÷ֱ��ʰ�ť
  eID_Setting2,        //���ù���ģʽ��ť
  eID_Setting3,        //��������Ч����ť
   
  eID_BT1,             //�ֱ��ʽ��淵�ذ���
  eID_BT2,             //����ģʽ���淵�ذ���
  eID_BT3,             //����Ч�����淵�ذ���
  ID_EXIT,
  ID_FPS,
};

/*
 * @brief  ��ձ�������
 * @param  hdc:    ��ͼ������
 * @param  lprc��  ��������
 * @param  hwnd: ���ƴ��ھ��
 * @retval TRUE
*/
static BOOL cbErase(HDC hdc, const RECT* lprc,HWND hwnd)
{
  SetBrushColor(hdc, MapRGB(hdc,0,0,0));
  FillRect(hdc, lprc);
  return TRUE;
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
  /***************����͸���ؼ�����*************************/
  
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  ClientToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
  ScreenToClient(SetWIN, (POINT *)&rc_tmp, 1);
  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);
  
  rc_scrollbar.x = rc.x;
  rc_scrollbar.y = rc.h/2;
  rc_scrollbar.w = rc.w;
  rc_scrollbar.h = 2;
  EnableAntiAlias(hdc, TRUE);
  SetBrushColor(hdc, MapRGB888(hdc, Page_c));
  FillRect(hdc, &rc_scrollbar);

  /* ���� */
  SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);

  SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
  FillCircle(hdc, rc.x + rc.w / 2, rc.y + rc.h / 2, rc.h / 2);
  InflateRect(&rc, -2, -2);

  SetBrushColor(hdc, MapRGB888(hdc, fore_c));
  FillCircle(hdc, rc.x + rc.w / 2, rc.y + rc.h / 2, rc.h / 2);
  EnableAntiAlias(hdc, FALSE);
}

/*
 * @brief  �Զ��廬�������ƺ���
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Cam_scrollbar_ownerdraw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
	HDC hdc_mem;
	HDC hdc_mem1;
	RECT rc;
	RECT rc_cli;

	hwnd = ds->hwnd;
	hdc = ds->hDC;
	GetClientRect(hwnd, &rc_cli);

	hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);
	hdc_mem1 = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);   	
	//���ư�ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem1, RGB888( 250, 250, 250), RGB888( 250, 250, 250), RGB888( 255, 255, 255));
	//������ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem,RGB888( 250, 250, 250), RGB888( 0, 250, 0), RGB888( 0, 250, 0));
  SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);   

	//��
	BitBlt(hdc, rc_cli.x, rc_cli.y, rc.x, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
	//��
	BitBlt(hdc, rc.x + rc.w, 0, rc_cli.w - (rc.x + rc.w) , rc_cli.h, hdc_mem1, rc.x + rc.w, 0, SRCCOPY);

	//���ƻ���
	if (ds->State & SST_THUMBTRACK)//����
	{
      BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem1, rc.x, 0, SRCCOPY);
		
	}
	else//δѡ��
	{
		BitBlt(hdc, rc.x, 0, rc.w+1, rc_cli.h, hdc_mem, rc.x, 0, SRCCOPY);
	}
	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem1);
	DeleteDC(hdc_mem);
}
/*
 * @brief  �Զ����Զ��Խ�����
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Checkbox_owner_draw(DRAWITEM_HDR *ds) 
{
	HDC hdc;
	RECT rc;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  EnableAntiAlias(hdc, TRUE);
	if (focus_status==1)//��ť�ǰ���״̬
	{ 
		SetBrushColor(hdc, MapRGB(hdc, 119, 136, 153)); 
		FillRoundRect(hdc, &rc, rc.h / 2);
		InflateRect(&rc, -3, -3);

		SetBrushColor(hdc, MapRGB(hdc, 0, 250, 0)); 
		FillRoundRect(hdc, &rc, rc.h / 2);

		GetClientRect(ds->hwnd, &rc);
		SetBrushColor(hdc, MapRGB(hdc, 119, 136, 153)); 
		FillCircle(hdc, rc.w - 15, 15, 15);


		SetBrushColor(hdc, MapRGB(hdc, 250, 250, 250)); 
		FillCircle(hdc, rc.w - 15, 15, 12);
	}
	else//��ť�ǵ���״̬
	{ 
		SetBrushColor(hdc, MapRGB(hdc, 119, 136, 153)); 
		FillRoundRect(hdc, &rc, rc.h/2);
		InflateRect(&rc, -3,  -3);

		SetBrushColor(hdc, MapRGB(hdc, 250, 250, 250)); 
		FillRoundRect(hdc, &rc, rc.h / 2); 

		GetClientRect(ds->hwnd, &rc);
		SetBrushColor(hdc, MapRGB(hdc, 119, 136, 153)); //�������ɫ(BrushColor��������Fill���͵Ļ�ͼ����)
		FillCircle(hdc, rc.x + 15, 15, 15);//�þ�����䱳��
		
		SetBrushColor(hdc, MapRGB(hdc, 250, 250, 250)); //�������ɫ(BrushColor��������Fill���͵Ļ�ͼ����)
		FillCircle(hdc, rc.x + 15, 15, 12);

	}
  EnableAntiAlias(hdc, FALSE);
}
/*
 * @brief  �Զ��尴ť���ֱ��ʣ�����ģʽ������Ч����
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Button_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];
  HFONT font_old;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

  GetWindowText(ds->hwnd, wbuf, 128); //��ð�ť�ؼ�������

  if(ds->ID >= eID_TB1 && ds->ID <= eID_TB3)
  {
    font_old = SetFont(hdc, controlFont_32);
    rc.x = 160;
    rc.w = 40;
    if(ds->State & BST_PUSHED)//���������±���ɫ
      SetTextColor(hdc, MapRGB(hdc, 192,192,192));
    else
      SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
    DrawText(hdc, L"C", -1, &rc, DT_VCENTER);
    SetFont(hdc, font_old);
    rc.x = 5;
    rc.w = 160;
    DrawText(hdc, wbuf, -1, &rc, DT_VCENTER | DT_RIGHT);

  }
  else
  {
    SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
    DrawText(hdc, wbuf, -1, &rc, DT_VCENTER | DT_LEFT);
  }

}
/*
 * @brief  �Զ�����ͨ͸����ť
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void button_owner_draw(DRAWITEM_HDR *ds) 
{
	//	HWND hwnd;
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];

	//	hwnd =ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
	SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
	
  GetWindowText(ds->hwnd, wbuf, 128); //��ð�ť�ؼ�������
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER | DT_CENTER);//��������(���ж��뷽ʽ)

}
/*
 * @brief  �Զ���������ð�ť
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/

static void BtCam_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  GetWindowText(ds->hwnd, wbuf, 128); //��ð�ť�ؼ�������

	if(ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
//    GUI_DEBUG("ds->ID=%d,BST_PUSHED",ds->ID);
		SetBrushColor(hdc,MapRGB(hdc,105,105,105)); //�������ɫ(BrushColor��������Fill���͵Ļ�ͼ����)
		SetPenColor(hdc,MapRGB(hdc,105,105,105));        //���û���ɫ(PenColor��������Draw���͵Ļ�ͼ����)
		SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));      //��������ɫ
    
    DrawRoundRect(hdc, &rc, MIN(rc.w,rc.h)>>1);
    InflateRect(&rc, -1, -1);
    DrawRoundRect(hdc, &rc, MIN(rc.w,rc.h)>>1); 
    
    FillRoundRect(hdc, &rc, MIN(rc.w,rc.h)>>1);  
    DrawText(hdc, wbuf, -1, &rc, DT_VCENTER | DT_CENTER);//��������(���ж��뷽ʽ) 
    
	}
	else//��ť�ǵ���״̬
	{ 

		SetPenColor(hdc,MapRGB(hdc,250,250,250));
		SetTextColor(hdc, MapRGB(hdc, 250,250,250)); 
    DrawRoundRect(hdc, &rc, MIN(rc.w,rc.h)>>1);
    InflateRect(&rc, -1, -1);
    DrawRoundRect(hdc, &rc, MIN(rc.w,rc.h)>>1);

    DrawText(hdc, wbuf, -1, &rc, DT_VCENTER | DT_CENTER);//��������(���ж��뷽ʽ)    
	}
}
/*
 * @brief  �Զ��巵�ذ�ť
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void exit_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  if(ds->State & BST_PUSHED) //��ť�ǰ���״̬
	{

		SetTextColor(hdc, MapRGB(hdc, 105, 105, 105));      //��������ɫ
	}
	else//��ť�ǵ���״̬
	{ 

		SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
	}

	/* ʹ�ÿ���ͼ������ */
	SetFont(hdc, controlFont_32);

	GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������

	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER);//��������(���ж��뷽ʽ)
   rc.x = 25; 
  /* �ָ�Ĭ������ */
	SetFont(hdc, defaultFont);
  DrawText(hdc, L"����", -1, &rc, DT_VCENTER);
}
/*
 * @brief  ������Ļ
 * @param  NONE
 * @retval NONE
*/
//  rt_tick_t tick1 = 0;
//  rt_tick_t tick2 = 0;
//static void Update_Dialog()
//{


////  GUI_DEBUG("%d", tick);
//	while(1) //�߳��Ѵ�����
//	{

//		{
// 
////      tick1 = rt_tick_get();
//			GUI_SemWait(cam_sem, 0xFFFFFFFF);
////      tick2 = rt_tick_get();
////      GUI_DEBUG("%d",tick2-tick1);
//      InvalidateRect(Cam_hwnd,NULL,FALSE);
//	
//		}
//	}
////  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
//}
/**
  * @brief  �����Զ��Խ�����
  * @param  ��
  * @retval ��
  * @notes  
  */
static int thread=0;
TaskHandle_t h;
static void Set_AutoFocus()
{
	if(thread==0)
	{  
//      h=rt_thread_create("Set_AutoFocus",(void(*)(void*))Set_AutoFocus,NULL,1024*2,5,5);
    xTaskCreate((TaskFunction_t )(void(*)(void*))Set_AutoFocus,  /* ������ں��� */
                            (const char*    )"Set_AutoFocus",/* �������� */
                            (uint16_t       )8*1024/4,  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )NULL,/* ������ں������� */
                            (UBaseType_t    )5, /* ��������ȼ� */
                            (TaskHandle_t  )&h);/* ������ƿ�ָ�� */
//      rt_thread_startup(h);				
      thread =1;
      return;
	}
	while(thread==1) //�߳��Ѵ�����
	{
    GUI_SemWait(set_sem, 0xFFFFFFFF);
    if(focus_status != 1)
    {
      //��ͣ�Խ�
      OV5640_FOCUS_AD5820_Pause_Focus();

    }
    else
    {
      //�Զ��Խ�
      OV5640_FOCUS_AD5820_Constant_Focus();

    } 

    GUI_Yield();
	}
  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}
/*
 *������y0--��y0Ϊ��������룬h---Ҫ����Ŀؼ��߶�
 *���ӣ��ؼ���������
 */
int Set_VCENTER(int y0, int h)
{
  return y0-h/2;
}
/*============================================================================*/
/*
 * @brief  ��ʼ����������
 * @param  NONE
 * @retval NONE
*/
static void Camera_ReConfig(void)
{
  // cam_mode.frame_rate = FRAME_RATE_15FPS;	
	
	// //ISP����
	// cam_mode.cam_isp_sx = 0;
	// cam_mode.cam_isp_sy = 0;	
	
	// cam_mode.cam_isp_width = 1920;
	// cam_mode.cam_isp_height = 1080;
	
	// //�������
	// cam_mode.scaling = 1;     //ʹ���Զ�����
	// cam_mode.cam_out_sx = 16;	//ʹ���Զ����ź�һ�����ó�16����
	// cam_mode.cam_out_sy = 4;	  //ʹ���Զ����ź�һ�����ó�4����
	// cam_mode.cam_out_width = 800;
	// cam_mode.cam_out_height = 480;
	
	// //LCDλ��
	// cam_mode.lcd_sx = 0;
	// cam_mode.lcd_sy = 0;
	// cam_mode.lcd_scan = 5; //LCDɨ��ģʽ�����������ÿ���1��3��5��7ģʽ
	
	// //���¿ɸ����Լ�����Ҫ������������Χ���ṹ�����Ͷ���	
	// cam_mode.light_mode = 0;//�Զ�����ģʽ
	// cam_mode.saturation = 0;	
	// cam_mode.brightness = 0;
	// cam_mode.contrast = 0;
	// cam_mode.effect = 0;		//����ģʽ
	// cam_mode.exposure = 0;		

	// cam_mode.auto_focus = 1;
}
/*
 * @brief  ���÷ֱ����Ӵ���
*/
int cur_Resolution = eID_RB3;//��ǰ�ֱ���
static LRESULT	dlg_set_Resolution_WinProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE: //���ڴ���ʱ,���Զ���������Ϣ,��������һЩ��ʼ���Ĳ����򴴽��Ӵ���.
    {
      RECT rc;
      GetClientRect(hwnd, &rc);
      rc.x =5;
      rc.y =55;
      rc.w =200;
      rc.h =24;
      //��ѡһ����--���÷ֱ���
      CreateWindow(BUTTON,L"320*240",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB1,NULL,NULL);
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"480*272",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB2,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"800*480(Ĭ��)",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB3,NULL,NULL);   

      switch(cur_Resolution)
      {
        case eID_RB1:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB1),BM_SETSTATE,BST_CHECKED,0);
          break;
        }
        case eID_RB2:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB2),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB3:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB3),BM_SETSTATE,BST_CHECKED,0);
          break;
        }              
      }   
      //���ذ���
      CreateWindow(BUTTON, L"F", BS_FLAT | BS_NOTIFY|WS_TRANSPARENT|WS_OWNERDRAW |WS_VISIBLE,
      0, 0, 240, 50, hwnd, eID_BT1, NULL, NULL); 

      //��������
      SetWindowEraseEx(hwnd, cbErase, TRUE);
      break;
    }
    case WM_PAINT: //������Ҫ����ʱ�����Զ���������Ϣ.
    {
      PAINTSTRUCT ps;
      HDC hdc;
      RECT rc;
      hdc =BeginPaint(hwnd,&ps); //��ʼ��ͼ

      ////�û��Ļ�������...
      GetClientRect(hwnd, &rc);
      //�ϱ���Ŀ
      rc.h = 50;
      SetBrushColor(hdc,MapRGB(hdc,0,0,0));
      FillRect(hdc, &rc);
      GetClientRect(hwnd, &rc);
      SetBrushColor(hdc,MapRGB(hdc,105,105,105));
      
      rc.y = 50;
      rc.h = rc.h-50;
      FillRect(hdc, &rc);         
      SetTextColor(hdc, MapRGB(hdc,250,250,250));

      rc.x =100;
      rc.y =0;
      rc.w =200; 
      rc.h =50;

      DrawText(hdc,L"�ֱ���",-1,&rc,DT_CENTER|DT_VCENTER); 


      //TextOut(hdc,10,10,L"Hello",-1);

      EndPaint(hwnd,&ps); //������ͼ
      break;
    }
   
    case WM_DRAWITEM:
    {
      DRAWITEM_HDR *ds;
      ds = (DRAWITEM_HDR*)lParam;
      if (ds->ID == eID_SCROLLBAR)
      {
        Cam_scrollbar_ownerdraw(ds);
        return TRUE;
      }
      if (ds->ID == eID_BT1)
      {
        exit_owner_draw(ds);
        return TRUE;
      }
    } 
    case WM_NOTIFY:
    {
      u16 code,id;
      code =HIWORD(wParam); //���֪ͨ������.
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.
      if(id==eID_BT1 && code==BN_CLICKED)
      {
        PostCloseMessage(hwnd);
      }

      if(id >= eID_RB1 && id<= eID_RB3)
      {
        if(code == BN_CLICKED)
        { 
          cur_Resolution = id;
          switch(cur_Resolution)
          {
            case eID_RB1:
            {
              OV5640_Capture_Control(DISABLE);
              //�������
              cam_mode.scaling = 1;      //ʹ���Զ�����
              cam_mode.cam_out_sx = 16;	//ʹ���Զ����ź�һ�����ó�16����
              cam_mode.cam_out_sy = 4;	  //ʹ���Զ����ź�һ�����ó�4����
              cam_mode.cam_out_width = 320;
              cam_mode.cam_out_height = 240;

              //LCDλ��
              cam_mode.lcd_sx = 270;
              cam_mode.lcd_sy = 120;
              OV5640_OutSize_Set(cam_mode.scaling,
                       cam_mode.cam_out_sx,
                       cam_mode.cam_out_sy,
                       cam_mode.cam_out_width,
                       cam_mode.cam_out_height);

              OV5640_Capture_Control(ENABLE);
             
              state = 3;
              break;  
            }          
            case eID_RB2:
            {
              OV5640_Capture_Control(DISABLE);
              //�������
              cam_mode.scaling = 1;      //ʹ���Զ�����
              cam_mode.cam_out_sx = 16;	//ʹ���Զ����ź�һ�����ó�16����
              cam_mode.cam_out_sy = 4;	  //ʹ���Զ����ź�һ�����ó�4����
              cam_mode.cam_out_width = 480;
              cam_mode.cam_out_height = 272;

              //LCDλ��
              cam_mode.lcd_sx = 160;
              cam_mode.lcd_sy = 104;
              OV5640_OutSize_Set(cam_mode.scaling,
                       cam_mode.cam_out_sx,
                       cam_mode.cam_out_sy,
                       cam_mode.cam_out_width,
                       cam_mode.cam_out_height);

              OV5640_Capture_Control(ENABLE);

              state = 3;
              break;
            }
            case eID_RB3:
            {
              OV5640_Capture_Control(DISABLE);
              //�������
              cam_mode.scaling = 1;      //ʹ���Զ�����
              cam_mode.cam_out_sx = 16;	//ʹ���Զ����ź�һ�����ó�16����
              cam_mode.cam_out_sy = 4;	  //ʹ���Զ����ź�һ�����ó�4����
              cam_mode.cam_out_width = 800;
              cam_mode.cam_out_height = 480;

              //LCDλ��
              cam_mode.lcd_sx = 0;
              cam_mode.lcd_sy = 0;
              OV5640_OutSize_Set(cam_mode.scaling,
                       cam_mode.cam_out_sx,
                       cam_mode.cam_out_sy,
                       cam_mode.cam_out_width,
                       cam_mode.cam_out_height);


              OV5640_Capture_Control(ENABLE);
              
              state = 3;
              break;
            }
          }
        
        }
      }
      break;
    }
    case	WM_CTLCOLOR:
    {
      u16 id;
      id =LOWORD(wParam);
      CTLCOLOR *cr;
      cr =(CTLCOLOR*)lParam;
      if(id >=eID_RB1 && id <= eID_RB3)
      {
        cr->TextColor =RGB888(250,250,250);
        cr->BackColor =RGB888(105,105,105);
        cr->BorderColor =RGB888(50,50,50);
        cr->ForeColor =RGB888(105,105,105);
        return TRUE;            
      }

      return FALSE;

    }     
    case WM_CLOSE:
    {
      HWND wnd =GetDlgItem(SetWIN, eID_TB1);

      switch(cur_Resolution)
      {
        case eID_RB1:
          SetWindowText(wnd, L"320*240");
          break;
        case eID_RB2:
          SetWindowText(wnd, L"480*272");
          break;
        case eID_RB3:
          SetWindowText(wnd, L"800*480(Ĭ��)");
          break;
      }         

      RedrawWindow(SetWIN,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);

      DestroyWindow(hwnd);
      ShowWindow(SetWIN, SW_SHOW);
      return TRUE;  
    }

    default: //�û������ĵ���Ϣ,��ϵͳ����.
    {
      return DefWindowProc(hwnd,msg,wParam,lParam);
    }

  }
  return WM_NULL;

}

/*
 * @brief  ���ù���ģʽ
*/
int cur_LightMode = eID_RB4;//��ǰ�Ĺ���ģʽ
static LRESULT	dlg_set_LightMode_WinProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  switch(msg)
	{
		case WM_CREATE: //���ڴ���ʱ,���Զ���������Ϣ,��������һЩ��ʼ���Ĳ����򴴽��Ӵ���.
		{
      RECT rc;
      GetClientRect(hwnd, &rc);
      rc.x =5;
      rc.y =55;
      rc.w =200;
      rc.h =24;
      CreateWindow(BUTTON,L"�Զ�(Ĭ��)",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
                  rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB4,NULL,NULL);
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
                  rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB5,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
                  rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB6,NULL,NULL);          
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"�칫��",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
                  rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB7,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
                  rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB8,NULL,NULL);     

      CreateWindow(BUTTON, L"F", BS_FLAT | BS_NOTIFY|WS_TRANSPARENT|WS_OWNERDRAW |WS_VISIBLE,
                  0, 0, 240, 50, hwnd, eID_BT2, NULL, NULL); 
                  
      SetWindowEraseEx(hwnd, cbErase, TRUE);

      switch(cur_LightMode)
      {
        case eID_RB4:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB4),BM_SETSTATE,BST_CHECKED,0);
          break;
        }
        case eID_RB5:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB5),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB6:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB6),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB7:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB7),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB8:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB8),BM_SETSTATE,BST_CHECKED,0);
          break;
        }             
      }          
      break;
		}

		
 		case	WM_CTLCOLOR:
		{
      u16 id;
      id =LOWORD(wParam);
      CTLCOLOR *cr;
      cr =(CTLCOLOR*)lParam;
      if(id >=eID_RB4 && id <= eID_RB8)
      {
        cr->TextColor =RGB888(250,250,250);
        cr->BackColor =RGB888(200,220,200);
        cr->BorderColor =RGB888(50,50,50);
        cr->ForeColor =RGB888(105,105,105);
        return TRUE;            
      }

      return FALSE;
			
		}  
    case WM_NOTIFY:
    {
      u16 code,id;
      code =HIWORD(wParam); //���֪ͨ������.
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.
      if(id==eID_BT2 && code==BN_CLICKED)
      {
        PostCloseMessage(hwnd);
      }
      if(id >= eID_RB4 && id<= eID_RB8)
      {
        if(code == BN_CLICKED)
        { 
          cur_LightMode = id;
          switch(cur_LightMode)
          {
            case eID_RB4:
            {
              cam_mode.light_mode = 0;
              OV5640_LightMode(cam_mode.light_mode);
              break;  
            }            
            case eID_RB5:
            {
              cam_mode.light_mode = 1;
              OV5640_LightMode(cam_mode.light_mode);
              break;
            }
            case eID_RB6:
            {
              cam_mode.light_mode = 2;
              OV5640_LightMode(cam_mode.light_mode);
              break;
            }
            case eID_RB7:
            {
              cam_mode.light_mode = 3;
              OV5640_LightMode(cam_mode.light_mode);
              break;
            }
            case eID_RB8:
            {
              cam_mode.light_mode = 4;
              OV5640_LightMode(cam_mode.light_mode);
              break;
            }
          }   
        }
      }
      break;
    }     
    case WM_DRAWITEM:
    { 
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       if (ds->ID == eID_BT2)
       {
          exit_owner_draw(ds);
          return TRUE;
       }
    }  
    case WM_CLOSE:
    {
      switch(cur_LightMode)
      {
        case eID_RB4:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB2), L"�Զ�(Ĭ��)");
          break;
        }          
        case eID_RB5:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB2), L"����");
          break;
        }
        case eID_RB6:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB2), L"����");
          break;
        }
        case eID_RB7:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB2), L"�칫��");
          break;
        }
        case eID_RB8:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB2), L"����");
          break;
        }
      }         

      DestroyWindow(hwnd);
      ShowWindow(SetWIN, SW_SHOW);
      return TRUE; 
    } 
		case WM_PAINT: //������Ҫ����ʱ�����Զ���������Ϣ.
		{
      PAINTSTRUCT ps;
      HDC hdc;
      RECT rc;
      hdc =BeginPaint(hwnd,&ps); //��ʼ��ͼ

      GetClientRect(hwnd, &rc);

      rc.h = 50;
      SetBrushColor(hdc,MapRGB(hdc,0,0,0));
      FillRect(hdc, &rc);
      GetClientRect(hwnd, &rc);
      SetBrushColor(hdc,MapRGB(hdc,105,105,105));
      rc.y = 50;
      rc.h = rc.h-50;
      FillRect(hdc, &rc);         
      SetTextColor(hdc, MapRGB(hdc,250,250,250));

      rc.x =100;
      rc.y =0;
      rc.w =200; 
      rc.h =50;

      DrawText(hdc,L"����ģʽ",-1,&rc,DT_CENTER|DT_VCENTER); 

      EndPaint(hwnd,&ps); //������ͼ
      break; 
		}
		     
		default: //�û������ĵ���Ϣ,��ϵͳ����.
		{
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}

	}
	return WM_NULL;

}

/*
 * @brief  ��������Ч��
*/
int cur_SpecialEffects = eID_RB16;//��ǰ����Ч��
static LRESULT	dlg_set_SpecialEffects_WinProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE: //���ڴ���ʱ,���Զ���������Ϣ,��������һЩ��ʼ���Ĳ����򴴽��Ӵ���.
		{
      RECT rc;
      GetClientRect(hwnd, &rc);
      rc.x =5;
      rc.y =55;
      rc.w =200;
      rc.h =24;
      CreateWindow(BUTTON,L"��ɫ",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB9,NULL,NULL);
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"ůɫ",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB10,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"�ڰ�",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB11,NULL,NULL);          
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB12,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"��ɫ",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB13,NULL,NULL); 
      OffsetRect(&rc,0,rc.h+10);         
      CreateWindow(BUTTON,L"ƫ��",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB14,NULL,NULL);         
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB15,NULL,NULL);   
      OffsetRect(&rc,0,rc.h+10);
      CreateWindow(BUTTON,L"����(Ĭ��)",BS_RADIOBOX|WS_VISIBLE|WS_TRANSPARENT,
      rc.x,rc.y,rc.w,rc.h,hwnd,(1<<16)|eID_RB16,NULL,NULL); 
      CreateWindow(BUTTON, L"F", BS_FLAT | BS_NOTIFY|WS_TRANSPARENT|WS_OWNERDRAW |WS_VISIBLE,
      0, 0, 240, 50, hwnd, eID_BT3, NULL, NULL); 

      SetWindowEraseEx(hwnd, cbErase, TRUE);

      switch(cur_SpecialEffects)
      {
        case eID_RB9:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB9),BM_SETSTATE,BST_CHECKED,0);
          break;
        }
        case eID_RB10:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB10),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB11:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB11),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB12:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB12),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB13:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB13),BM_SETSTATE,BST_CHECKED,0);
          break;
        }     
        case eID_RB14:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB14),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB15:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB15),BM_SETSTATE,BST_CHECKED,0);
          break;
        }    
        case eID_RB16:
        {
          SendMessage(GetDlgItem(hwnd, eID_RB16),BM_SETSTATE,BST_CHECKED,0);
          break;
        }              
      }          
      break;
		}
		
    case WM_ERASEBKGND:
    {
      HDC hdc =(HDC)wParam;
      RECT rc =*(RECT*)lParam;
      
      SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
      FillRect(hdc, &rc);
      return TRUE;
    }
    case	WM_CTLCOLOR:
    {
      u16 id;
      id =LOWORD(wParam);
      CTLCOLOR *cr;
      cr =(CTLCOLOR*)lParam;
      if(id >=eID_RB9 && id <= eID_RB16)
      {
        cr->TextColor =RGB888(250,250,250);
        cr->BackColor =RGB888(200,220,200);
        cr->BorderColor =RGB888(50,50,50);
        cr->ForeColor =RGB888(105,105,105);
        return TRUE;            
      }
      return FALSE;			
		}  
    case WM_NOTIFY:
    {
      u16 code,id;
      code =HIWORD(wParam); //���֪ͨ������.
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.
      if(id==eID_BT3 && code==BN_CLICKED)
      {
        PostCloseMessage(hwnd);
      }
      if(id >= eID_RB9 && id<= eID_RB16)
      {
        if(code == BN_CLICKED)
        { 
          cur_SpecialEffects = id;
          switch(cur_SpecialEffects)
          {
            case eID_RB9:
            {
              cam_mode.effect = 1;
              OV5640_SpecialEffects(cam_mode.effect);
              break;    
            }            
            case eID_RB10:
            {
              cam_mode.effect = 2;
              OV5640_SpecialEffects(cam_mode.effect);
              break;
            }
            case eID_RB11:
            {
              cam_mode.effect = 3;
              OV5640_SpecialEffects(cam_mode.effect);               
              break;
            }
            case eID_RB12:
            {
              cam_mode.effect = 4;
              OV5640_SpecialEffects(cam_mode.effect);                
              break;
            }
            case eID_RB13:
            {
              cam_mode.effect = 5;
              OV5640_SpecialEffects(cam_mode.effect);                
              break;
            }
            case eID_RB14:
            {
              cam_mode.effect = 6;
              OV5640_SpecialEffects(cam_mode.effect);                
              break;
            }
            case eID_RB15:
            {
              cam_mode.effect = 7;
              OV5640_SpecialEffects(cam_mode.effect);                
              break;
            }
            case eID_RB16:
            {
              cam_mode.effect = 0;
              OV5640_SpecialEffects(cam_mode.effect);
              break;  
            }            
          } 
        }
      }
      break;
    }     
    case WM_DRAWITEM:
    { 
      DRAWITEM_HDR *ds;
      ds = (DRAWITEM_HDR*)lParam;
      if (ds->ID == eID_BT3)
      {
        exit_owner_draw(ds);
        return TRUE;
      }
    }  
    case WM_CLOSE:
    {
      switch(cur_SpecialEffects)
      {
        case eID_RB9:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"��ɫ");
          break;                 
        }
        case eID_RB10:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"ůɫ");
          break;
        }
        case eID_RB11:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"�ڰ�");
          break;
        }
        case eID_RB12:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"����");break;
        }
        case eID_RB13:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"��ɫ");
          break;
        }
        case eID_RB14:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"ƫ��");
          break;
        }
        case eID_RB15:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"����");
          break;
        }
        case eID_RB16:
        {
          SetWindowText(GetDlgItem(SetWIN, eID_TB3), L"����(Ĭ��)");
          break;   
        }        
      }         
      ShowWindow(SetWIN, SW_SHOW);
      DestroyWindow(hwnd);
      return TRUE; 
    }      

		case WM_PAINT: //������Ҫ����ʱ�����Զ���������Ϣ.
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			hdc =BeginPaint(hwnd,&ps); //��ʼ��ͼ
      GetClientRect(hwnd, &rc);

      rc.h = 50;
      SetBrushColor(hdc,MapRGB(hdc,0,0,0));
      FillRect(hdc, &rc);
      GetClientRect(hwnd, &rc);
      SetBrushColor(hdc,MapRGB(hdc,105,105,105));
      rc.y = 50;
      rc.h = rc.h-50;
      FillRect(hdc, &rc);         
      SetTextColor(hdc, MapRGB(hdc,250,250,250));

      rc.x =100;
      rc.y =0;
      rc.w =200; 
      rc.h =50;

      DrawText(hdc,L"����Ч��",-1,&rc,DT_CENTER|DT_VCENTER);

			EndPaint(hwnd,&ps); //������ͼ
      break;
		}
  
    default: //�û������ĵ���Ϣ,��ϵͳ����.
      return DefWindowProc(hwnd,msg,wParam,lParam);
  }
	return WM_NULL;

}
/*
 * @brief  �������ô��ڻص�����
*/
extern int updata;
static LRESULT	dlg_set_WinProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	RECT rc;
  static SCROLLINFO sif, sif1, sif2;
  RECT rc_first[2];
  RECT rc_second[2];
  RECT rc_third[2];
  //static int state = 0;
	switch(msg)
	{
		case WM_CREATE: //���ڴ���ʱ,���Զ���������Ϣ,��������һЩ��ʼ���Ĳ����򴴽��Ӵ���.
		{
			b_close =FALSE;
			rc.x =5;
			rc.y =50;
			rc.w =400;
			rc.h =50;
      //��rc�ȼ��ֳ����ݣ�����rc_first
      MakeMatrixRect(rc_first, &rc, 5, 0, 2, 1);
      CreateWindow(BUTTON,L"�Զ��Խ�",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
                   rc_first[0].x,rc_first[0].y,rc_first[0].w,rc_first[0].h,hwnd,eID_SET1,NULL,NULL); 
      rc_first[1].y = Set_VCENTER(rc_first[0].y+rc_first[0].h/2,30);
      CreateWindow(BUTTON,L" ",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
                   rc.w-70,rc_first[1].y,60,30,hwnd,eID_switch,NULL,NULL);   
            
			OffsetRect(&rc,0,rc.h);
      MakeMatrixRect(rc_second, &rc, 5, 0, 2, 1);
      CreateWindow(BUTTON,L"����",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
                   rc_second[0].x,rc_second[0].y,rc_second[0].w,rc_second[0].h,hwnd,eID_SET2,NULL,NULL);
      sif.cbSize = sizeof(sif);
      sif.fMask = SIF_ALL;
      sif.nMin = -2;
      sif.nMax = 2;
      sif.nValue = cam_mode.brightness;
      sif.TrackSize = 31;//����ֵ
      sif.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
      rc_second[1].y = Set_VCENTER(rc_second[0].y+rc_second[0].h/2,31);
      CreateWindow(SCROLLBAR, L"SCROLLBAR_liangdu", WS_OWNERDRAW|WS_VISIBLE, 
                   rc_second[1].x,rc_second[1].y,180,31, hwnd, eID_SCROLLBAR, NULL, NULL);                 
      SendMessage(GetDlgItem(hwnd, eID_SCROLLBAR), SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);      
 
 
			OffsetRect(&rc,0,rc.h);
      CreateWindow(BUTTON,L"���Ͷ�",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,eID_SET3,NULL,NULL);
      MakeMatrixRect(rc_third, &rc, 5, 0, 2, 1);

      sif1.cbSize = sizeof(sif1);
      sif1.fMask = SIF_ALL;
      sif1.nMin = -3;
      sif1.nMax = 3;
      sif1.nValue = cam_mode.saturation;
      sif1.TrackSize = 31;//����ֵ
      sif1.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
      rc_third[1].y = Set_VCENTER(rc_third[0].y+rc_third[0].h/2,31);
      CreateWindow(SCROLLBAR, L"SCROLLBAR_R", WS_OWNERDRAW|WS_VISIBLE, 
                   rc_third[1].x,rc_third[1].y,180,31, hwnd, eID_SCROLLBAR1, NULL, NULL);
      SendMessage(GetDlgItem(hwnd, eID_SCROLLBAR1), SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif1);

      OffsetRect(&rc,0,rc.h);
      CreateWindow(BUTTON,L"�Աȶ�",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,eID_SET4,NULL,NULL);

      MakeMatrixRect(rc_third, &rc, 5, 0, 2, 1);
      sif2.cbSize = sizeof(sif2);
      sif2.fMask = SIF_ALL;
      sif2.nMin = -3;
      sif2.nMax = 3;
      sif2.nValue = cam_mode.contrast;
      sif2.TrackSize = 31;//����ֵ
      sif2.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
      rc_third[1].y = Set_VCENTER(rc_third[0].y+rc_third[0].h/2,31);
      CreateWindow(SCROLLBAR, L"SCROLLBAR_R", WS_OWNERDRAW|WS_VISIBLE, 
                   rc_third[1].x,rc_third[1].y, 180, 31, hwnd, eID_SCROLLBAR2, NULL, NULL);
      SendMessage(GetDlgItem(hwnd, eID_SCROLLBAR2), SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif2);


      OffsetRect(&rc,0,rc.h);
      CreateWindow(BUTTON,L"�ֱ���",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,eID_SET5,NULL,NULL);
      MakeMatrixRect(rc_third, &rc, 0, 0, 2, 1);
      rc_third[1].y = Set_VCENTER(rc_third[0].y+rc_third[0].h/2,30);
      CreateWindow(BUTTON,L"800*480(Ĭ��)",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
                   rc_third[1].x,rc_third[1].y,200,30,hwnd,eID_TB1,NULL,NULL); 

      switch(cur_Resolution)
      {
        case eID_RB1:
          SetWindowText(GetDlgItem(hwnd, eID_TB1), L"320*240");break;                 
        case eID_RB2:
          SetWindowText(GetDlgItem(hwnd, eID_TB1), L"480*272");break;
        case eID_RB3:
          SetWindowText(GetDlgItem(hwnd, eID_TB1), L"800*480(Ĭ��)");break;
      }          
      OffsetRect(&rc,0,rc.h);
      CreateWindow(BUTTON,L"����ģʽ",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,eID_SET6,NULL,NULL);
      MakeMatrixRect(rc_third, &rc, 0, 0, 2, 1);
      rc_third[1].y = Set_VCENTER(rc_third[0].y+rc_third[0].h/2,40);
      CreateWindow(BUTTON,L"�Զ�(Ĭ��)",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
      rc_third[1].x,rc_third[1].y,200,40,hwnd,eID_TB2,NULL,NULL);                      

      switch(cur_LightMode)
      {
        case eID_RB4:
          SetWindowText(GetDlgItem(hwnd, eID_TB2), L"�Զ�(Ĭ��)");break;                 
        case eID_RB5:
          SetWindowText(GetDlgItem(hwnd, eID_TB2), L"����");break;
        case eID_RB6:
          SetWindowText(GetDlgItem(hwnd, eID_TB2), L"����");break;
        case eID_RB7:
          SetWindowText(GetDlgItem(hwnd, eID_TB2), L"�칫��");break;
        case eID_RB8:
          SetWindowText(GetDlgItem(hwnd, eID_TB2), L"����");break;
      }  
      OffsetRect(&rc,0,rc.h);
      CreateWindow(BUTTON,L"����Ч��",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.x,rc.y,rc.w,rc.h,hwnd,eID_SET7,NULL,NULL);

      MakeMatrixRect(rc_third, &rc, 0, 0, 2, 1);
      rc_third[1].y = Set_VCENTER(rc_third[0].y+rc_third[0].h/2,30);         
      CreateWindow(BUTTON,L"����(Ĭ��)",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,
      rc_third[1].x,rc_third[1].y,200,30,hwnd,eID_TB3,NULL,NULL);                      
      switch(cur_SpecialEffects)
      {
        case eID_RB9:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"��ɫ");break;                 
        case eID_RB10:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"ůɫ");break;
        case eID_RB11:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"�ڰ�");break;
        case eID_RB12:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"����");break;
        case eID_RB13:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"��ɫ");break;
        case eID_RB14:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"ƫ��");break;
        case eID_RB15:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"����");break;
        case eID_RB16:
          SetWindowText(GetDlgItem(hwnd, eID_TB3), L"����(Ĭ��)");break;            
      }           
      GetClientRect(hwnd, &rc);
      hdc_bk = CreateMemoryDC(SURF_SCREEN, rc.w, rc.h);
      CreateWindow(BUTTON, L"F", BS_FLAT | BS_NOTIFY|WS_TRANSPARENT|WS_OWNERDRAW |WS_VISIBLE,
      0, 0, 240, 50, hwnd, ID_EXIT, NULL, NULL);   
      SetTimer(hwnd,2,20,TMR_START,NULL);
      break;
    }

		case WM_TIMER://ʵ�ִ��������Ч��
		{
			switch(wParam)
			{
				case 1:
				{
          break;
				}
				case 2:
				{
					if(GetKeyState(VK_LBUTTON)!=0)
					{
						break;
					}
					GetWindowRect(hwnd,&rc);

					if(b_close==FALSE)
					{
						if(rc.y < win_rc.y )
						{
							if((win_rc.y-rc.y)>50)
							{
								rc.y +=30;
							}
							if((win_rc.y-rc.y)>30)
							{
								rc.y +=20;
							}
							else
							{
								rc.y +=4;
							}
							ScreenToClient(GetParent(hwnd),(POINT*)&rc.x,1);
                     
							MoveWindow(hwnd,rc.x,rc.y,rc.w,rc.h,TRUE);
						}
					}
					else
					{
						if(rc.y > -(rc.h))
						{
							rc.y -= 40;

							ScreenToClient(GetParent(hwnd),(POINT*)&rc.x,1);
                     
							MoveWindow(hwnd,rc.x,rc.y,rc.w,rc.h,TRUE);
						}
						else
						{
							PostCloseMessage(hwnd);
						}
					}


				}
				break;
			}
      break;
		}
		
		
		case WM_NOTIFY: //WM_NOTIFY��Ϣ:wParam��16λΪ���͸���Ϣ�Ŀؼ�ID,��16λΪ֪ͨ��;lParamָ����һ��NMHDR�ṹ��.
		{
			u16 code,id;
      NMHDR *nr;  
      u16 ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID. 
      nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.
			code =HIWORD(wParam); //���֪ͨ������.
			id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.
      NM_SCROLLBAR *sb_nr;
      sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
      switch (nr->code)
      {
        case SBN_THUMBTRACK: //R�����ƶ�
        {
          switch(ctr_id)
          {
            case eID_SCROLLBAR://����
            {
              cam_mode.brightness = sb_nr->nTrackValue; //�õ���ǰ������ֵ
              OV5640_BrightnessConfig(cam_mode.brightness);
              SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, cam_mode.brightness); 
              break;
            }
            case eID_SCROLLBAR1://���Ͷ�
            {
              cam_mode.saturation = sb_nr->nTrackValue; //�õ���ǰ������ֵ
              OV5640_Color_Saturation(cam_mode.saturation);
              SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, cam_mode.saturation); 
              break;
            }
            case eID_SCROLLBAR2://�Աȶ�
            {
              cam_mode.contrast = sb_nr->nTrackValue; //�õ���ǰ������ֵ
              OV5640_ContrastConfig(cam_mode.contrast);
              SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, cam_mode.contrast);                      
              break;
            }

          }
          break;
        } 
      
      }
			if((id==eID_Setting1|| id == eID_TB1)&& code==BN_CLICKED)
			{
        ShowWindow(hwnd, SW_HIDE); //�������ô���
        WNDCLASS wcex;
        
        GetClientRect(Cam_hwnd,&rc);
        rc.x = rc.x+(rc.w-win_rc.w)/2;
        rc.w =400;
        rc.h =155;

        wcex.Tag	 		= WNDCLASS_TAG;
        wcex.Style			= CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc	= (WNDPROC)dlg_set_Resolution_WinProc;
        wcex.cbClsExtra		= 0;
        wcex.cbWndExtra		= 0;
        wcex.hInstance		= NULL;
        wcex.hIcon			= NULL;
        wcex.hCursor		= NULL;
        
        CreateWindowEx(WS_EX_FRAMEBUFFER,
                        &wcex,L"Set_1_xxx",
                        WS_OVERLAPPED|WS_CLIPCHILDREN|WS_VISIBLE,
                        rc.x,rc.y,rc.w,rc.h,
                        Cam_hwnd,0,NULL,NULL);
                         
			}

			if((id==eID_Setting2|| id == eID_TB2) && code==BN_CLICKED)
			{
        WNDCLASS wcex;
        ShowWindow(hwnd, SW_HIDE);//�������ô���
        GetClientRect(Cam_hwnd,&rc);
        
        rc.x = rc.x+(rc.w-win_rc.w)/2;
        rc.w =400;
        rc.h =225;
        wcex.Tag	 		= WNDCLASS_TAG;
        wcex.Style			= CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc	= (WNDPROC)dlg_set_LightMode_WinProc;
        wcex.cbClsExtra		= 0;
        wcex.cbWndExtra		= 0;
        wcex.hInstance		= NULL;
        wcex.hIcon			= NULL;
        wcex.hCursor		= NULL;


        CreateWindowEx(WS_EX_FRAMEBUFFER,
                        &wcex,L"Set_2_xxx",
                        WS_OVERLAPPED|WS_CLIPCHILDREN|WS_VISIBLE,
                        rc.x,rc.y,rc.w,rc.h,
                        Cam_hwnd,0,NULL,NULL);

			}
			if((id==eID_Setting3|| id == eID_TB3) && code==BN_CLICKED)
			{
        WNDCLASS wcex;
        ShowWindow(hwnd, SW_HIDE);
        GetClientRect(Cam_hwnd,&rc);

        rc.x = rc.x+(rc.w-win_rc.w)/2;
 
        rc.w =400;
        rc.h =325;
        wcex.Tag	 		= WNDCLASS_TAG;
        wcex.Style			= CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc	= (WNDPROC)dlg_set_SpecialEffects_WinProc;
        wcex.cbClsExtra		= 0;
        wcex.cbWndExtra		= 0;
        wcex.hInstance		= NULL;
        wcex.hIcon			= NULL;
        wcex.hCursor		= NULL;


        CreateWindowEx(WS_EX_FRAMEBUFFER,
                      &wcex,L"Set_3_xxx",
                      WS_OVERLAPPED|WS_CLIPCHILDREN|WS_VISIBLE,
                      rc.x,rc.y,rc.w,rc.h,
                      Cam_hwnd,0,NULL,NULL);

			}
			if(id==ID_EXIT && code==BN_CLICKED) // ��ť����������.
			{
				PostCloseMessage(hwnd); //ʹ����WM_CLOSE��Ϣ�رմ���.
			}
         
			if (id == eID_switch && code == BN_CLICKED)//�л��Զ��Խ�״̬
			{
				focus_status = ~focus_status;
        GUI_SemPost(set_sem);
			}               
      break;		
    }



		case WM_PAINT: //������Ҫ����ʱ�����Զ���������Ϣ.
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			hdc =BeginPaint(hwnd,&ps); //��ʼ��ͼ
      GetClientRect(hwnd, &rc);

      rc.h = 50;
      SetBrushColor(hdc_bk,MapRGB(hdc_bk,0,0,0));
      FillRect(hdc_bk, &rc);
      GetClientRect(hwnd, &rc);
      SetBrushColor(hdc_bk,MapRGB(hdc_bk,105,105,105));
      rc.y = 50;
      rc.h = rc.h-50;
      FillRect(hdc_bk, &rc);         
      SetTextColor(hdc_bk, MapRGB(hdc_bk,250,250,250));

      rc.x =100;
      rc.y =0;
      rc.w =200; 
      rc.h =50;

      DrawText(hdc_bk,L"��������",-1,&rc,DT_CENTER|DT_VCENTER); 
      SetPenColor(hdc_bk, MapRGB(hdc_bk, 245,245,245));
      GetClientRect(hwnd, &rc);
      //�����
      HLine(hdc_bk, 0, 50, 400);
      HLine(hdc_bk, 0, 100, 400);
      HLine(hdc_bk, 0, 150, 400);
      HLine(hdc_bk, 0, 200, 400);
      HLine(hdc_bk, 0, 250, 400);
      HLine(hdc_bk, 0, 300, 400);
      HLine(hdc_bk, 0, 350, 400);
      BitBlt(hdc, 0,0,rc.w, rc.h, hdc_bk,0,0,SRCCOPY);        
      EndPaint(hwnd,&ps); //������ͼ
      break;
		}
		case WM_CTLCOLOR:
		{
			u16 id;
			id =LOWORD(wParam);
      CTLCOLOR *cr;
      cr =(CTLCOLOR*)lParam;
      if((id >=eID_SET1 && id <= eID_SET7 )|| (id >= eID_TB1 && id <= eID_TB3) )
      {
        cr->TextColor =RGB888(255,255,255);//������ɫ��RGB888��ɫ��ʽ)
        cr->BackColor =RGB888(0,0,0);//������ɫ��RGB888��ɫ��ʽ)
        cr->BorderColor =RGB888(0,0,0);//�߿���ɫ��RGB888��ɫ��ʽ)
        return TRUE;            
      }    
			return FALSE;	
		}	

		case WM_DRAWITEM:
		{
      DRAWITEM_HDR *ds;
      ds = (DRAWITEM_HDR*)lParam;
      if (ds->ID == eID_switch)
      {
        Checkbox_owner_draw(ds); //ִ���Ի��ư�ť
      }
      if (ds->ID == eID_SCROLLBAR || ds->ID == eID_SCROLLBAR1 || ds->ID == eID_SCROLLBAR2)
      {
        Cam_scrollbar_ownerdraw(ds);
        return TRUE;
      }         
      if ((ds->ID >= eID_SET1 && ds->ID <= eID_SET7) || (ds->ID >= eID_TB1 && ds->ID <= eID_TB3)||
          (ds->ID >= eID_Setting1 && ds->ID <= eID_Setting3))
      {
        Button_owner_draw(ds); //ִ���Ի��ư�ť
      }
      if(ds->ID == ID_EXIT)  
      {
        exit_owner_draw(ds);
      }

      return TRUE;
		}
		case WM_CLOSE:
		{
			b_close=TRUE;
      
			GetWindowRect(hwnd,&rc);

			if(rc.y <= -(rc.h))
			{
				DestroyWindow(hwnd);
				return TRUE;
			}
			return FALSE;
		}
    case WM_DESTROY:
    {
      DeleteDC(hdc_bk);
      PostQuitMessage(hwnd);
      break;
    }
		default: //�û������ĵ���Ϣ,��ϵͳ����.
		{
			return DefWindowProc(hwnd,msg,wParam,lParam);
		}

	}
	return WM_NULL;
}
/*
 * @brief  ����ͷ���ڻص�����
*/
extern int SelectDialogBox(HWND hwndParent, RECT *rc, const WCHAR *pText, const WCHAR *pCaption, const MSGBOX_OPTIONS *ops);
static LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static uint8_t OV5640_State = 0;    // 0:���Լ�⵽����ͷ
  switch(msg)
  {
    case WM_CREATE:
    {
      /* ��ʼ������ͷGPIO��IIC */
      OV5640_HW_Init();  
      /* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
      OV5640_ReadID(&OV5640_Camera_ID);

      if(OV5640_Camera_ID.PIDH  == 0x56)
      {
        GUI_DEBUG("OV5640 ID:%x %x",OV5640_Camera_ID.PIDH ,OV5640_Camera_ID.PIDL);
      }
      else
      {
        SetTimer(hwnd, 3, 3, TMR_START | TMR_SINGLE, NULL);      // ��ʼ������������ʾ
        break;
      }
//      cam_sem = GUI_SemCreate(0,1);//ͬ������ͷͼ��
//      set_sem = GUI_SemCreate(1,1);//�Զ��Խ��ź�
//      //�����Զ��Խ��߳�
////      h_autofocus=rt_thread_create("Update_Dialog",(void(*)(void*))Update_Dialog,NULL,4096,4,5);
//      xTaskCreate((TaskFunction_t )(void(*)(void*))Update_Dialog,  /* ������ں��� */
//                            (const char*    )"Update_Dialog",/* �������� */
//                            (uint16_t       )4*1024/4,  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
//                            (void*          )NULL,/* ������ں������� */
//                            (UBaseType_t    )5, /* ��������ȼ� */
//                            (TaskHandle_t  )&h_autofocus);/* ������ƿ�ָ�� */
////      rt_thread_startup(h_autofocus);
//      //Ĭ�Ͽ����Զ��Խ�
//      Set_AutoFocus();
      
      //ͼ�񻺳���	
//      bits = (U16 *)GUI_VMEM_Alloc(2*800*480); 
      
      SetTimer(hwnd,1,1000,TMR_START,NULL);  
//      SetTimer(hwnd,2,1,TMR_START,NULL);   
//      GetClientRect(hwnd, &rc);
//      //���ð���
//      CreateWindow(BUTTON,L"��������",WS_OWNERDRAW|WS_TRANSPARENT,rc.w-135,419,120,40,hwnd,eID_SET,NULL,NULL);
//      //�˳�����
//      CreateWindow(BUTTON, L"O",WS_OWNERDRAW|WS_TRANSPARENT,730, 0, 70, 70, hwnd, ID_EXIT, NULL, NULL); 
//      //֡��
//      CreateWindow(BUTTON,L" ",WS_OWNERDRAW|WS_TRANSPARENT|WS_VISIBLE,rc.w-600,400,400,72,hwnd,ID_FPS,NULL,NULL);
      break;  
    }
    case WM_DRAWITEM:
    {
      DRAWITEM_HDR *ds;
      ds = (DRAWITEM_HDR*)lParam;
      switch(ds->ID)
      {
        case eID_SET:
        {
          BtCam_owner_draw(ds); //ִ���Ի��ư�ť
          return TRUE;          
        }
        case ID_EXIT:
        {
          home_owner_draw(ds); 
          return TRUE;          
        }
        case ID_FPS:
        {
          button_owner_draw(ds);
          return TRUE;          
        }
        
      }
    }
    case WM_LBUTTONDOWN://�����Ļ���رմ���
    {

      PostCloseMessage(hwnd);
  

      break;
    }
    case WM_TIMER://����ͷ״̬��
    {
      u16 id;
      static int i = 0;
      
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.     
      if(id == 2)
      {
        i++;
        if(updata==1)
        {
          GUI_DEBUG("%d", i);
          i = 0;
          updata = 0;
          InvalidateRect(Cam_hwnd,NULL,FALSE);
        }
      }
      else if(id == 1)
      {
        switch(state)
        {
          case 0:
          {
            OV5640_Init();  
            OV5640_RGB565Config();
            OV5640_USER_Config();
            OV5640_FOCUS_AD5820_Init();

            if(cam_mode.auto_focus ==1)
            {
              OV5640_FOCUS_AD5820_Constant_Focus();
              focus_status = 1;
            }
            //ʹ��DCMI�ɼ�����
            DCMI_Cmd(ENABLE); 
            DCMI_CaptureCmd(ENABLE); 

            state = 1;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
          }
          case 1:
          {
            ShowWindow(GetDlgItem(hwnd, eID_SET), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_EXIT), SW_SHOW);
            state=2;
            break;
          }
          case 2:
          {
            update_flag = 1;
            break;
          }
        }
      }
      else if(id == 3)
      {
        RECT RC;
        MSGBOX_OPTIONS ops;
        const WCHAR *btn[] = { L"ȷ��",L"ȡ��" };      //�Ի����ڰ�ť������

        ops.Flag = MB_ICONERROR;
        ops.pButtonText = btn;
        ops.ButtonCount = 2;
        RC.w = 180;
        RC.h = 120;
        RC.x = (GUI_XSIZE - RC.w) >> 1;
        RC.y = (GUI_YSIZE - RC.h) >> 1;
        SelectDialogBox(hwnd, &RC, L"û�м�⵽OV6540ģ��\n�����¼�����ӡ�", L"����", &ops);    // ��ʾ������ʾ��
        PostCloseMessage(hwnd);
        break; 
      }
      break;
    }
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      SURFACE *pSurf;
      HDC hdc_mem;
      HDC hdc;
      RECT rc;
//      static int switch_res = 0;
      static int old_fps = 0;
      WCHAR wbuf[128];
      hdc = BeginPaint(hwnd,&ps);
      GetClientRect(hwnd,&rc);
      if(state==0)
      {
        SetTextColor(hdc,MapRGB(hdc,250,250,250));
        SetBrushColor(hdc,MapRGB(hdc,0,0,0));
        SetPenColor(hdc,MapRGB(hdc,250,0,0));
        FillRect(hdc, &rc);
        DrawText(hdc,L"���ڳ�ʼ������ͷ\r\n\n��ȴ�...",-1,&rc,DT_VCENTER|DT_CENTER|DT_BKGND);

      }     
      else
      {
        SetBrushColor(hdc,MapRGB(hdc,0,0,0));
        FillRect(hdc, &rc);
      }


      EndPaint(hwnd,&ps);
      break;
    }

    case WM_DESTROY:
    {
      state = 0;
      thread=0;//��������ѭ��
      OV5640_Reset();//��λ����ͷ
      OV5640_Capture_Control(DISABLE);//�ر�����ͷ�ɼ�ͼ��
      DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,DISABLE); //�ر�DMA�ж�
      DCMI_Cmd(DISABLE); //DCMIʧ��
      DCMI_CaptureCmd(DISABLE); 
      
      if (!OV5640_State)
      {
//        GUI_Thread_Delete(h_autofocus);//ɾ���Զ��Խ��߳�
//        GUI_Thread_Delete(h);//ɾ�����´����߳�
  //      GUI_VMEM_Free(bits);//�ͷ�ͼ�λ�����
      }
//      GUI_VMEM_Free(cam_buff1);
//      GUI_VMEM_Free(cam_buff0);
      //��λ����ͷ���ò���
      cur_Resolution = eID_RB3;
      cur_LightMode = eID_RB4;
      cur_SpecialEffects = eID_RB16;
      Camera_ReConfig();
      cur_index = 0;
      LCD_Init((uint32_t)LCD_FRAME_BUFFER, 0, LTDC_Pixelformat_RGB565);
      //LCD_LayerCamInit((uint32_t)LCD_FRAME_BUFFER, GUI_XSIZE, GUI_YSIZE);
      GUI_msleep(40);
      return PostQuitMessage(hwnd);	
    }    
    case WM_NOTIFY: //WM_NOTIFY��Ϣ:wParam��16λΪ���͸���Ϣ�Ŀؼ�ID,��16λΪ֪ͨ��;lParamָ����һ��NMHDR�ṹ��.
    {
      u16 code,id;
      static int flag = 0;//���ô����Ƿ񵯳�
      code =HIWORD(wParam); //���֪ͨ������.
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.
      if(flag == 0)//���ô���δ���ڣ��򴴽�
      {
        if(id==eID_SET && code==BN_CLICKED)
        {
          flag = 1;
          WNDCLASS wcex;


          wcex.Tag	 		= WNDCLASS_TAG;
          wcex.Style			= CS_HREDRAW | CS_VREDRAW;
          wcex.lpfnWndProc	= (WNDPROC)dlg_set_WinProc;
          wcex.cbClsExtra		= 0;
          wcex.cbWndExtra		= 0;
          wcex.hInstance		= NULL;
          wcex.hIcon			= NULL;
          wcex.hCursor		= NULL;

          if(1)
          {
            RECT rc;

            GetClientRect(hwnd,&rc);
           

            win_rc.w =400;
            win_rc.h =400;

            win_rc.x = rc.x+(rc.w-win_rc.w)/2;
            win_rc.y = rc.y;//rc.y+(rc.h>>2);

            SetWIN = CreateWindowEx(
            NULL,
            &wcex,L"Set",
            WS_OVERLAPPED|WS_CLIPCHILDREN|WS_VISIBLE,

            win_rc.x,-win_rc.h-4,win_rc.w,win_rc.h,
            hwnd,0,NULL,NULL);
          }

        }
      }
      else//���ô����Ѿ����ڣ��ٴε�����ð�ť����رմ���
      {
        flag = 0;
        PostCloseMessage(SetWIN);

      }

      if(id==ID_EXIT && code==BN_CLICKED)//�˳�����
      {
        PostCloseMessage(hwnd);
      }
      break;  
    }
    case WM_Test:
    {
      InvalidateRect(hwnd,NULL,FALSE);
    }
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return WM_NULL;
}


void	GUI_Camera_DIALOG(void)
{	
	WNDCLASS	wcex;
	MSG msg;

   g_dma2d_en = TRUE;
	wcex.Tag = WNDCLASS_TAG;  
  
  
//  cam_buff0 = (uint16_t *)GUI_VMEM_Alloc(LCD_XSIZE*LCD_YSIZE*2);
//  cam_buff1 = (uint16_t *)GUI_VMEM_Alloc(LCD_XSIZE*LCD_YSIZE*2);

  
  
	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinProc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);

	//����������
	Cam_hwnd = CreateWindowEx(WS_EX_NOFOCUS,
                                    &wcex,
                                    L"GUI_Camera_Dialog",
                                    WS_VISIBLE|WS_CLIPCHILDREN|WS_OVERLAPPED,
                                    0, 0, GUI_XSIZE, GUI_YSIZE,
									NULL, NULL, NULL, NULL);

	//��ʾ������
	ShowWindow(Cam_hwnd, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, Cam_hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
  }


}
