#include <emXGUI.h>
#include <string.h>
#include "Widget.h"
#include "./camera/bsp_ov5640.h"
#include "x_libc.h"
#include "./camera/ov5640_AF.h"
#include "qr_decoder_user.h"
#include "GUI_AppDef.h"

extern BOOL g_dma2d_en;//DMA2Dʹ�ܱ�־λ������ͷDMEO�����ֹ
extern OV5640_IDTypeDef OV5640_Camera_ID;
extern HWND Cam_hwnd;//�����ھ��
extern HWND SetWIN;//�������ô���
extern int state;//��ʼ������ͷ״̬��
//extern uint16_t *cam_buff0;
//extern uint16_t *cam_buff1;
extern GUI_SEM *cam_sem;//����ͼ��ͬ���ź�������ֵ�ͣ�
uint8_t QR_Task = 0;
TaskHandle_t QR_Task_Handle;
//����ؼ�ID
enum eID
{
	eID_OK =0x1000,
	eID_DText,
  eID_QR_EXIT,
};

/* �û��Զ�����Ϣ */
enum eMSG 
{
	eMSG_QRScan_OK = WM_USER + 1,
};

void ScanCompleteDialog(HWND hwndParent, RECT rc,const WCHAR *pText,const WCHAR *pCaption,const MSGBOX_OPTIONS *ops);


/*
 * @brief  ������Ļ
 * @param  NONE
 * @retval NONE
*/
//  rt_tick_t tick1 = 0;
//  rt_tick_t tick2 = 0;

static void Update_Dialog(void *p)
{
  RECT rc;
  
  rc.x = GUI_XSIZE/2 - 195/2;
  rc.y = GUI_YSIZE/2 - 195/2;
  rc.w = 195;
  rc.h = 195;
  
	while(QR_Task) //�߳��Ѵ�����
	{
    GUI_SemWait(cam_sem, 0xFFFFFFFF);
    InvalidateRect(Cam_hwnd, &rc, FALSE);
	}
  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}

static void QR_decoder_Task(void *p)
{
	while(QR_Task) //�߳��Ѵ�����
	{
    char  qr_type_len=0;
    short qr_data_len=0;
    char qr_type_buf[10];
    char qr_data_buf[512];
    int addr=0;
    int i=0,j=0;
    char qr_num=0;
    qr_num = QR_decoder();
    if(qr_num)
    { 
      BEEP_ON;
      GUI_msleep(50);
      //����������ǰ���ʶ������ĸ�����װ�õĶ�ά���飬��Щ������Ҫ
      //����ʶ������ĸ�������������ͨ�����ڷ��͵���λ�������ն�
      qr_type_len = decoded_buf[i][addr++];//��ȡ�������ͳ���
       
      for(j=0;j < qr_type_len;j++)
        qr_type_buf[j]=decoded_buf[i][addr++];//��ȡ������������
      
      qr_type_buf[j] = '\0';
       
      qr_data_len  = decoded_buf[i][addr++]<<8; //��ȡ�������ݳ��ȸ�8λ
      qr_data_len |= decoded_buf[i][addr++];    //��ȡ�������ݳ��ȵ�8λ
       
      for(j=0;j < qr_data_len;j++)
        qr_data_buf[j]=decoded_buf[i][addr++];//��ȡ��������
      
      qr_data_buf[j] = '\0';

//      printf("���ͣ�%s\n���ݣ�%s\n", qr_type_buf, qr_data_buf);

      addr =0;//����
      
      WCHAR wbuf_type[40];
      WCHAR *wbuf_data = (WCHAR *)GUI_VMEM_Alloc(512 * sizeof(WCHAR));
      x_mbstowcs_cp936(wbuf_type, qr_type_buf, sizeof(wbuf_type));
      x_mbstowcs_cp936(wbuf_data, qr_data_buf, 512 * sizeof(WCHAR));

      PostAsyncMessage(Cam_hwnd, eMSG_QRScan_OK, (WPARAM)wbuf_data, (LPARAM)wbuf_type);     // ʶ����ɣ���ʾ���
      BEEP_OFF;
      
      vTaskSuspend(QR_Task_Handle);    // �����Լ� ����ִ��

      QR_decoder();     // �˳�ǰʶ��һ�Σ������һ֡
      GUI_VMEM_Free(wbuf_data);
      qr_num = 0;
    }
    
    GUI_msleep(10);
	}
  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}

