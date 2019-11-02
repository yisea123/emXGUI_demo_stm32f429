#include "emXGUI.h"
#include "x_libc.h"

#include "./led/bsp_led.h"  
#include "GUI_AppDef.h"
#include "GUI_RGBLED_DIALOG.h"
#include "Widget.h"
#include "emXGUI_JPEG.h"
/* Ӳ�����ƽӿ� */
extern void TIM_GPIO_Config(void);
extern void TIM_Mode_Config(void);
extern void TIM_RGBLED_Close(void);
extern void SetRGBColor(uint32_t rgb);
extern void SetColorValue(uint8_t r,uint8_t g,uint8_t b);
extern const unsigned char RGBdesktop[];
//static BITMAP RGBdesktop_0;
/**********************�ֽ���*********************/
struct leddlg
{
	char col_R;  //R����ֵ
	char col_G;  //G����ֵ
	char col_B;  //B����ֵ
   char led_R;//Ӳ��RGB����ɫ����ֵ
   char led_G;//Ӳ��RGB����ɫ����ֵ
   char led_B;//Ӳ��RGB����ɫ����ֵ
   int colR_ctr;//Ӳ��RGB�ƿ���λ
   int colG_ctr;//Ӳ��RGB�ƿ���λ
   int colB_ctr;//Ӳ��RGB�ƿ���λ
}leddlg_S={255, 165, 208, 255, 165, 208, 1, 1, 1};

icon_S GUI_RGBLED_Icon[18] = 
{
      {"tuichu",           {730,0,70,70},       FALSE},//�˳�����
      {"biaotilan",        {100,0,600,80},      FALSE},//APP������
      {"APPHouse",         {425,80,275,275},      FALSE},//APP����ͼ��
      {"hongdeng",         {93, 80, 72, 72},  FALSE},//���ͼ��
      {"lvdeng",           {222, 80, 72, 72},  FALSE},//�̵�ͼ��
      {"landeng",          {350, 80, 72, 72},  FALSE},//����ͼ��
      {"hongdengscrollbar",{113, 155, 35, 200},  FALSE},//��ɫ������
      {"lvdengscrollbar",  {242, 155, 35, 200},  FALSE},//��ɫ������
      {"landengscrollbar", {370, 155, 35, 200},  FALSE},//��ɫ������ 
      
      {"kongzhikaiguan",   {675, 355,160,160},  FALSE},//���ƿ���
      
      {"hongdengwenzi",    {96, 420, 72, 50}, FALSE},//����-���
      {"lvdengwenzi",      {226, 420, 72, 50}, FALSE},//����-�̵�
      {"landengwenzi",     {356, 420, 72, 50}, FALSE},//����-����
      {"Rshuzhi",          {96, 355, 72, 72}, FALSE},//����-R��ֵ
      {"Gshuzhi",          {226, 355, 72, 72}, FALSE},//����-G��ֵ
      {"Bshuzhi",          {356, 355, 72, 72}, FALSE},//����-B��ֵ
};

RGBLED_DIALOG_s RGBLED_DIALOG =
{
   .RGBLED_Hwnd = NULL,
   .State = TRUE,
   .exit_bt_draw = home_owner_draw,
   .col_R = 255,
   .col_G = 165,
   .col_B = 208,
   
};

static void Delete_DlALOG()
{
   RGBLED_DIALOG.col_R = 255;
   RGBLED_DIALOG.col_G = 165;
   RGBLED_DIALOG.col_B = 208;
   RGBLED_DIALOG.State = 1;
   leddlg_S.colB_ctr = 1;
   leddlg_S.colG_ctr = 1;
   leddlg_S.colR_ctr = 1;
   leddlg_S.led_R = 255;
   leddlg_S.led_G = 165;
   leddlg_S.led_B = 208;
   leddlg_S.col_R = 255;
   leddlg_S.col_G = 165;
   leddlg_S.col_B = 208;
   DeleteDC(RGBLED_DIALOG.hdc_mem);
   TIM_RGBLED_Close();
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
static void GUI_RGBLED_drawscrollbar_V(HWND hwnd, HDC hdc, COLOR_RGB32 back_c, COLOR_RGB32 Page_c, COLOR_RGB32 fore_c)
{
	 RECT rc, rc_tmp;
  
   RECT rc_scrollbar;

   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, RGBLED_DIALOG.hdc_mem, rc_tmp.x, rc_tmp.y, SRCCOPY);

   rc_scrollbar.x = rc.w/2;
   rc_scrollbar.y = rc.y;
   rc_scrollbar.w = 2;
   rc_scrollbar.h = rc.h;
   EnableAntiAlias(hdc, TRUE);
   SetBrushColor(hdc, MapRGB888(hdc, Page_c));
	 FillRect(hdc, &rc_scrollbar);

	/* ���� */
	SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);

	SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
