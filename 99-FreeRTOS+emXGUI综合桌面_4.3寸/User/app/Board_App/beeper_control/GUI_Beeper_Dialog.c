#include <emXGUI.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "ff.h"
#include "x_libc.h"
#include "GUI_AppDef.h"
#include "emXGUI_JPEG.h"
#include "emxgui_png.h"
#include "./pic_load/gui_pic_load.h"

/* ��ť ID */
enum 
{
  ID_BEEPER_TICK = 0x1000,    // �������δ�ť
  ID_BEEPER_SW,               // ���������س�����ť
  ID_BEEPER_PRESS,            // ��������ס�찴ť
  ID_BEEPER_EXIT,             // �������˳���ť
};

static uint8_t sw_flag    = 0;
static uint8_t tick_flag  = 0;
static uint8_t press_flag = 3;

//�˳���ť�ػ���
static void ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
{
   HDC hdc;
   RECT rc;

   hdc = ds->hDC;   
   rc = ds->rc; 

   if (ds->State & BST_PUSHED)
   { //��ť�ǰ���״̬
      SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));      //��������ɫ
   }
   else
   { //��ť�ǵ���״̬
      SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));
   }
   
   rc.w = 25;
   OffsetRect(&rc, 0, 5);

   for(int i=0; i<4; i++)
   {
      HLine(hdc, rc.x, rc.y, rc.w);
      rc.y += 6;
   }
}

/*
 * @brief  ѡ��ť
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void sw_button_OwnerDraw(DRAWITEM_HDR *ds)
{
  HWND hwnd;
	HDC hdc;
  RECT rc, rc_tmp;
	WCHAR wbuf[128];

  hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
  rc.h = 51;
  if (ds->ID == ID_BEEPER_SW)
  {
    if (sw_flag == 0)
    {
      BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_png[hdc_beeper_button], 0, 0, SRCCOPY);
      DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
    }
    else 
    {
      rc.y = ds->rc.h - rc.h;
      BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_png[hdc_beeper_button], 0, 0, SRCCOPY);

      DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
    }
  }
  else if (ds->ID == ID_BEEPER_TICK)
  {
    if (tick_flag == 0)
    {
      BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_png[hdc_beeper_button], 0, 0, SRCCOPY);
      DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
    }
    else 
    {
      rc.y = ds->rc.h - rc.h;
      BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_png[hdc_beeper_button], 0, 0, SRCCOPY);
      
      DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);    // ��������(���ж��뷽ʽ)
    }
  }
}

/*
 * @brief  �м䰴ס��ť
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void press_button_OwnerDraw(DRAWITEM_HDR *ds)
{
  HWND hwnd;
	HDC hdc;
  RECT rc, rc_tmp;

  hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

	/* ���� */
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_beeper_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  BitBlt(hdc, 34, 48, 62, 115, hdc_beeper_png[hdc_beeper_horn], 0, 0, SRCCOPY);
  
  switch (press_flag)
  {
    case 3:
      BitBlt(hdc, 125, 40, 44, 131, hdc_beeper_png[hdc_beeper_horn3], 0, 0, SRCCOPY);
    
    case 2:
      BitBlt(hdc, 115, 61, 32, 90, hdc_beeper_png[hdc_beeper_horn2], 0, 0, SRCCOPY);

    case 1:
      BitBlt(hdc, 105, 83, 20, 45, hdc_beeper_png[hdc_beeper_horn1], 0, 0, SRCCOPY);
  
  default:
    break;
  }
}

static LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
      RECT rc;
      GetClientRect(hwnd, &rc); 
            
      CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY | WS_OWNERDRAW | WS_VISIBLE,
                  444, 0, 36, 30, hwnd, ID_BEEPER_EXIT, NULL, NULL); 

      CreateWindow(BUTTON, L"�δ�", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY | WS_OWNERDRAW | WS_VISIBLE,
                  43, 98, 51, 94, hwnd, ID_BEEPER_TICK, NULL, NULL); 

      CreateWindow(BUTTON, L"��ס��", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY | WS_OWNERDRAW | WS_VISIBLE,
                  137, 46, 206, 206, hwnd, ID_BEEPER_PRESS, NULL, NULL); 

      CreateWindow(BUTTON, L"����", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY | WS_OWNERDRAW | WS_VISIBLE,
                  390, 98, 51, 94, hwnd, ID_BEEPER_SW, NULL, NULL); 


      break;
    } 
    case WM_TIMER:
    {
      static uint8_t beep_flag = 0;

      beep_flag = !beep_flag;

      if (beep_flag)
      {
        BEEP_ON;
      }
      else
      {
        BEEP_OFF;
      }

      press_flag++;
      press_flag %= 4;

      RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_PRESS), NULL, TRUE);
      break;
    }

    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      hdc = BeginPaint(hwnd, &ps);
      
      BitBlt(hdc, 0, 0, GUI_XSIZE, GUI_YSIZE, hdc_beeper_bk, 0, 0, SRCCOPY);


      EndPaint(hwnd, &ps);
      break;
    }

    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case ID_BEEPER_EXIT:
          {
            ExitButton_OwnerDraw(ds);
            return TRUE;             
          }  

          case ID_BEEPER_SW:
          case ID_BEEPER_TICK:
          {
            sw_button_OwnerDraw(ds);
            return TRUE;   
          }

          case ID_BEEPER_PRESS:
          {
            press_button_OwnerDraw(ds);
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
      if(code == BN_CLICKED)
      {
        if (id == ID_BEEPER_EXIT)
        {
          PostCloseMessage(hwnd);
        }
        else if (id == ID_BEEPER_TICK)
        {
          tick_flag = !tick_flag;    // ������־

          if (tick_flag)
          {
            SetTimer(hwnd, 1, 500, TMR_START, NULL);    // �����δ�ʱ��
          }
          else
          {
            KillTimer(hwnd, 1);    // �رյδ�ʱ��
            BEEP_OFF;              // �رշ�����
            press_flag =3;
          }
          
          sw_flag = 0;    // ��λ��־
          RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_SW), NULL, TRUE);
          RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_PRESS), NULL, TRUE);
        }
        else if (id == ID_BEEPER_SW)
        {
          sw_flag = !sw_flag;    // ������־

          if (sw_flag)
          {
            BEEP_ON;    // ��������
          }
          else
          { 
            BEEP_OFF;    // �ط����� 
          }

          KillTimer(hwnd, 1);
          tick_flag = 0;
          press_flag = 3;

          RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_TICK), NULL, TRUE);
          RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_PRESS), NULL, TRUE);
        }
        else if (id == ID_BEEPER_PRESS)
        {
          KillTimer(hwnd, 1);
          BEEP_OFF;    // �ط����� 
          press_flag = 3;
          RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_PRESS), NULL, TRUE);
        }
      }
      
      if(code == BN_PUSHED && id == ID_BEEPER_PRESS)    // ����
      {
        tick_flag = 0;
        sw_flag = 0;

        RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_TICK), NULL, TRUE);
        RedrawWindow(GetDlgItem(hwnd, ID_BEEPER_SW), NULL, TRUE);
        SetTimer(hwnd, 1, 500, TMR_START, NULL);
      }

      break;
    } 

    case WM_DESTROY:
    {
      tick_flag = 0;
      sw_flag = 0;
      press_flag = 3;
      
      BEEP_OFF;
      
      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
  
}

void GUI_Beeper_Dialog(void)
{
	
	WNDCLASS	wcex;
	MSG msg;
  HWND MAIN_Handle;
	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = win_proc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   
	//����������
	MAIN_Handle = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                              &wcex,
                              L"GUI_Beeper_Dialog",
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


