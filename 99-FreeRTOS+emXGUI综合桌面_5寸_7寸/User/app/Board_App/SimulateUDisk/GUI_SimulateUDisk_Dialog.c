#include <emXGUI.h>
#include <string.h>
#include <stdio.h>
#include "x_libc.h"
#include "GUI_AppDef.h"
#include "GUI_SimulateUDisk_Dialog.h"
#include "emXGUI_JPEG.h"
#include "emxgui_png.h"

#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usb_bsp.h"

#include "./pic_load/gui_pic_load.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_dev __ALIGN_END ;

//�˳���ť�ػ���
static void _ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
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

static LRESULT	win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
      RECT rc;
      GetClientRect(hwnd, &rc);
                      
      CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY |WS_OWNERDRAW|WS_VISIBLE,
                  740, 22, 36, 36, hwnd, eID_SUD_EXIT, NULL, NULL);

      CreateWindow(BUTTON, L"����", WS_TRANSPARENT| BS_NOTIFY | WS_VISIBLE | BS_3D|WS_OWNERDRAW,
                  318, 390, 166,  70, hwnd, eID_SUD_LINK, NULL, NULL);    // ʹ��ʱ�ӵİ�ť����
      
    //   BOOL res;
    //   u8 *jpeg_buf;
    //   u32 jpeg_size;
    //   JPG_DEC *dec;
    //   // res = RES_Load_Content(GUI_SUD_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
    //   res = FS_Load_Content(GUI_SUD_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
    //   bk_hdc = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
    //   if(res)
    //   {
    //     /* ����ͼƬ���ݴ���JPG_DEC��� */
    //     dec = JPG_Open(jpeg_buf, jpeg_size);

    //     /* �������ڴ���� */
    //     JPG_Draw(bk_hdc, 0, 0, dec);

    //     /* �ر�JPG_DEC��� */
    //     JPG_Close(dec);
    //   }
    //   /* �ͷ�ͼƬ���ݿռ� */
    //   RES_Release_Content((char **)&jpeg_buf);

      break;
    } 
    case WM_TIMER:
    {
      int tmr_id;

      tmr_id = wParam;    // ��ʱ�� ID

      if (tmr_id == 10)    
      {
        
      }
      
      break;
    }

    case WM_ERASEBKGND:
    {
      HDC hdc = (HDC)wParam;
      RECT rc =*(RECT*)lParam;

      // GetClientRect(hwnd, &rc);
      // SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
      // FillRect(hdc, &rc);
      
      BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_clock_bk, rc.x, rc.y, SRCCOPY);    // ʹ����ʱ��APP��ͬ�ı���

      return TRUE;
    }

    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      RECT rc  = {0, 80, GUI_XSIZE, 330};
      RECT rc1 = {100, 0, 600, 80};

      hdc = BeginPaint(hwnd, &ps);
      
      SetFont(hdc, defaultFont); 
      SetTextColor(hdc, MapRGB(hdc, 250, 250, 250));
      DrawText(hdc, L"�ⲿFLASHģ��U��", -1, &rc1, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
      SetTextInterval(hdc, -1, 30);
      DrawText(hdc, L"��Ӧ��ʹ���ⲿFLASH�ĺ�10Mģ��U��\r\n���ڵ������ǰʹ��Micro USB\r\n���������ӿ������J24�����ԣ�", -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
   
      EndPaint(hwnd, &ps);

      break;
    }

    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_SUD_EXIT:
          {
            _ExitButton_OwnerDraw(ds);
            return TRUE;              
          }  

          case eID_SUD_LINK:
          {
            btn_owner_draw(ds);
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

      switch(id)
       {
        /* �˳���ť���� */
          case eID_SUD_EXIT:
          {
               switch(code)
                {
                    case BN_CLICKED:
                    {
                        PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
                    }  
                    break;
                }
          }
          break;

          /* ���Ӱ�ť���� */
          case eID_SUD_LINK:
          {
               switch(code)
                {
                    case BN_CLICKED:
                    {
                        /*!< At this stage the microcontroller clock setting is already configured, 
                        this is done through SystemInit() function which is called from startup
                        file (startup_stm32fxxx_xx.s) before to branch to application main.
                        To reconfigure the default setting of SystemInit() function, refer to
                        system_stm32fxxx.c file
                        */       
                        USBD_Init(&USB_OTG_dev,
                                  USB_OTG_HS_CORE_ID,
                                  &USR_desc,
                                  &USBD_MSC_cb, 
                                  &USR_cb);
                      
                      SetWindowText(GetDlgItem(hwnd, eID_SUD_LINK), L"������");
                      EnableWindow(GetDlgItem(hwnd, eID_SUD_LINK), FALSE);
                    }  
                    break;
                }
          }
          break;
       }
      
      break;
    } 

    case WM_DESTROY:
    { 
      // DeleteDC(bk_hdc);
      DCD_DevDisconnect(&USB_OTG_dev);
      USB_OTG_STOP();
      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
  
}

void GUI_SimulateUDisk_Dialog(void)
{
	HWND SUD_Main_Handle;
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
	SUD_Main_Handle = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                              &wcex,
                              L"GUI Simulate U Disk Dialog",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0, 0, GUI_XSIZE, GUI_YSIZE,
                              NULL, NULL, NULL, NULL);

   //��ʾ������
	ShowWindow(SUD_Main_Handle, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, SUD_Main_Handle))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