//	rc.x += (rc.w >> 2) >> 1;
//	rc.w -= rc.w >> 2;
	/* �߿� */
	FillCircle(hdc, rc.x + rc.h / 2+1, rc.y + rc.h / 2, rc.h / 2);
   InflateRect(&rc, -2, -2);

	SetBrushColor(hdc, MapRGB888(hdc, fore_c));
	FillCircle(hdc, rc.x + rc.h / 2+1, rc.y + rc.h / 2, rc.h / 2);
  EnableAntiAlias(hdc, FALSE);
}
/*
 * @brief  �Զ���ص�����
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void GUI_RGBLED_ScrollbarOwnerDraw_V(DRAWITEM_HDR *ds)
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
	GUI_RGBLED_drawscrollbar_V(hwnd, hdc_mem1, ID_SCROLLBAR_BackColor, RGB888(250, 250, 250), RGB888(255, 255, 255));
	//������ɫ���͵Ĺ�����
	switch (ds->ID)
	{
		case ID_SCROLLBAR_R:
		{
			GUI_RGBLED_drawscrollbar_V(hwnd, hdc_mem, ID_SCROLLBAR_BackColor, RGB888(leddlg_S.col_R, 0, 0), RGB888(leddlg_S.col_R, 0, 0));
			break;
		}
		case ID_SCROLLBAR_G:
		{
			GUI_RGBLED_drawscrollbar_V(hwnd, hdc_mem, ID_SCROLLBAR_BackColor, RGB888(0, leddlg_S.col_G, 0), RGB888(0, leddlg_S.col_G, 0));
			break;
		}
		case ID_SCROLLBAR_B:
		{
			GUI_RGBLED_drawscrollbar_V(hwnd, hdc_mem, ID_SCROLLBAR_BackColor, RGB888(0, 0, leddlg_S.col_B), RGB888(0, 0, leddlg_S.col_B));
			break;
		}
	}
   
   
   SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);
	//���ƻ���
	if (ds->State & SST_THUMBTRACK)//����
	{
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem, rc.x, 0, SRCCOPY);
	}
	else//δѡ��
	{
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem1, rc.x, 0, SRCCOPY);
	}	
	//��
	BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc.y, hdc_mem, 0, 0, SRCCOPY);
	//��
	BitBlt(hdc, rc_cli.x, rc.y+rc.h, rc_cli.w , rc_cli.h-(rc.y+rc.h), hdc_mem1, 0, rc.y + rc.h, SRCCOPY);

	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem1);
	DeleteDC(hdc_mem);
}

void GUI_RGBLED_HomeOwnerDraw(DRAWITEM_HDR *ds) 
{
	HWND hwnd;
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

	
  
   SetTextColor(hdc, MapARGB(hdc, 255,255, 255, 255));
   //��ť����״̬
   if (ds->State & BST_PUSHED)
	{ 
		SetBrushColor(hdc, MapARGB(hdc, 255,105,105,105));
	}
	else//��ť����״̬
	{ 
		SetBrushColor(hdc, MapARGB(hdc, 255,215,61,50));
	}
   FillCircle(hdc, rc.x+rc.w, rc.y, rc.w);
   /* ʹ�ÿ���ͼ������ */
	SetFont(hdc, controlFont_64);
	GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
   rc.y = -10;
   rc.x = 16;
	DrawText(hdc, wbuf, -1, &rc, NULL);//��������(���ж��뷽ʽ)


  /* �ָ�Ĭ������ */
	SetFont(hdc, defaultFont);
}
/**
  * @brief  CheckBox��ť�ؼ����ػ���
  * @param  ds:DRAWITEM_HDR�ṹ��
  * @retval NULL
  */
