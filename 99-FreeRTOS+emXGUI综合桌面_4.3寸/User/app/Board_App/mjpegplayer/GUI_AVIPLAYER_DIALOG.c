#include "emXGUI.h"
#include "x_libc.h"
#include <string.h>
#include "GUI_AVIList_DIALOG.h"
#include "GUI_AVIPLAYER_DIALOG.h"
#include "./Bsp/wm8978/bsp_wm8978.h" 
#include "./mjpegplayer/Backend_vidoplayer.h"
#include "emXGUI_JPEG.h"
#include "GUI_AppDef.h"

#define AVI_Player_48 "Music_Player_48_48.xft"
#define AVI_Player_64 "Music_Player_64_64.xft"
#define AVI_Player_72 "Music_Player_72_72.xft"

// #define ID_EXIT       0x3000
 
void	GUI_AVIList_DIALOG(void);
static SCROLLINFO sif;/*���ö����������Ĳ���*/
static SCROLLINFO sif_horn;/*���������������Ĳ���*/
int avi_chl = 0;
static COLORREF color_bg;//͸���ؼ��ı�����ɫ
static HDC hdc_bk;//����ͼ��
static int power=20;//����ֵ
static int power_horn=40;//����ֵ
int showmenu_flag = 0;//��ʾ�˵���
HDC hdc_avi_play;
extern uint8_t avi_file_num;
extern int Play_index;
extern uint8_t  file_nums;
extern int sw_flag;//�л���־
extern char avi_playlist[FILE_MAX_NUM][FILE_NAME_LEN] __EXRAM;//����List
extern BOOL Player_Init(void);
//HFONT AVI_Player_hFont48=NULL;
//HFONT AVI_Player_hFont64  =NULL;
//HFONT AVI_Player_hFont72  =NULL;
uint8_t hide_flag = 0;

int LIST_STATE = 0;
//ͼ���������
static icon_S avi_icon[] = {
   {"yinliang",         {6, 234,35,35},      FALSE},
   {"bofangliebiao",    {440,235,35,35},      FALSE},
   {"back",             {185, 242, 24, 24},      FALSE},
   {"bofang",           {224, 237, 35, 35},      FALSE},
   {"next",             {271, 242, 24, 24},      FALSE},
   {"fenbianlv",        {0,40,380,40},   FALSE},
   {"zanting/bofang",   {300, 140, 200, 200}, FALSE},
   {"xiayishou",        {600, 200, 80, 80},   FALSE},    
   {"mini_next",        {580, 4, 80, 80},     FALSE},
   {"mini_Stop",        {652, 4, 80, 80},     FALSE},
   {"mini_back",        {724, 3, 80, 80},     FALSE},  
   {"�ϱ���",           {0 ,0, 800, 80},     FALSE},
   {"�±���",           {0 ,400, 800, 80},     FALSE}, 
   {"bugle",            {413, 237, 35, 35},      FALSE},  
};

