/**
  *********************************************************************
  * @file    desktop.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   emXGUI ����
  *********************************************************************
  * ʵ��ƽ̨:Ұ�� F429-��ս�� STM32 ������
  * ����    :www.embedfire.com
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 


#include	"rtthread.h"
#include "emXGUI.h"





/*===================================================================================*/
extern HWND	slideWin_hwnd;

extern void GUI_AppMain(void);
extern void	GUI_DEMO_SlideWindow(void);

#define INFO_HEAD_HEIGHT   70

#define WM_MSG_SLIDE  (WM_USER + 10)


static	void	gui_app_thread(void *p)
{
    #if(GUI_TOUCHSCREEN_EN & GUI_TOUCHSCREEN_CALIBRATE)
    {
        int i=0;
        while(TouchPanel_IsPenDown())
    	{
    		GUI_msleep(100);
    		if(i++>10)
    		{
    			ShowCursor(FALSE);
    			TouchScreenCalibrate(NULL);
    			ShowCursor(TRUE);
    			break;
    		}
    	}
    }
    #endif
		
  /* ����APP���� */  
	GUI_AppMain();
 //   GUI_UserAppStart();
//   	ShellWindowStartup();
  //  return 0;
}

/*===================================================================================*/

/**
  * @brief  ���汳�����ƺ�����������������ʱ����ñ�������
            ͨ���޸ı����������ݿɸ�������ı���
  * @param  hdc ��ͼ������
  * @param  lprc Ҫ���Ƶľ�������ΪNULLʱ���Զ�����hwnd�Ŀͻ���
  * @param  hwnd ���ڶ�����
  * @retval ��
  */
static	void	_EraseBackgnd(HDC hdc,const RECT *lprc,HWND hwnd)
{
	RECT rc;

	if(lprc==NULL)
	{
		GetClientRect(hwnd,&rc);
	}
	else
	{
		CopyRect(&rc,lprc);
	}

	SetBrushColor(hdc,MapRGB(hdc,215,61,50));
	FillRect(hdc,&rc);
  	
  SetTextColor(hdc,MapRGB(hdc,255,255,255));
  
  SetTextColor(hdc,MapRGB(hdc,250,250,250));
  DrawText(hdc,L"emXGUI@Embedfire STM32F429 ",-1,&rc,DT_CENTER);

  
//  SetFont(hdc, iconFont);
//	DrawText(hdc,L" A B C D E \r\n F G H I J",-1,&rc,DT_LEFT|DT_VCENTER);

//  SetFont(hdc, defaultFont);
  
  GetClientRect(hwnd,&rc);
  SetBrushColor(hdc,MapRGB(hdc,82,85,82));
  rc.y = GUI_YSIZE - INFO_HEAD_HEIGHT;
  rc.h = INFO_HEAD_HEIGHT;
  FillRect(hdc,&rc);
  
  /* ���� */ 
  SetFont(hdc, logoFont); 
  /* ��ʾlogo */
//  GetClientRect(hwnd,&rc);
  rc.y += 5;        
  SetTextColor(hdc,MapRGB(hdc,250,0,0)); 
  DrawText(hdc,L" B",-1,&rc,DT_LEFT);
  
  /* �ָ�Ĭ������ */
  SetFont(hdc, defaultFont);

  GetClientRect(hwnd,&rc);
  rc.y = GUI_YSIZE - INFO_HEAD_HEIGHT;
    rc.h = INFO_HEAD_HEIGHT;
     
  SetTextColor(hdc,MapRGB(hdc,250,250,250)); 
  DrawText(hdc,L"��",-1,&rc,DT_CENTER);
  
  rc.x +=60;
  rc.y +=10;
  DrawText(hdc,L" Ұ��\r\n emXGUI",-1,&rc,DT_LEFT);

  GetClientRect(hwnd,&rc);
  rc.y = GUI_YSIZE - INFO_HEAD_HEIGHT;
  rc.h = INFO_HEAD_HEIGHT;

  rc.y += 10;
  DrawText(hdc,L"\r\n��ϸ",-1,&rc,DT_CENTER);
  DrawText(hdc,L"\r\nwww.embedFire.com  ",-1,&rc,DT_RIGHT);  

  
}