static void GUI_RGBLED_CheckBoxOwnerDraw(DRAWITEM_HDR *ds)
{
   HDC hdc; //�ؼ�����HDC
   HWND hwnd; //�ؼ���� 
   RECT rc_cli;//�ؼ���λ�ô�С����
   WCHAR wbuf[128];
	 hwnd = ds->hwnd;
	 hdc = ds->hDC; 
   GetClientRect(hwnd, &rc_cli);
   //��������㣬��ʽΪSURF_ARGB4444
   
	GetWindowText(ds->hwnd,wbuf,128); //��ð�ť�ؼ�������  
   
//   SetBrushColor(hdc,MapRGB(hdc, 169,169,169));
//   FillRect(hdc, &rc_cli);
   //NEXT����BACK����LIST������ʱ���ı���ɫ
	if((ds->State & BST_PUSHED))//��ť�ǰ���״̬
	{ 
      EnableAlpha(hdc,TRUE);
      EnableAntiAlias(hdc, TRUE);
      SetAlpha(hdc, 128);
      SetBrushColor(hdc,MapRGB(hdc, 185,218,251));
      FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2);       
      EnableAlpha(hdc,FALSE);
      SetBrushColor(hdc,MapRGB(hdc,  185,218,251));
      FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2-5);      
      EnableAntiAlias(hdc, FALSE);
	}
   else
   {
      EnableAntiAlias(hdc, TRUE);//�����
      if(RGBLED_DIALOG.State == TRUE)
      {
         EnableAlpha(hdc,TRUE);
         SetAlpha(hdc, 128);
         SetBrushColor(hdc,MapRGB(hdc, 50,205,50));
         FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2); 
         
         EnableAlpha(hdc,FALSE);
         SetBrushColor(hdc,MapRGB(hdc,  50,205,50));
         
         FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2-5);                 
      }
      else
      {
         EnableAlpha(hdc,TRUE);
         SetAlpha(hdc, 128);
         
         SetBrushColor(hdc,MapRGB(hdc, 220,20,60));
         FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2); 
         
         EnableAlpha(hdc,FALSE);
         SetBrushColor(hdc,MapRGB(hdc,  220,20,60));
         FillCircle(hdc, rc_cli.x + rc_cli.w/2, rc_cli.y + rc_cli.w/2, rc_cli.w/2-5);          
      }
      EnableAntiAlias(hdc, FALSE);
   }
   SetFont(hdc, controlFont_32);
   SetTextColor(hdc, MapRGB(hdc, 255,255,255));      //��������ɫ
   rc_cli.x = 35;
   rc_cli.y = 55;
   if(RGBLED_DIALOG.State == TRUE)
   { 
      DrawText(hdc, L"op",-1,&rc_cli,NULL);//��������(���ж��뷽ʽ)           
   }
   else
   {     
      DrawText(hdc, L"np",-1,&rc_cli,NULL);//��������(���ж��뷽ʽ)         
   }      
}
/**
  * @brief  button_owner_draw ��ť�ؼ����ػ���
  * @param  ds:DRAWITEM_HDR�ṹ��
  * @retval NULL
  */