/****************************�ؼ��ػ溯��***********************/
static void AVI_Button_OwnerDraw(DRAWITEM_HDR *ds)
{
  if(LIST_STATE == 1)
    return;
   HDC hdc; //�ؼ�����HDC
   HWND hwnd; //�ؼ���� 
   RECT rc_cli;//�ؼ���λ�ô�С����
   WCHAR wbuf[128];
   hwnd = ds->hwnd;
	 hdc = ds->hDC; 
   //��ȡ�ؼ���λ�ô�С��Ϣ
   GetClientRect(hwnd, &rc_cli);
   
	 GetWindowText(hwnd,wbuf,128); //��ð�ť�ؼ�������  
  
   // GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc_cli);//�õ��ؼ���λ��
   // WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   // BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_avi_play, rc_tmp.x, rc_tmp.y, SRCCOPY);
   SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
   FillRect(hdc, &rc_cli);

   //���ð�������ɫ
   SetTextColor(hdc, MapRGB(hdc, 250,250,250));
   if(ds->ID == eID_TB2)
    DrawText(hdc, wbuf,-1,&rc_cli,DT_VCENTER|DT_RIGHT);
   else if(ds->ID == eID_TB3)
    DrawText(hdc, wbuf,-1,&rc_cli,DT_VCENTER|DT_LEFT);
   else
    DrawText(hdc, wbuf,-1,&rc_cli,DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
   

}
/**
  * @brief  button_owner_draw ��ť�ؼ����ػ���
  * @param  ds:DRAWITEM_HDR�ṹ��
  * @retval NULL
  */
static void button_owner_draw(DRAWITEM_HDR *ds)
{
   HDC hdc; //�ؼ�����HDC
   // HDC hdc_mem;//�ڴ�HDC����Ϊ������
   HWND hwnd; //�ؼ���� 
   RECT rc_cli;//�ؼ���λ�ô�С����
   WCHAR wbuf[128];
	hwnd = ds->hwnd;
	hdc = ds->hDC; 
   //��ȡ�ؼ���λ�ô�С��Ϣ
   GetClientRect(hwnd, &rc_cli);
   //��������㣬��ʽΪSURF_ARGB4444
   // hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);
   
	GetWindowText(ds->hwnd,wbuf,128); //��ð�ť�ؼ�������  
   // GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc_cli);//�õ��ؼ���λ��
   // WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   // BitBlt(hdc_mem, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_avi_play, rc_tmp.x, rc_tmp.y, SRCCOPY);
   
  SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
  FillRect(hdc, &rc_cli);
   //���ð�������ɫ
   SetTextColor(hdc, MapARGB(hdc, 250,250,250,250));
   if((ds->ID == eID_BUTTON_Back || ds->ID == eID_BUTTON_Next)&& ds->State & BST_PUSHED)
      SetTextColor(hdc, MapARGB(hdc, 250,105,105,105));
   if(ds->ID == eID_BUTTON_Back || ds->ID == eID_BUTTON_Next)
   {
      SetFont(hdc, controlFont_24);

   }
   else if(ds->ID == eID_BUTTON_Play || ds->ID == eID_BUTTON_Play)
   {
      SetFont(hdc, controlFont_32);
   }
   else
   {
      //���ð�ť����
      SetFont(hdc, controlFont_32);
   }
 
   DrawText(hdc, wbuf,-1,&rc_cli,DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
   
   // BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
   
   //StretchBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_mem, 0, 0, rc_cli.w, rc_cli.h, SRCCOPY);
   
   // DeleteDC(hdc_mem);  
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
	GetClientRect(hwnd, &rc);
	/* ���� */
	SetBrushColor(hdc, back_c);
	FillRect(hdc, &rc);
   // GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   // GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
   // WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   // BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_avi_play, rc_tmp.x, rc_tmp.y, SRCCOPY);
  
  
   rc_scrollbar.x = rc.x;
   rc_scrollbar.y = rc.h/2;
   rc_scrollbar.w = rc.w;
   rc_scrollbar.h = 2;
   
	SetBrushColor(hdc, MapRGB888(hdc, Page_c));
	FillRect(hdc, &rc_scrollbar);

	/* ���� */
	SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);

	SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
	//rc.y += (rc.h >> 2) >> 1;
	//rc.h -= (rc.h >> 2);
	/* �߿� */
	//FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
	FillCircle(hdc, rc.x + rc.h / 2, rc.y + rc.h / 2, rc.h / 2);
   InflateRect(&rc, -2, -2);

	SetBrushColor(hdc, MapRGB888(hdc, fore_c));
	FillCircle(hdc, rc.x + rc.h / 2, rc.y + rc.h / 2, rc.h / 2);
   //FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
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
	HDC hdc_mem1;
	RECT rc;
	RECT rc_cli;
	//	int i;

	hwnd = ds->hwnd;
	hdc = ds->hDC;
	GetClientRect(hwnd, &rc_cli);

	hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);
	hdc_mem1 = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);   
         
   	
	//���ư�ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem1, RGB888(0, 0, 0), RGB888( 250, 250, 250), RGB888( 255, 255, 255));
	//������ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem, RGB888(0, 0, 0), RGB888( 250, 0, 0), RGB888( 250, 0, 0));
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
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem, rc.x, 0, SRCCOPY);
	}
	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem1);
	DeleteDC(hdc_mem);
}

/**
  * @brief  ������Ƶ����
  * @param  hwnd����Ļ���ڵľ��
  * @retval ��
  * @notes  
  */