/*=========================================================================================*/
static const WCHAR *pTextInt;
static const WCHAR *pCaptionInt;
static const MSGBOX_OPTIONS *opsInt;

static void DialogBoxButton_OwnerDraw(DRAWITEM_HDR *ds)
{
	HDC hdc;
	RECT rc;
	WCHAR wbuf[128];

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.
  
  if(ds->State&BST_PUSHED)
	{
    /* ������������ */
    SetBrushColor(hdc,MapRGB(hdc, 200, 200, 200));
	}
	else
	{
    /* ������������ */
    SetBrushColor(hdc,MapRGB(hdc, 250, 250, 250));
	}
  
  FillRoundRect(hdc, &rc, 7);
    
  SetPenColor(hdc,MapRGB(hdc, 200, 200, 200));    /* ����ɫ���� */
  HLine(hdc, rc.x, rc.y, rc.x+rc.w);
  if (ds->ID < opsInt->ButtonCount - 1)    /* ���һ����ť�����ұߵ�һ���� */
  {
    VLine(hdc, rc.x+rc.w-1, rc.y, rc.y+rc.h);
  }
  
  SetPenColor(hdc,MapRGB(hdc, 105, 105, 105));           // ���û��ʵ���ɫ
  GetWindowText(ds->hwnd, wbuf, 128);                    // ��ð�ť�ؼ�������
//  OffsetRect(&rc, 0, -3);
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
}

static LRESULT ScanCompleteWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    /* ���ڴ��� */
    case WM_CREATE:
    {
      RECT rc;
      RECT rc_Text;
      RECT rc_button;
//      int FontH;
//      FontH = GetFontAveHeight(defaultFont);
      GetClientRect(hwnd, &rc);

      for(uint8_t xC=0; xC<opsInt->ButtonCount; xC++)
      {
        /* ���㰴��λ�� ���߶�Ϊ���ڵ��ķ�֮һ�� */
        rc_button.w = rc.w / opsInt->ButtonCount;    // ��ť�Ŀ��
        rc_button.h = rc.h / 4;                      // ���ڸ߶ȵ� 1/4
        rc_button.x = rc_button.w * xC;
        rc_button.y = rc.h - rc_button.h;
        
        /* ����һ����ť */
        CreateWindow(BUTTON, opsInt->pButtonText[xC], BS_FLAT|WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW, //
                     rc_button.x, rc_button.y, rc_button.w, rc_button.h, hwnd, xC, NULL, NULL);
      }
      
       /* ������Ϣ���͵���ʾλ�� */
      rc_Text.w = rc.w;
      rc_Text.h = rc.h / 4 * 2;
      rc_Text.y = rc.y + rc.h / 4;
      rc_Text.x = rc.x;
      
      /* ��ʾ��Ϣ���� */
      CreateWindow(TEXTBOX, pTextInt, WS_VISIBLE, rc_Text.x, rc_Text.y, rc_Text.w, rc_Text.h, hwnd, eID_DText, NULL, NULL);
      SendMessage(GetDlgItem(hwnd, eID_DText), TBM_SET_TEXTFLAG, 0, DT_TOP | DT_LEFT | DT_BKGND | DT_WORDBREAK);
//      DrawText(hdc, , -1, &rc_Text, DT_VCENTER|DT_CENTER);
      
      break; 
    }
    
    case WM_ERASEBKGND:
    {
      return TRUE;    // ֱ�ӷ��� TRUE ��ⲻ�ửĬ�ϱ���
    }

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc;
      RECT rc;
      RECT rc_Text;

      GetClientRect(hwnd,&rc);
      hdc = BeginPaint(hwnd,&ps);
      