static void GUI_RGBLEDButton_OwnerDraw(DRAWITEM_HDR *ds)
{
   HDC hdc; //�ؼ�����HDC
   HWND hwnd; //�ؼ���� 
   RECT rc_cli, rc_tmp;//�ؼ���λ�ô�С����
   WCHAR wbuf[128];
   hwnd = ds->hwnd;
	 hdc = ds->hDC; 
//   if(ds->ID ==  ID_BUTTON_START && show_lrc == 1)
//      return;
   //��ȡ�ؼ���λ�ô�С��Ϣ
   GetClientRect(hwnd, &rc_cli);
   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   //GetClientRect(hwnd, &rc_cli);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, RGBLED_DIALOG.hdc_mem, rc_tmp.x, rc_tmp.y, SRCCOPY);  
  
   
   //��������㣬��ʽΪSURF_ARGB4444
	 GetWindowText(ds->hwnd,wbuf,128); //��ð�ť�ؼ�������  
   //EnableAlpha(hdc,TRUE);

   //���ż�ʹ��100*100������
   //���ð�������ɫ
   SetTextColor(hdc, MapRGB(hdc,250,250,250));
   //NEXT����BACK����LIST������ʱ���ı���ɫ

 
   DrawText(hdc, wbuf,-1,&rc_cli,DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
    
}
static	LRESULT	win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static SURFACE *pSurfTop = NULL;
   static HDC hdc_bk = NULL;
   
   RECT rc;
	switch (msg)
	{
      case WM_CREATE: 
      {
         WCHAR wbuf[128];
         GetClientRect(hwnd, &rc);
         //pSurfTop = CreateSurface(SURF_ARGB4444, rc.w, rc.h, NULL, 0);
         
 

          CreateWindow(BUTTON, L"O",WS_TRANSPARENT|WS_OWNERDRAW|WS_VISIBLE,
                      GUI_RGBLED_Icon[0].rc.x, GUI_RGBLED_Icon[0].rc.y, 
                      GUI_RGBLED_Icon[0].rc.w, GUI_RGBLED_Icon[0].rc.h, 
                      hwnd, ID_EXIT, NULL, NULL); 

         
         CreateWindow(TEXTBOX, L"ȫ��LED��", WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[1].rc.x, GUI_RGBLED_Icon[1].rc.y, 
                      GUI_RGBLED_Icon[1].rc.w, GUI_RGBLED_Icon[1].rc.h, 
                      hwnd, ID_TEXTBOX_Title, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_Title),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER); 
         
         CreateWindow(TEXTBOX, L"c", WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[2].rc.x, GUI_RGBLED_Icon[2].rc.y, 
                      GUI_RGBLED_Icon[2].rc.w, GUI_RGBLED_Icon[2].rc.h, 
                      hwnd, ID_TEXTBOX_APP, NULL, NULL);
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_APP), iconFont_252);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_APP),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER);

         CreateWindow(TEXTBOX, L"k", WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[3].rc.x, GUI_RGBLED_Icon[3].rc.y, 
                      GUI_RGBLED_Icon[3].rc.w, GUI_RGBLED_Icon[3].rc.h, 
                      hwnd, ID_TEXTBOX_R_LED, NULL, NULL);
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_R_LED), controlFont_72);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_R_LED),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER); 
                        
         CreateWindow(TEXTBOX, L"k", WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[4].rc.x, GUI_RGBLED_Icon[4].rc.y, 
                      GUI_RGBLED_Icon[4].rc.w, GUI_RGBLED_Icon[4].rc.h, 
                      hwnd, ID_TEXTBOX_G_LED, NULL, NULL);
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_G_LED), controlFont_72);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_G_LED),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER); 
                        
         CreateWindow(TEXTBOX, L"k", WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[5].rc.x, GUI_RGBLED_Icon[5].rc.y, 
                      GUI_RGBLED_Icon[5].rc.w, GUI_RGBLED_Icon[5].rc.h, 
                      hwnd, ID_TEXTBOX_B_LED, NULL, NULL);
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_B_LED), controlFont_72);         
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_B_LED),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER); 



         RGBLED_DIALOG.sif_R.cbSize = sizeof(RGBLED_DIALOG.sif_R);
         RGBLED_DIALOG.sif_R.fMask = SIF_ALL;
         RGBLED_DIALOG.sif_R.nMin = 0;
         RGBLED_DIALOG.sif_R.nMax = 255;
         RGBLED_DIALOG.sif_R.nValue = RGBLED_DIALOG.col_R;
         RGBLED_DIALOG.sif_R.TrackSize = 35;
         RGBLED_DIALOG.sif_R.ArrowSize = 0;

         /*����������--R*/
         CreateWindow(SCROLLBAR, L"SCROLLBAR_R", SBS_VERT|WS_OWNERDRAW |WS_TRANSPARENT| WS_VISIBLE, 
                      GUI_RGBLED_Icon[6].rc.x, GUI_RGBLED_Icon[6].rc.y, 
                      GUI_RGBLED_Icon[6].rc.w, GUI_RGBLED_Icon[6].rc.h, 
                      hwnd, ID_SCROLLBAR_R, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_SCROLLBAR_R), SBM_SETSCROLLINFO, TRUE, (LPARAM)&RGBLED_DIALOG.sif_R);

         RGBLED_DIALOG.sif_G.cbSize = sizeof(RGBLED_DIALOG.sif_G);
         RGBLED_DIALOG.sif_G.fMask = SIF_ALL;
         RGBLED_DIALOG.sif_G.nMin = 0;
         RGBLED_DIALOG.sif_G.nMax = 255;
         RGBLED_DIALOG.sif_G.nValue = RGBLED_DIALOG.col_G;
         RGBLED_DIALOG.sif_G.TrackSize = 35;
         RGBLED_DIALOG.sif_G.ArrowSize = 0;
         /*����������--G*/
         CreateWindow(SCROLLBAR, L"SCROLLBAR_G", SBS_VERT|WS_OWNERDRAW |WS_TRANSPARENT| WS_VISIBLE, 
                      GUI_RGBLED_Icon[7].rc.x, GUI_RGBLED_Icon[7].rc.y, 
                      GUI_RGBLED_Icon[7].rc.w, GUI_RGBLED_Icon[7].rc.h, 
                      hwnd, ID_SCROLLBAR_G, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_SCROLLBAR_G), SBM_SETSCROLLINFO, TRUE, (LPARAM)&RGBLED_DIALOG.sif_G);
         RGBLED_DIALOG.sif_B.cbSize = sizeof(RGBLED_DIALOG.sif_B);
         RGBLED_DIALOG.sif_B.fMask = SIF_ALL;
         RGBLED_DIALOG.sif_B.nMin = 0;
         RGBLED_DIALOG.sif_B.nMax = 255;
         RGBLED_DIALOG.sif_B.nValue = RGBLED_DIALOG.col_B;
         RGBLED_DIALOG.sif_B.TrackSize = 35;
         RGBLED_DIALOG.sif_B.ArrowSize = 0;
         /*����������--B*/
         CreateWindow(SCROLLBAR, L"SCROLLBAR_B", SBS_VERT|WS_OWNERDRAW | WS_TRANSPARENT|WS_VISIBLE, 
                      GUI_RGBLED_Icon[8].rc.x, GUI_RGBLED_Icon[8].rc.y, 
                      GUI_RGBLED_Icon[8].rc.w, GUI_RGBLED_Icon[8].rc.h,
                      hwnd, ID_SCROLLBAR_B, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_SCROLLBAR_B), SBM_SETSCROLLINFO, TRUE, (LPARAM)&RGBLED_DIALOG.sif_B);

         CreateWindow(BUTTON,L"Checkbox5",WS_TRANSPARENT|WS_OWNERDRAW|WS_VISIBLE,
                      GUI_RGBLED_Icon[9].rc.x, GUI_RGBLED_Icon[9].rc.y, 
                      GUI_RGBLED_Icon[9].rc.w, GUI_RGBLED_Icon[9].rc.h,
                      hwnd,ID_CHECKBOX_SW,NULL,NULL);         
         /*�����ı���--���*/
         CreateWindow(BUTTON, L"qp", WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW, 
                      GUI_RGBLED_Icon[10].rc.x, GUI_RGBLED_Icon[10].rc.y, 
                      GUI_RGBLED_Icon[10].rc.w, GUI_RGBLED_Icon[10].rc.h,
                      hwnd, ID_TEXTBOX_R, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_R),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_BKGND); 
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_R), controlFont_32);                        
         //R�ķ���ֵ 
         x_wsprintf(wbuf, L"%d", leddlg_S.col_R);
         CreateWindow(BUTTON, wbuf, WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW|WS_DISABLED, 
                      GUI_RGBLED_Icon[13].rc.x, GUI_RGBLED_Icon[13].rc.y, 
                      GUI_RGBLED_Icon[13].rc.w, GUI_RGBLED_Icon[13].rc.h,
                      hwnd, ID_TEXTBOX_R_NUM, NULL, NULL); 
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_R_NUM), controlFont_32);               
         //G�ķ���ֵ 
         x_wsprintf(wbuf, L"%d", leddlg_S.col_G);
         CreateWindow(BUTTON, wbuf, WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW|WS_DISABLED, 
                      GUI_RGBLED_Icon[14].rc.x, GUI_RGBLED_Icon[14].rc.y, 
                      GUI_RGBLED_Icon[14].rc.w, GUI_RGBLED_Icon[14].rc.h,          
                      hwnd, ID_TEXTBOX_G_NUM, NULL, NULL);
 
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_G_NUM), controlFont_32);                         
         //�����ı���--����         
         CreateWindow(BUTTON, L"rp", WS_TRANSPARENT|WS_OWNERDRAW|WS_VISIBLE, 
                      GUI_RGBLED_Icon[11].rc.x, GUI_RGBLED_Icon[11].rc.y, 
                      GUI_RGBLED_Icon[11].rc.w, GUI_RGBLED_Icon[11].rc.h,         
                      hwnd, ID_TEXTBOX_G, NULL, NULL);
         SendMessage(GetDlgItem(hwnd, ID_TEXTBOX_G),TBM_SET_TEXTFLAG,0,
                        DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_BKGND);   
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_G), controlFont_32);  
         //�����ı���--�̵�
         CreateWindow(BUTTON, L"sp", WS_TRANSPARENT|WS_OWNERDRAW|WS_VISIBLE, 
                      GUI_RGBLED_Icon[12].rc.x, GUI_RGBLED_Icon[12].rc.y, 
                      GUI_RGBLED_Icon[12].rc.w, GUI_RGBLED_Icon[12].rc.h,          
                      hwnd, ID_TEXTBOX_B, NULL, NULL);  
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_B), controlFont_32);
         //B�ķ���ֵ         
         x_wsprintf(wbuf, L"%d", leddlg_S.col_B);         
         CreateWindow(BUTTON, wbuf, WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW|WS_DISABLED, 
                      GUI_RGBLED_Icon[15].rc.x, GUI_RGBLED_Icon[15].rc.y, 
                      GUI_RGBLED_Icon[15].rc.w, GUI_RGBLED_Icon[15].rc.h,
                      hwnd, ID_TEXTBOX_B_NUM, NULL, NULL);
         SetWindowFont(GetDlgItem(hwnd, ID_TEXTBOX_B_NUM), controlFont_32);   

         //����λͼ�ṹ����
//         RGBdesktop_0.Format	= BM_RGB888;     //λͼ��ʽ
//         RGBdesktop_0.Width  = 240;              //���
//         RGBdesktop_0.Height = 158;             //�߶�
//         RGBdesktop_0.WidthBytes =RGBdesktop_0.Width*3; //ÿ���ֽ���
//         RGBdesktop_0.LUT =NULL;                //���ұ�(RGB/ARGB��ʽ��ʹ�øò���)
//         RGBdesktop_0.Bits =(void*)RGBdesktop;    //λͼ����
//         hdc_bk = CreateMemoryDC(SURF_SCREEN, RGBdesktop_0.Width, RGBdesktop_0.Height);
         BOOL res;
         u8 *jpeg_buf;
         u32 jpeg_size;
         JPG_DEC *dec;
         res = RES_Load_Content(GUI_RGB_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
         RGBLED_DIALOG.hdc_mem = CreateMemoryDC(SURF_SCREEN, 800, 480);
         if(res)
         {
            /* ����ͼƬ���ݴ���JPG_DEC��� */
            dec = JPG_Open(jpeg_buf, jpeg_size);

            /* �������ڴ���� */
            JPG_Draw(RGBLED_DIALOG.hdc_mem, 0, 0, dec);

            /* �ر�JPG_DEC��� */
            JPG_Close(dec);
         }
         /* �ͷ�ͼƬ���ݿռ� */
         RES_Release_Content((char **)&jpeg_buf);
         
         //DrawBitmap(hdc_bk,0, 0,&RGBdesktop_0,NULL);
         //StretchBlt(RGBLED_DIALOG.hdc_mem, rc.x, rc.y, rc.w, rc.h, hdc_bk, 0, 0,RGBdesktop_0.Width, RGBdesktop_0.Height, SRCCOPY);               
         SetColorValue(leddlg_S.led_R, leddlg_S.led_G, leddlg_S.led_B);
         //GUI_DEBUG("%x%x", leddlg_S.led_R/16, leddlg_S.led_R%16);
         break;
      }

      case WM_NOTIFY: 
      {
         NMHDR *nr;
         WCHAR wbuf[128];
         u16 ctr_id; 
         u16 code,  id;
         id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
         code=HIWORD(wParam);//��ȡ��Ϣ������
         ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID.
         nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.
         if(id == ID_EXIT && code == BN_CLICKED)
         {
            PostCloseMessage(hwnd);
         }
         
         if (ctr_id == ID_SCROLLBAR_R)
         {
            NM_SCROLLBAR *sb_nr;		
            sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
            switch (nr->code)
            {
               case SBN_THUMBTRACK: //R�����ƶ�
               {
                  leddlg_S.col_R = sb_nr->nTrackValue; //��û��鵱ǰλ��ֵ
                  SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, leddlg_S.col_R); //����λ��ֵ
                  x_wsprintf(wbuf, L"%d", leddlg_S.col_R);
                  SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_R_NUM), wbuf);
                  RedrawWindow(GetDlgItem(hwnd, ID_TEXTBOX_R_LED), NULL, RDW_ALLCHILDREN|RDW_INVALIDATE);
               }
               break;
            }
         }
         
         if (ctr_id == ID_SCROLLBAR_G)
         {
            NM_SCROLLBAR *sb_nr;
            sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
            switch (nr->code)
            {
               case SBN_THUMBTRACK: //G�����ƶ�
               {
                  leddlg_S.col_G = sb_nr->nTrackValue; //��û��鵱ǰλ��ֵ
                  SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, leddlg_S.col_G); //����λ��ֵ
                  x_wsprintf(wbuf, L"%d", leddlg_S.col_G);
                  SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_G_NUM), wbuf);
                  RedrawWindow(GetDlgItem(hwnd, ID_TEXTBOX_G_LED), NULL, RDW_ALLCHILDREN|RDW_INVALIDATE);
               }
               break;
            }
         }

         if(ctr_id == ID_SCROLLBAR_B)
         {
            NM_SCROLLBAR *sb_nr;
            sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
            switch (nr->code)
            {
               case SBN_THUMBTRACK: //B�����ƶ�
               {
                  leddlg_S.col_B = sb_nr->nTrackValue; //���B���鵱ǰλ��ֵ
                  SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, leddlg_S.col_B); //����B�����λ��
                  x_wsprintf(wbuf, L"%d", leddlg_S.col_B);
                  InvalidateRect(GetDlgItem(hwnd, ID_TEXTBOX_B_NUM), NULL, TRUE);
                  SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_B_NUM), wbuf);
                  InvalidateRect(GetDlgItem(hwnd, ID_TEXTBOX_B_LED), NULL, TRUE);
                  
                  //RedrawWindow(GetDlgItem(hwnd, ID_TEXTBOX_B_LED), NULL, RDW_ALLCHILDREN|RDW_INVALIDATE);
               }
               break;
            }
         }
        if(id == ID_CHECKBOX_SW)
        {
          if(code == BN_CLICKED) //�������
          {
                 RGBLED_DIALOG.State = !RGBLED_DIALOG.State;
                 if(RGBLED_DIALOG.State==FALSE)
                 {
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B_NUM), SW_HIDE); 
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R_NUM), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G_NUM), SW_HIDE);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_R), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_G), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_B), SW_HIDE);
                   
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_Title), SW_HIDE);                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_APP), SW_HIDE);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R_LED), SW_HIDE); 
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G_LED), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B_LED), SW_HIDE);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G), SW_HIDE);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R), SW_HIDE);
//                   
//                   
//                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_Title), SW_HIDE);                   
//                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_APP), SW_HIDE);                   
                   
                 }
                 else
                 {
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B_NUM), SW_SHOW); 
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R_NUM), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G_NUM), SW_SHOW);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_R), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_G), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_SCROLLBAR_B), SW_SHOW);
                   
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_Title), SW_SHOW);                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_APP), SW_SHOW);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R_LED), SW_SHOW); 
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G_LED), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B_LED), SW_SHOW);
                   
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_B), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_G), SW_SHOW);
                   ShowWindow(GetDlgItem(hwnd, ID_TEXTBOX_R), SW_SHOW);                    
                 }
                 RedrawWindow(hwnd, NULL, RDW_ALLCHILDREN|RDW_INVALIDATE|RDW_ERASE);
          }

        }     
         
         RedrawWindow(GetDlgItem(hwnd, ID_TEXTBOX_APP), NULL, RDW_ALLCHILDREN|RDW_INVALIDATE);
         if(RGBLED_DIALOG.State == TRUE)
         {
            leddlg_S.led_R=leddlg_S.col_R;
            leddlg_S.led_G=leddlg_S.col_G;
            leddlg_S.led_B=leddlg_S.col_B;
            SetColorValue(leddlg_S.led_R, leddlg_S.led_G, leddlg_S.led_B);
         }
         else
         {
            SetColorValue(0, 0, 0);
         }
         break;
      }
      case WM_DRAWITEM:
      {
         DRAWITEM_HDR *ds;
         ds = (DRAWITEM_HDR*)lParam;

         rc = ds->rc;
         if(ds->ID == ID_TEXTBOX_R || ds->ID == ID_TEXTBOX_G || ds->ID ==ID_TEXTBOX_B)
         {
            GUI_RGBLEDButton_OwnerDraw(ds);
            return TRUE;
         }
         switch(ds->ID)
         {
            case ID_EXIT:
            {              
               GUI_RGBLED_HomeOwnerDraw(ds);

               return TRUE;              
            }
            case ID_SCROLLBAR_R:
            {
               GUI_RGBLED_ScrollbarOwnerDraw_V(ds);
               return TRUE;
            }
            case ID_SCROLLBAR_G:
            {
               GUI_RGBLED_ScrollbarOwnerDraw_V(ds);
               return TRUE;
            }
            case ID_SCROLLBAR_B:
            {
               GUI_RGBLED_ScrollbarOwnerDraw_V(ds);
               return TRUE;
            }
            case ID_CHECKBOX_SW:
            {
               GUI_RGBLED_CheckBoxOwnerDraw(ds);
               return TRUE;
            }
            case ID_TEXTBOX_R_NUM:
            {
               GUI_RGBLEDButton_OwnerDraw(ds);
               return TRUE;
            }
            case ID_TEXTBOX_G_NUM:
            {
               GUI_RGBLEDButton_OwnerDraw(ds);
               return TRUE;
            }
            case ID_TEXTBOX_B_NUM:
            {
               GUI_RGBLEDButton_OwnerDraw(ds);
               return TRUE;
            }            
         }
         break;

      }