#if 0
static	int	gui_input_thread(void *p)
{
	SYS_thread_set_priority(NULL,+4);
	while(1)
	{
		GUI_InputHandler(); //���������豸
		GUI_msleep(20);
	}
}
#endif


/**
  * @brief  ����ص�����
  * @param  hwnd ��ǰ�������Ϣ�Ĵ��ڶ�����
  * @param  msg ��Ϣ����ֵ�����Ա�ʶ�����ֵ�ǰ��������Ϣ
  * @param  wParam ��Ϣ����ֵ������msg��Ϣ����ֵ��ͬ
  * @param  lParam ��Ϣ����ֵ������msg��Ϣ����ֵ��ͬ
  * @retval ���ظ�SendMessage��ֵ
  */
static 	 LRESULT  	desktop_proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    static int win_pos=0;

	switch(msg)
	{
    /* ���洴��ʱ,���������Ϣ,������������һЩ��ʼ������. */
		case	WM_CREATE:	


			   ////����1��20ms��ʱ��������ѭ���¼�.
				 SetTimer(hwnd,1,20,TMR_START,NULL);

				//����App�߳�						
				if(1)
				{
						rt_thread_t h;
					
						h=rt_thread_create("GUI_APP",gui_app_thread,NULL,8192,5,5);
						rt_thread_startup(h);		

						h=rt_thread_create("GUI_slide",GUI_DEMO_SlideWindow,NULL,8192,5,5);
						rt_thread_startup(h);				
          
				}

				break;

		/* ��ʱ���������豸����Ϣ */
		case	WM_TIMER:
      #if(GUI_INPUT_DEV_EN)
        {
          u16 id;

          id =LOWORD(wParam);
          if(id==1)
          {
            GUI_InputHandler(); //���������豸
          }
        }
      #endif
		break;

    /* �ͻ���������Ҫ������ */
		case	WM_ERASEBKGND:
		{
			HDC hdc =(HDC)wParam;
			_EraseBackgnd(hdc,NULL,hwnd);
		}
		return TRUE;
    
    case WM_LBUTTONDOWN:
		{

			POINT pt;
			RECT rc;

			pt.x =GET_LPARAM_X(lParam);
			pt.y =GET_LPARAM_Y(lParam);

			GetClientRect(hwnd,&rc);
      GUI_DEBUG("pt.x =%d,pt.y=%d",pt.x,pt.y);

      if(win_pos == 0)
      {
        rc.y = GUI_YSIZE - INFO_HEAD_HEIGHT;
        rc.h = INFO_HEAD_HEIGHT;
      }

			if(PtInRect(&rc,&pt))
			{
//        KillTimer(hwnd,2);
//        SetTimer(hwnd,2,5,TMR_START,NULL);

        /* ������Ϣ���������� */
				if(win_pos==0)
				{
					win_pos =1;          
				}
				else
				{
					win_pos =0;
				}
        
        if(slideWin_hwnd)
        {
          SendMessage(slideWin_hwnd,WM_MSG_SLIDE,win_pos,0); 
          GUI_DEBUG("SendMesage");

        }
			}
		}
		break;

    /* �û������ĵ���Ϣ����ϵͳ���� */
		default:
				return	DefDesktopProc(hwnd,msg,wParam,lParam);
	}

	return WM_NULL;

}

/**
  * @brief  ������������
  * @param  ��
  * @retval ��
  */
void GUI_DesktopStartup(void)
{
	WNDCLASS	wcex;
	HWND hwnd;
	MSG msg;

	wcex.Tag 		    = WNDCLASS_TAG;

	wcex.Style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= desktop_proc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;//hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;//LoadCursor(NULL, IDC_ARROW);

	//�������洰��.
	hwnd = GUI_CreateDesktop(	WS_EX_LOCKPOS,
                              &wcex,
                              L"DESKTOP",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0,0,GUI_XSIZE,GUI_YSIZE,
                              NULL,0,NULL,NULL);

	GUI_Printf("HWND_Desktop=%08XH\r\n",	hwnd);

	//��ʾ���洰��.
	ShowWindow(hwnd,SW_SHOW);

	//����ϵͳ�򿪹����ʾ(���԰�ʵ��������Ƿ���Ҫ).
//	ShowCursor(TRUE);

	while(GetMessage(&msg,hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/********************************END OF FILE****************************/