//      SetBrushColor(hdc,MapRGB(hdc,50,0,0));
//      FillRect(hdc, &rc);
      SetBrushColor(hdc, MapRGB(hdc,250,250,250));
      FillRoundRect(hdc, &rc, 7);
      
      /* ������Ϣ���͵���ʾλ�� */
      rc_Text.w = rc.w;
      rc_Text.h = rc.h / 4;
      rc_Text.x = rc.x;
      rc_Text.y = rc.y;
      
      /* ��ʾ��Ϣ���� */
      DrawText(hdc, pCaptionInt, -1, &rc_Text, DT_VCENTER|DT_CENTER);
      
      EndPaint(hwnd,&ps);
      break;
    }
    
    case WM_NOTIFY: 
    {
      u16 code;
      u16 id;
			id =LOWORD(wParam);
      code=HIWORD(wParam);//��ȡ��Ϣ������

      if(code == BN_CLICKED)
      {
        if (id == 0)
          PostCloseMessage(hwnd);
      }
      break;
     }
    
     case	WM_CTLCOLOR:
		{
			/* �ؼ��ڻ���ǰ���ᷢ�� WM_CTLCOLOR��������.
			 * wParam����ָ���˷��͸���Ϣ�Ŀؼ�ID;lParam����ָ��һ��CTLCOLOR�Ľṹ��ָ��.
			 * �û�����ͨ������ṹ��ı�ؼ�����ɫֵ.�û��޸���ɫ�������践��TRUE������ϵͳ
			 * �����Ա��β���������ʹ��Ĭ�ϵ���ɫ���л���.
			 *
			 */
			u16 id;
			id =LOWORD(wParam);
      /* �ı��򱳾�������ɫ���� */
			if(id== eID_DText)
			{
				CTLCOLOR *cr;
				cr =(CTLCOLOR*)lParam;
        cr->TextColor =RGB888(0,0,0);//������ɫ��RGB888��ɫ��ʽ)
				cr->BackColor =RGB888(250,250,250);//������ɫ��RGB888��ɫ��ʽ)
				return TRUE;
			}

        return FALSE;
		}
    
     case WM_DRAWITEM:
     {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       
       DialogBoxButton_OwnerDraw(ds);
       
       return TRUE;
     }
    
    case WM_DESTROY:
    {
      return PostQuitMessage(hwnd);
    }
    
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  
  return WM_NULL;
}

//�˳���ť�ػ���
static void QR_ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
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
		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));      //���û���ɫ
	}
	else
	{ //��ť�ǵ���״̬
		SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));
	}
  
  rc.w = 25;
  OffsetRect(&rc, 0, 12);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 6;
  }

}

void ScanCompleteDialog(HWND hwndParent, RECT rc,const WCHAR *pText,const WCHAR *pCaption,const MSGBOX_OPTIONS *ops)
{
  WNDCLASS	wcex;

  if(hwndParent==NULL)
	{
		hwndParent =GetDesktopWindow();
	}
  
  /* ��ʼ���ڲ����� */
  pTextInt = pText;
  pCaptionInt = pCaption;
  opsInt = ops;
  
	wcex.Tag = WNDCLASS_TAG;  
	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ScanCompleteWinProc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);

	DialogWindow(	&wcex,	pCaption,
//							WS_OVERLAPPED|WS_BORDER|WS_DLGFRAME|WS_CLIPCHILDREN,
                WS_CLIPCHILDREN,//WS_CAPTIONWS_BORDER|
							rc.x, rc.y, rc.w, rc.h,
							hwndParent, 0x0000, NULL, NULL);//hInst

// 	//��ʾ������
// 	ShowWindow(hwnd, SW_SHOW);
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
 * @brief  ����ͷ���ڻص�����
*/
extern int SelectDialogBox(HWND hwndParent, RECT *rc, const WCHAR *pText, const WCHAR *pCaption, const MSGBOX_OPTIONS *ops);
static LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//  static HDC hdc_mem_temp;
  
  switch(msg)
  {
    case WM_CREATE:
    {
      /* ��ʼ������ͷGPIO��IIC */
      OV5640_HW_Init();  
      /* ��ȡ����ͷоƬID��ȷ������ͷ�������� */
      OV5640_ReadID(&OV5640_Camera_ID);
      
       CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY |WS_OWNERDRAW|WS_VISIBLE,
                  444, 0, 36, 37, hwnd, eID_QR_EXIT, NULL, NULL); 

      if(OV5640_Camera_ID.PIDH  == 0x56)
      {
        SetTimer(hwnd, 1, 400, TMR_START | TMR_SINGLE, NULL);  
        GUI_DEBUG("OV5640 ID:%x %x",OV5640_Camera_ID.PIDH ,OV5640_Camera_ID.PIDL);
      }
      else
      {
        // MSGBOX_OPTIONS ops;
        // //const WCHAR *btn[]={L"ȷ��"};
        // int x,y,w,h;

        // ops.Flag =MB_ICONERROR;
        // //ops.pButtonText =btn;
        // ops.ButtonCount =0;
        // w =500;
        // h =200;
        // x =(GUI_XSIZE-w)>>1;
        // y =(GUI_YSIZE-h)>>1;
        // MessageBox(hwnd,x,y,w,h,L"û�м�⵽OV5640����ͷ��\n�����¼�����ӡ�",L"����",&ops); 
        // PostCloseMessage(hwnd);
        SetTimer(hwnd, 3, 3, TMR_START | TMR_SINGLE, NULL);      // ��ʼ������������ʾ
      }
      cam_sem = GUI_SemCreate(0,1);//ͬ������ͷͼ��
      GUI_DEBUG("cam_sem = %p", cam_sem);
      QR_Task = 1;
      
      xTaskCreate((TaskFunction_t )Update_Dialog,   /* ������ں��� */
                            (const char*    )"Update_Dialog",       /* �������� */
                            (uint16_t       )1*512/4,                 /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )NULL,                  /* ������ں������� */
                            (UBaseType_t    )5,                     /* ��������ȼ� */
                            (TaskHandle_t  )NULL);                  /* ������ƿ�ָ�� */
      
      xTaskCreate((TaskFunction_t )QR_decoder_Task,  /* ������ں��� */
                            (const char*    )"QR decoder Task",     /* �������� */
                            (uint16_t       )1024*5,              /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )NULL,                  /* ������ں������� */
                            (UBaseType_t    )4,                     /* ��������ȼ� */
                            (TaskHandle_t  )&QR_Task_Handle);        /* ������ƿ�ָ�� */

      // HDC hdc_mem_320;
      // hdc_mem_temp = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, GUI_XSIZE, GUI_YSIZE);
      // ClrDisplay(hdc_mem_temp, NULL, ARGB8888(100, 0, 0, 0));
        
      // hdc_mem_320 = CreateMemoryDC((SURF_FORMAT)COLOR_FORMAT_ARGB8888, 320, 320);
      // ClrDisplay(hdc_mem_320, NULL,  ARGB8888(1, 0, 0, 0));
        
      // BitBlt(hdc_mem_temp, GUI_XSIZE/2 - 320/2, GUI_YSIZE/2 - 320/2, 320, 320, hdc_mem_320, 0 , 0, SRCCOPY);
      // DeleteDC(hdc_mem_320);
      break;  
    }

    case WM_LBUTTONDOWN://�����Ļ���رմ���
    {

//      PostCloseMessage(hwnd);
  

      break;
    }
    case WM_TIMER://����ͷ״̬��
    {
      u16 id;
      
      id = LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.     
      if(id == 1)
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
//                focus_status = 1;
            }
            //ʹ��DCMI�ɼ�����
            DCMI_Cmd(ENABLE); 
            DCMI_CaptureCmd(ENABLE); 

            state = 1;
            InvalidateRect(hwnd, NULL, TRUE);
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
    
    case WM_ERASEBKGND:
    {
      RECT rc = {0, 0, GUI_XSIZE, GUI_YSIZE};
      HDC hdc =(HDC)wParam;
      
      SetBrushColor(hdc,MapRGB(hdc,220,220,220));
      FillRect(hdc, &rc);
      
      rc.x = GUI_XSIZE/2 - 195/2;
      rc.y = GUI_YSIZE/2 - 195/2 - 40;
      rc.w = 195;
      rc.h = 40;
      SetTextColor(hdc,MapRGB(hdc,50,50,50));
      DrawText(hdc, L"��ά��ʶ��", -1, &rc, DT_VCENTER|DT_CENTER);
      return TRUE;
    }
    
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      SURFACE *pSurf;
      HDC hdc_mem;
      HDC hdc;
      RECT rc;
      // RECT rc_title = {100, 0, 600, 70};

      hdc = BeginPaint(hwnd,&ps);
      GetClientRect(hwnd,&rc);
      if(state==0)
      {
        SetTextColor(hdc,MapRGB(hdc,50,50,50));
        SetBrushColor(hdc,MapRGB(hdc,220,220,220));
        SetPenColor(hdc,MapRGB(hdc,250,0,0));
        
        DrawText(hdc,L"���ڳ�ʼ������ͷ\r\n\n��ȴ�...",-1,&rc,DT_VCENTER|DT_CENTER|DT_BKGND);
      }  
      else if (state==1)
      {   

        pSurf =CreateSurface(SURF_RGB565,cam_mode.cam_out_width, cam_mode.cam_out_height, 0, (U16*)cam_buff00);     
        
        hdc_mem =CreateDC(pSurf,NULL);
        
        SetPenColor(hdc_mem, MapRGB(hdc,250,0,0));
        rc.x = GUI_XSIZE/2 - 195/2;
        rc.y = GUI_YSIZE/2 - 195/2;
        rc.w = 195;
        rc.h = 195;
        DrawRect(hdc_mem, &rc);

//        BitBlt(hdc_mem, 0, 0, GUI_XSIZE, GUI_YSIZE, hdc_mem_temp, 0 , 0, SRCCOPY);
        
//        BitBlt(hdc,  cam_mode.lcd_sx , cam_mode.lcd_sy, cam_mode.cam_out_width,  
//               cam_mode.cam_out_height, hdc_mem, rc.x, rc.y, SRCCOPY);
        BitBlt(hdc,  rc.x, rc.y, rc.w, rc.h, hdc_mem, rc.x, rc.y, SRCCOPY);

        // SetTextColor(hdc,MapRGB(hdc,10,10,10));
        // DrawText(hdc, L"��ά��ʶ��", -1, &rc_title, DT_CENTER|DT_VCENTER); 

        DeleteSurface(pSurf);
        DeleteDC(hdc_mem);
      }
      
      EndPaint(hwnd,&ps);
      
        
      OV5640_Capture_Control(ENABLE);//�ر�����ͷ�ɼ�ͼ��
      DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE); //�ر�DMA�ж�
      DCMI_Cmd(ENABLE);                               //DCMIʧ��
      DCMI_CaptureCmd(ENABLE); 
      DCMI_Stop();
      HAL_DCMI_Start_DMA((uint32_t)cam_buff00,
                        cam_mode.cam_out_height*cam_mode.cam_out_width/2);
      DCMI_Start();
      break;
    }
    
    case eMSG_QRScan_OK:
    {
      WCHAR *wbuf_type = (WCHAR *)lParam; 
      WCHAR *wbuf_data = (WCHAR *)wParam; 
      
      RECT RC;
      MSGBOX_OPTIONS ops;
      const WCHAR *btn[] ={L"ȷ��"};      //�Ի����ڰ�ť������

      ops.pButtonText =btn;
      ops.ButtonCount =1;
      RC.w = 300;
      RC.h = 200;
      RC.x = (GUI_XSIZE - RC.w) >> 1;
      RC.y = (GUI_YSIZE - RC.h) >> 1;
      
      ScanCompleteDialog(Cam_hwnd, RC, wbuf_data, wbuf_type, &ops);    // ��ʾʶ�������
      vTaskResume(QR_Task_Handle);    // �ָ�ʶ������
      
      break;
    }
    
    case WM_DESTROY:
    {
      state = 0;
      //DeleteDC(hdc_mem_temp);
      OV5640_Reset();//��λ����ͷ
      OV5640_Capture_Control(DISABLE);//�ر�����ͷ�ɼ�ͼ��
      DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,DISABLE); //�ر�DMA�ж�
      DCMI_Cmd(DISABLE); //DCMIʧ��
      DCMI_CaptureCmd(DISABLE); 
      if (QR_Task)
      {
        GUI_SemDelete(cam_sem);
      }
      QR_Task=0;
//      GUI_VMEM_Free(cam_buff1);
//      GUI_VMEM_Free(cam_buff0);
      //��λ����ͷ���ò���
      Camera_ReConfig();
      cur_index = 0;
      //LCD_LayerCamInit((uint32_t)LCD_FRAME_BUFFER,800, 480);
      return PostQuitMessage(hwnd);	
    }    
    
    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_QR_EXIT:
          {
            QR_ExitButton_OwnerDraw(ds);
            return TRUE;             
          }
       }

       break;
    }
    
    case WM_NOTIFY: //WM_NOTIFY��Ϣ:wParam��16λΪ���͸���Ϣ�Ŀؼ�ID,��16λΪ֪ͨ��;lParamָ����һ��NMHDR�ṹ��.
    {
      u16 code,id;
      code =HIWORD(wParam); //���֪ͨ������.
      id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.

      if(id==eID_QR_EXIT && code==BN_CLICKED)//�˳�����
      {
        PostCloseMessage(hwnd);
      }
      break;  
    }
    
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return WM_NULL;
}


void	GUI_Camera_QRCode_DIALOG(void)
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
                                    L"GUI_Camera_QRCode_Dialog",
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