//      case WM_PAINT:
//      {
//         HDC hdc;
//         PAINTSTRUCT ps;
//         RECT rc;

//         hdc = BeginPaint(hwnd, &ps);

//         GetClientRect(hwnd, &rc);
////         BitBlt(hdc, 0, 0, 800, 480, hdc_bk, 0, 0, SRCCOPY);

//         EndPaint(hwnd, &ps);
//         break;
//      }      
      case WM_CTLCOLOR:
      {
         u16 id;
         id =LOWORD(wParam);         
         CTLCOLOR *cr;
         cr =(CTLCOLOR*)lParam;
         
         switch(id)
         {
            case ID_TEXTBOX_APP:
            {
               cr->BackColor = ID_TEXTBOX_APP_BackColor; 
               //cr->BorderColor = ID_TEXTBOX_APP_BackColor;
               cr->TextColor = RGB888(leddlg_S.col_R, leddlg_S.col_G, leddlg_S.col_B);
               break;
            }
            case ID_TEXTBOX_R_LED:
            {
               cr->TextColor = RGB888(leddlg_S.col_R, 0, 0);
               cr->BackColor = ID_TEXTBOX_R_LED_BackColor;     
               break;
            }
            case ID_TEXTBOX_G_LED:
            {
               cr->TextColor = RGB888(0, leddlg_S.col_G, 0);
               cr->BackColor = ID_TEXTBOX_G_LED_BackColor;     
               break;
            }
            case ID_TEXTBOX_B_LED:
            {
               cr->TextColor = RGB888(0, 0, leddlg_S.col_B);
               cr->BackColor = ID_TEXTBOX_B_LED_BackColor;     
               break;
            }              
            default:
               return FALSE;
         }
         
         
         
         return TRUE;
         
      } 
      case WM_ERASEBKGND:
      {
         HDC hdc =(HDC)wParam;
         RECT rc =*(RECT*)lParam;
         
         if(RGBLED_DIALOG.State!=FALSE)
            BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, RGBLED_DIALOG.hdc_mem, rc.x, rc.y, SRCCOPY);
         else
         {
            SetBrushColor(hdc, MapRGB(hdc, 0,0,0));
            FillRect(hdc, &rc);             
         }
    


         return TRUE;

      } 
      case WM_DESTROY:
      {        
         Delete_DlALOG();
         DeleteDC(hdc_bk);
         DeleteSurface(pSurfTop);
         return PostQuitMessage(hwnd);	
      }          
      default:
         return	DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return	WM_NULL;
}


void	GUI_LED_DIALOG(void)
{
	HWND	hwnd;
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
   //��ʼ����ʱ��
   //LED_GPIO_Config();
   TIM_GPIO_Config();
   TIM_Mode_Config();
   //SetColorValue(leddlg_S.col_R, leddlg_S.col_R, leddlg_S.col_R);
   
	//����������
	hwnd = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                        &wcex,
                        L"GUI_LED_DIALOG",
                        WS_CLIPCHILDREN,
                        0, 0, GUI_XSIZE, GUI_YSIZE,
                        NULL, NULL, NULL, NULL);
   //��ʾ������
	ShowWindow(hwnd, SW_SHOW);
	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}