TaskHandle_t h_avi;//���ֲ��Ž���
static int thread=0;
static void App_PlayVEDIO(HWND hwnd)
{
	
	int app=0;
   //HDC hdc;
   
	if(thread==0)
	{  
//      h_avi=rt_thread_create("App_PlayVEDIO",(void(*)(void*))App_PlayVEDIO,NULL,10*1024,1,5);
      xTaskCreate((TaskFunction_t )(void(*)(void*))App_PlayVEDIO,  /* ������ں��� */
                            (const char*    )"App_PlayVEDIO",/* �������� */
                            (uint16_t       )3*1024,  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )hwnd,/* ������ں������� */
                            (UBaseType_t    )7, /* ��������ȼ� */
                            (TaskHandle_t  )&h_avi);/* ������ƿ�ָ�� */
      thread =1;
//      rt_thread_startup(h_avi);//�����߳�
      power = sif.nValue;				
      return;
	}
	while(thread) //�߳��Ѵ�����
	{     
		if(app==0)
		{
         //hdc = GetDC(hwnd);
			app=1;
//      GUI_DEBUG("%s", avi_playlist[Play_index]);
      AVI_play(avi_playlist[Play_index], hwnd, power, power_horn);         
			app=0;
        // ReleaseDC(hwnd, hdc);
		}
	}
  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}
static void exit_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
  HDC hdc;
  RECT rc;
//  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
//  hwnd = ds->hwnd;

//  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
//  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

//  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_avi_play, rc_tmp.x, rc_tmp.y, SRCCOPY);

 SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
 FillRect(hdc, &rc);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));
	}
	else
	{ //��ť�ǵ���״̬

		SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));      //���û���ɫ
	}
  
  rc.w = 25;
  OffsetRect(&rc, 0, 8);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 6;
  }

}
/**
  * @brief  ���������б����
  * @param  ��
  * @retval ��
  * @notes  
  */
static TaskHandle_t h1;
static int avilist_thread=0;
static void App_AVIList()
{
//	static int thread=0;
	static int app=0;
   
	if(avilist_thread==0)
	{  
//      h1=rt_thread_create("App_AVIList",(void(*)(void*))App_AVIList,NULL,4096,5,5);
      xTaskCreate((TaskFunction_t )(void(*)(void*))App_AVIList,  /* ������ں��� */
                            (const char*    )"App_AVIList",/* �������� */
                            (uint16_t       )1*1024,  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )NULL,/* ������ں������� */
                            (UBaseType_t    )11, /* ��������ȼ� */
                            (TaskHandle_t  )&h1);/* ������ƿ�ָ�� */
//      rt_thread_startup(h1);				
      avilist_thread =1;
      return;
	}
//  while(1)
    {
    if(avilist_thread==1) //�߳��Ѵ�����
    {
      if(app==0)
      {
        app=1;
        GUI_AVIList_DIALOG();
        app=0;
        avilist_thread=0;
      }
    }
//    GUI_msleep(10);
  }
  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}


static SCROLLINFO sif_time;/*���ý������Ĳ���*/
HWND avi_wnd_time;


static HWND wnd;
static HWND wnd_horn;

static HWND wnd_power;//����icon���

static int Set_Widget_VCENTER(int y0, int h)
{
  return y0-h/2;
}
//HDC hdc_AVI=NULL;

extern HWND hwnd_AVI = NULL;
GUI_MUTEX*	AVI_JPEG_MUTEX = NULL;    // ����ȷ��һ֡ͼ���ú��ͷ������˳��߳�
static int t0=0;
static int frame=0;
volatile int win_fps=0;
extern volatile int avi_fps;
extern UINT      BytesRD;
extern uint8_t   Frame_buf[];

static LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static int ttt = 0;
   static BOOL res;
  static int IsCreate = 0;
  
   switch(msg)
   {
      case WM_CREATE:
      {
					t0 =GUI_GetTickCount();
				  frame =0;
					win_fps =0;
        
//          AVI_Player_hFont48 = GUI_Init_Extern_Font(AVI_Player_48);
//          AVI_Player_hFont64 = GUI_Init_Extern_Font(AVI_Player_64);
//          AVI_Player_hFont72 = GUI_Init_Extern_Font(AVI_Player_72);
//          AVI_Player_hFont48 = controlFont_48;
//          AVI_Player_hFont64 = controlFont_64;
//          AVI_Player_hFont72 = controlFont_72;
				

         AVI_JPEG_MUTEX = GUI_MutexCreate();    // ����һ���ݹ��ź���
        
			hdc_avi_play =CreateMemoryDC(SURF_SCREEN, 480, 272);
         ClrDisplay(hdc_avi_play, NULL, 0x363636);
#if 1 
         //����icon���л�����ģʽ�������ؿؼ����ֵ
         //avi_icon[0].rc.y = Set_Widget_VCENTER(440, avi_icon[0].rc.h);
         wnd_power = CreateWindow(BUTTON,L"A",WS_OWNERDRAW| WS_VISIBLE,//��ť�ؼ�������Ϊ�Ի��ƺͿ���
                                  avi_icon[0].rc.x,avi_icon[0].rc.y,//λ������Ϳؼ���С
                                  avi_icon[0].rc.w,avi_icon[0].rc.h,//��avi_icon[0]����
                                  hwnd,eID_BUTTON_Power,NULL,NULL);//������hwnd,IDΪID_BUTTON_Power�����Ӳ���Ϊ�� NULL
         //avi_icon[1].rc.y = Set_Widget_VCENTER(440, avi_icon[1].rc.h);                         
         //�����б�icon
         CreateWindow(BUTTON,L"D",WS_OWNERDRAW|WS_VISIBLE, //��ť�ؼ�������Ϊ�Ի��ƺͿ���
                      avi_icon[1].rc.x,avi_icon[1].rc.y,//λ������
                      avi_icon[1].rc.w,avi_icon[1].rc.h,//�ؼ���С
                      hwnd,eID_BUTTON_List,NULL,NULL);//������hwnd,IDΪID_BUTTON_List�����Ӳ���Ϊ�� NULL

         CreateWindow(BUTTON,L"Q",WS_OWNERDRAW|WS_VISIBLE, //��ť�ؼ�������Ϊ�Ի��ƺͿ���
                      avi_icon[13].rc.x,avi_icon[13].rc.y,//λ������
                      avi_icon[13].rc.w,avi_icon[13].rc.h,//�ؼ���С
                      hwnd,eID_BUTTON_Bugle,NULL,NULL);//������hwnd,IDΪID_BUTTON_Bugle�����Ӳ���Ϊ�� NULL

         //avi_icon[2].rc.y = Set_Widget_VCENTER(440, avi_icon[2].rc.h);
         //��һ��icon
         CreateWindow(BUTTON,L"S",WS_OWNERDRAW| WS_VISIBLE, //��ť�ؼ�������Ϊ�Ի��ƺͿ���
                      avi_icon[2].rc.x,avi_icon[2].rc.y,//λ������
                      avi_icon[2].rc.w,avi_icon[2].rc.h,//�ؼ���С
                      hwnd,eID_BUTTON_Back,NULL,NULL);//������hwnd,IDΪID_BUTTON_List�����Ӳ���Ϊ�� NULL
         // avi_icon[3].rc.y = Set_Widget_VCENTER(440, avi_icon[3].rc.h);
         //����icon
         CreateWindow(BUTTON,L"U",WS_OWNERDRAW| WS_VISIBLE, //��ť�ؼ�������Ϊ�Ի��ƺͿ���
                      avi_icon[3].rc.x,avi_icon[3].rc.y,//λ������
                      avi_icon[3].rc.w,avi_icon[3].rc.h,//�ؼ���С
                      hwnd,eID_BUTTON_Play,NULL,NULL);//������hwnd,IDΪID_BUTTON_List�����Ӳ���Ϊ�� NULL
        // avi_icon[4].rc.y = Set_Widget_VCENTER(440, avi_icon[4].rc.h);
         //����icon
         CreateWindow(BUTTON,L"V",WS_OWNERDRAW| WS_VISIBLE, //��ť�ؼ�������Ϊ�Ի��ƺͿ���
                      avi_icon[4].rc.x,avi_icon[4].rc.y,//λ������
                      avi_icon[4].rc.w,avi_icon[4].rc.h,//�ؼ���С
                      hwnd,eID_BUTTON_Next,NULL,NULL);//������hwnd,IDΪID_BUTTON_List�����Ӳ���Ϊ�� NULL
         
         CreateWindow(BUTTON,L"�ֱ��ʣ�0*0",WS_OWNERDRAW| WS_VISIBLE,
                      119,20,121,20,hwnd,eID_TB2,NULL,NULL);
 
         
         //��������
         CreateWindow(BUTTON,L"��Ƶ������",WS_OWNERDRAW| WS_VISIBLE,
                      100,0,280,20,hwnd,eID_TB1,NULL,NULL);
                      
         //��ʱ��        
         CreateWindow(BUTTON,L"00:00:00",WS_OWNERDRAW| WS_VISIBLE,
                      411, 215, 66, 25,hwnd,eID_TB4,NULL,NULL);

         //��ǰʱ��           
         CreateWindow(BUTTON,L"00:00:00",WS_OWNERDRAW| WS_VISIBLE,
                      3, 215, 66, 25,hwnd,eID_TB5,NULL,NULL);
     
                     
         CreateWindow(BUTTON,L"֡��:0FPS/s",WS_OWNERDRAW| WS_VISIBLE,
                      240, 20, 119, 20,hwnd,eID_TB3,NULL,NULL);

         /*********************����������******************/
         sif_time.cbSize = sizeof(sif_time);
         sif_time.fMask = SIF_ALL;
         sif_time.nMin = 0;
         sif_time.nMax = 255;
         sif_time.nValue = 0;//��ʼֵ
         sif_time.TrackSize = 22;//����ֵ
         sif_time.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������          
         avi_wnd_time = CreateWindow(SCROLLBAR, L"SCROLLBAR_Time",  WS_OWNERDRAW|WS_VISIBLE, 
                         75, 213, 331, 23, hwnd, eID_SCROLLBAR_TIMER, NULL, NULL);
         SendMessage(avi_wnd_time, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif_time);
         /*********************����ֵ������******************/
         sif.cbSize = sizeof(sif);
         sif.fMask = SIF_ALL;
         sif.nMin = 0;
         sif.nMax = 63;//�������ֵΪ63
         sif.nValue = 20;//��ʼ����ֵ
         sif.TrackSize = 22;//����ֵ
         sif.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
         wnd = CreateWindow(SCROLLBAR, L"SCROLLBAR_R", WS_OWNERDRAW, 
                            40, 241, 67, 23, hwnd, eID_SCROLLBAR_POWER, NULL, NULL);
         SendMessage(wnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);      

         /*********************��������ֵ������******************/
         sif_horn.cbSize = sizeof(sif_horn);
         sif_horn.fMask = SIF_ALL;
         sif_horn.nMin = 0;
         sif_horn.nMax = 63;//�������ֵΪ63
         sif_horn.nValue = 40;//��ʼ����ֵ
         sif_horn.TrackSize = 22;//����ֵ
         sif_horn.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
         wnd_horn = CreateWindow(SCROLLBAR, L"SCROLLBAR_R", WS_OWNERDRAW, 
                            40, 241, 67, 23, hwnd, eID_SCROLLBAR_HORN, NULL, NULL);
         SendMessage(wnd_horn, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif_horn);     
         
         
         CreateWindow(BUTTON, L"O",WS_OWNERDRAW|WS_VISIBLE,
                        444, 0, 36, 33, hwnd, eID_EXIT, NULL, NULL);         
 #endif   
         
         
         u8 *jpeg_buf;
         u32 jpeg_size;
         JPG_DEC *dec;
         res = RES_Load_Content(GUI_RGB_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
         hdc_bk = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
         if(res)
         {
            /* ����ͼƬ���ݴ���JPG_DEC��� */
            dec = JPG_Open(jpeg_buf, jpeg_size);

            /* �������ڴ���� */
            JPG_Draw(hdc_bk, 0, 0, dec);

            /* �ر�JPG_DEC��� */
            JPG_Close(dec);
         }
         /* �ͷ�ͼƬ���ݿռ� */
         RES_Release_Content((char **)&jpeg_buf);         
         //GUI_AVIList_DIALOG();
         Player_Init();
         break;
      }

      case WM_DRAWITEM:
      {     
         DRAWITEM_HDR *ds;
         ds = (DRAWITEM_HDR*)lParam;
         if (ds->ID == eID_SCROLLBAR_POWER || ds->ID == eID_SCROLLBAR_TIMER || ds->ID == eID_SCROLLBAR_HORN)
         {
            scrollbar_owner_draw(ds);
            return TRUE;
         }//         
         if(ds->ID == eID_EXIT)
         {
            exit_owner_draw(ds);
            return TRUE;         
         }
         else if ((ds->ID >= eID_BUTTON_Power && ds->ID<= eID_BUTTON_Bugle))
         {
            button_owner_draw(ds);
            return TRUE;
         }
         if(ds->ID >= eID_TB1 && ds->ID <= eID_TB5)
         {
            AVI_Button_OwnerDraw(ds);
            return TRUE;
         }

      }  
//		case	WM_CTLCOLOR:
//		{
//			/* �ؼ��ڻ���ǰ���ᷢ�� WM_CTLCOLOR��������.
//			 * wParam����ָ���˷��͸���Ϣ�Ŀؼ�ID;lParam����ָ��һ��CTLCOLOR�Ľṹ��ָ��.
//			 * �û�����ͨ������ṹ��ı�ؼ�����ɫֵ.�û��޸���ɫ�������践��TRUE������ϵͳ
//			 * �����Ա��β���������ʹ��Ĭ�ϵ���ɫ���л���.
//			 *
//			 */
//			u16 id;
//			id =LOWORD(wParam);
//			if(id== ID_TB1 || id== ID_TB2 || id== ID_TB4 || id== ID_TB3)
//			{
//				CTLCOLOR *cr;
//				cr =(CTLCOLOR*)lParam;
//				cr->TextColor =RGB888(255,255,255);//������ɫ��RGB888��ɫ��ʽ)
//				cr->BackColor =RGB888(0,0,0);//������ɫ��RGB888��ɫ��ʽ)
//				cr->BorderColor =RGB888(255,0,0);//�߿���ɫ��RGB888��ɫ��ʽ)
//				return TRUE;
//			}
//			else
//			{
//				return FALSE;
//			}
//		}			
      //���ƴ��ڽ�����Ϣ
      case WM_PAINT:
      {
         PAINTSTRUCT ps;
         HDC hdc;//��Ļhdc
         
//				WCHAR wbuf[40];
         RECT rc;
         int t1;
         GetClientRect(hwnd, &rc);
         hdc = BeginPaint(hwnd, &ps);   
//         SetBrushColor(hdc, MapRGB(hdc, 0,0,0));
//         FillRect(hdc, &rc);
         frame++;
         t1 =GUI_GetTickCount();
         if((t1-t0)>=1000)
         {
            win_fps =frame;
            t0 =t1;
            frame =0;
         }
                
         
         
         color_bg = GetPixel(hdc, 385, 404);
         EndPaint(hwnd, &ps);
         if(ttt == 0)
         {
            ttt = 1;
            App_PlayVEDIO(hwnd);
         }
         break;
      }
      case WM_ERASEBKGND:
      {
         HDC hdc =(HDC)wParam;
         // RECT rc =*(RECT*)lParam;
         
         
         if (hide_flag == 0)
         {  
            RECT rc = {0, 40, GUI_XSIZE, GUI_YSIZE - 62};
            
            BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_avi_play, rc.x, rc.y, SRCCOPY);//hdc_bk
            
            rc.y = 0;
            rc.h = 40;
            SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
            FillRect(hdc, &rc);

            rc.y = GUI_YSIZE - 62;
            rc.h = 62;
            SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
            FillRect(hdc, &rc);
         }
         else
         {
            RECT rc = {0, 0, GUI_XSIZE, GUI_YSIZE};
            
            BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_avi_play, rc.x, rc.y, SRCCOPY);
         }
         return FALSE;

      }     

      case WM_LBUTTONUP:
      {
         hide_flag = !hide_flag;
        for (uint32_t xC=eID_SCROLLBAR_POWER; xC<=eID_TB5; xC++)
        {
          ShowWindow(GetDlgItem(hwnd, xC), hide_flag ? SW_HIDE : SW_SHOW);
        }
        InvalidateRect(hwnd,NULL, TRUE);
      } break;
      
      case WM_NOTIFY:
      {
         u16 code, id, ctr_id;
         id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
         code=HIWORD(wParam);//��ȡ��Ϣ������
         //���͵���
         if(code == BN_CLICKED)
         { 
            switch(id)
            {
               //����icon����case
               case eID_BUTTON_Power:
               {

                  avi_icon[0].state = ~avi_icon[0].state;
                  //InvalidateRect(hwnd, &music_icon[0].rc, TRUE);
                  //������iconδ������ʱ
                  if(avi_icon[0].state == FALSE)
                  {
                     WCHAR wbuf[3];
                    HWND  wnd1 = GetDlgItem(hwnd, eID_BUTTON_Bugle);

                    GetWindowText(wnd1, wbuf, 3);
                    if (wbuf[0] == L'P')//Ϊ���������
                    {
                      ShowWindow(wnd_horn, SW_HIDE); //��������
                    }
                    else// Ϊ�������
                    {
                      ShowWindow(wnd, SW_HIDE); //��������
                    }
                  }
                  //������icon������ʱ������Ϊ����ģʽ
                  else
                  {        
                      WCHAR wbuf[3];
                      HWND  wnd1 = GetDlgItem(hwnd, eID_BUTTON_Bugle);
                   
                      GetWindowText(wnd1, wbuf, 3);
                      if (wbuf[0] == L'P')//Ϊ���������
                      {
                         ShowWindow(wnd_horn, SW_SHOW); //��������
                      }
                      else// Ϊ�������
                      {
                         ShowWindow(wnd, SW_SHOW); //������ʾ
                      }        
                  }
                  break;
               }              
            
//               case ID_BUTTON_List:
//               {
//                 //if(IsCreate == 0)
//                 {
//                   IsCreate = 1;
//                   App_AVIList();
//                 }
////                 else
////                   avilist_thread = 1;
//                  break;
//               }

               /* ��Ƶ���ѡ��ť */
               case eID_BUTTON_Bugle:
               {
                  WCHAR wbuf[3];
                  HWND  wnd1 = GetDlgItem(hwnd, eID_BUTTON_Bugle);
               
                  GetWindowText(wnd1, wbuf, 3);
                  if (wbuf[0] == L'P')
                  {
                     SetWindowText(wnd1, L"Q");
                     wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);    // ����Ϊ�������
                  }
                  else
                  {
                     SetWindowText(wnd1, L"P");
                     wm8978_CfgAudioPath(DAC_ON, SPK_ON);                        // ����Ϊ���������
                  }

                  if(avi_icon[0].state != FALSE)    // �������ڻ������ѵ������л����ڻ�����
                  {
                     if (wbuf[0] == L'P')     // Ϊ�������������ոոı��ˣ���
                     {
                        ShowWindow(wnd_horn, SW_HIDE); // ������ʾ
                        ShowWindow(wnd, SW_SHOW);      // ������ʾ
                     }
                     else         // Ϊ�������������ոոı��ˣ���
                     {
                        ShowWindow(wnd_horn, SW_SHOW); // ������ʾ
                        ShowWindow(wnd, SW_HIDE);      // ��������
                     }
                  }
               }
               break; 
               case eID_BUTTON_Play:
               {

                  avi_icon[3].state = ~avi_icon[3].state;
                  //InvalidateRect(hwnd, &avi_icon[0].rc, TRUE);
                  //������iconδ������ʱ
                  if(avi_icon[3].state == FALSE)
                  {
                     I2S_Play_Start();
                     TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
                     TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3                        
                     
                     SetWindowText(GetDlgItem(hwnd, eID_BUTTON_Play), L"U");
                              
                  }
                  //������icon������ʱ����ͣ
                  else
                  {          
                     I2S_Play_Stop();
                     TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE); //����ʱ��3�����ж�
                     TIM_Cmd(TIM3,DISABLE); //ʹ�ܶ�ʱ��3                     
                     SetWindowText(GetDlgItem(hwnd, eID_BUTTON_Play), L"T");
                  }
                  //
                  break;
               }
               case eID_BUTTON_Back:
               {
            
                  Play_index--;
                  if(Play_index < 0)
                     Play_index = file_nums - 1;  
                  sw_flag = 1;   

                  
                  sif_time.nValue = 0;//����Ϊ0
                  SendMessage(avi_wnd_time, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif_time);                  
                  

                  break;
               }
               case eID_BUTTON_Next:
               {                  
                  Play_index++;
                  
                  if(Play_index > file_nums -1 )
                     Play_index = 0;
                  sw_flag = 1;

                  sif_time.nValue = 0;//����Ϊ0
                  SendMessage(avi_wnd_time, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif_time);                    
                  break;
               }
               case eID_EXIT:
               {
                  PostCloseMessage(hwnd);
                  break;
               }
            }
         }
        if(id==eID_BUTTON_List && code==BN_CLICKED)
        {
          /* �����б�ֹͣ���� */
          I2S_Play_Stop();
          TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE); //����ʱ��3�����ж�
          TIM_Cmd(TIM3,DISABLE); //ʹ�ܶ�ʱ��3                     
          SetWindowText(GetDlgItem(hwnd, eID_BUTTON_Play), L"T");

          GUI_MutexLock(AVI_JPEG_MUTEX,0xFFFFFFFF);    // ��ȡ������
          WNDCLASS wcex;
          LIST_STATE = 1;
          wcex.Tag	 		= WNDCLASS_TAG;
          wcex.Style			= CS_HREDRAW | CS_VREDRAW;
          wcex.lpfnWndProc	= (WNDPROC)list_win_proc;
          wcex.cbClsExtra		= 0;
          wcex.cbWndExtra		= 0;
          wcex.hInstance		= NULL;
          wcex.hIcon			= NULL;
          wcex.hCursor		= NULL;
          if(1)
          {
            RECT rc;

            GetClientRect(hwnd,&rc);

            CreateWindowEx(NULL,
                            &wcex,L"VideoList",
                            WS_VISIBLE|WS_CLIPCHILDREN,
                            0,0,GUI_XSIZE,GUI_YSIZE,
                            hwnd,0,NULL,NULL);
            GUI_MutexUnlock(AVI_JPEG_MUTEX);              // ����������
            
          }
        }
         
         NMHDR *nr;  
         ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID. 
         nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.
         //����������case
         if (ctr_id == eID_SCROLLBAR_TIMER)
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
                  //��λ���������λ��
                  avi_chl = 1;//������־
               }
               break;
            }
         }
         //��������������case
         static int ttt = 0;
         if (ctr_id == eID_SCROLLBAR_POWER)
         {
            NM_SCROLLBAR *sb_nr;
            sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
            
            switch (nr->code)
            {
               case SBN_THUMBTRACK: //R�����ƶ�
               {
                  power= sb_nr->nTrackValue; //�õ���ǰ������ֵ
                  if(power == 0) 
                  {
                     wm8978_OutMute(1);//����
                     SendMessage(wnd_horn, SBM_SETVALUE, TRUE, power_horn); //����SBM_SETVALUE����������ֵ
                     SetWindowText(wnd_power, L"J");
                     ttt = 1;
                     
                  }
                  else
                  {
                     if(ttt == 1)
                     {
                        SetWindowText(wnd_power, L"A");
                        ttt = 0;
                     }
                     wm8978_OutMute(0);
                     wm8978_SetOUT1Volume(power);//����WM8978������ֵ
                  } 
                  SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, power); //����SBM_SETVALUE����������ֵ
               }
               break;
            }
         }   
         

         //��������������case

         if (ctr_id == eID_SCROLLBAR_HORN)
         {
            NM_SCROLLBAR *sb_nr;
            sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
            
            switch (nr->code)
            {
               case SBN_THUMBTRACK: //R�����ƶ�
               {
                  power_horn= sb_nr->nTrackValue; //�õ���ǰ������ֵ
                  if(power_horn == 0) 
                  {
                     wm8978_OutMute(1);//����
                     SendMessage(wnd, SBM_SETVALUE, TRUE, power); //����SBM_SETVALUE����������ֵ
                     SetWindowText(wnd_power, L"J");
                     ttt = 1;
                     
                  }
                  else
                  {
                     if(ttt == 1)
                     {
                        SetWindowText(wnd_power, L"A");
                        ttt = 0;
                     }
                     wm8978_OutMute(0);
                     wm8978_SetOUT2Volume(power_horn);//����WM8978������ֵ
                  } 
                  SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, power_horn); //����SBM_SETVALUE����������ֵ
               }
               break;
            }
         } 
      } 
      break; 
      
      case WM_CLOSE:
      {
        GUI_MutexLock(AVI_JPEG_MUTEX,0xFFFFFFFF);    // ��ȡ������ȷ��һ֡ͼ����ڴ�ʹ�ú����ͷ�
         if(IsCreate)
         {
          IsCreate=0;
          GUI_Thread_Delete(h1);
         }
         GUI_MutexDelete(AVI_JPEG_MUTEX);
         DeleteDC(hdc_avi_play);
         thread_ctrl = 0;
         I2S_Play_Stop();
         I2S_Stop();		/* ֹͣI2S¼���ͷ��� */
         wm8978_Reset();	/* ��λWM8978����λ״̬ */
         TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE); //����ʱ��3�����ж�
         thread = 0;
         ttt = 0;//��������ִֻ��һ�εļ�¼����
         //rt_thread_delete(h1);
         power=20;
         Play_index = 0;
         avi_file_num = 0;
         res = FALSE;
         GUI_Thread_Delete(h_avi);
         
         DeleteDC(hdc_bk);
         return DestroyWindow(hwnd); //����DestroyWindow���������ٴ��ڣ��ú��������WM_DESTROY��Ϣ����; //�رմ��ڷ���TRUE��
      }
      default :
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }  
   return WM_NULL;
}



//���ֲ��������
HWND	VideoPlayer_hwnd;
void	GUI_VideoPlayer_DIALOG(void)
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
	VideoPlayer_hwnd = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                                    &wcex,
                                    L"GUI_MUSICPLAYER_DIALOG",
                                    WS_VISIBLE|WS_OVERLAPPED|WS_CLIPCHILDREN,
                                    0, 0, GUI_XSIZE, GUI_YSIZE,
                                    NULL, NULL, NULL, NULL);

	//��ʾ������
	ShowWindow(VideoPlayer_hwnd, SW_SHOW);
  
	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, VideoPlayer_hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


