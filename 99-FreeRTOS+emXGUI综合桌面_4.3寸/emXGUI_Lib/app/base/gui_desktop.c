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


#include "GUI_AppDef.h"
#include "emXGUI.h"
#include "./pic_load/gui_pic_load.h"

/* �ⲿ��Դ������ɱ�־ */
BOOL Load_state = FALSE;
extern uint8_t Theme_Flag;   // �����־
/* ¼���ļ����·�� */
#define SCREENSHOTDIR	"0:/screenshot"

/*===================================================================================*/
extern void	GUI_Boot_Interface_Dialog(void *param);
extern void GUI_AppMain(void);


void	gui_app_thread(void *p)
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
//	GUI_AppMain();
 //   GUI_UserAppStart();
//   	ShellWindowStartup();
    while(1)
    {
      int i = 0;
      DIR dir;
      FRESULT result; 
      FIL *file;
      BOOL res;
      char recfilename[50];
      
      if (Key_Scan (KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON) 
      {
        /* �ļ�����ռ� */
        file =(FIL*)GUI_VMEM_Alloc(sizeof(FIL));
        
        /*  ���·�������ڣ������ļ���  */
        result = f_opendir(&dir,SCREENSHOTDIR);
        while(result != FR_OK)
        {
          i++;
          if (i > 5)
          {
            GUI_DEBUG("���ļ�ʧ�ܣ�����SD����");
            break;
          }
          f_mkdir(SCREENSHOTDIR);
          result = f_opendir(&dir,SCREENSHOTDIR);
        }
        
        if(i >= 5)
        {
          GUI_VMEM_Free(file);
          continue;
        }
        
        /* Ѱ�Һ����ļ��� */
        for(i=1;i<0xff;++i)
        {
          sprintf(recfilename,"0:/screenshot/screenshot%03d.bmp",i);
          result=f_open(file,(const TCHAR *)recfilename,FA_READ);
          if(result==FR_NO_FILE)break;					
        }
        f_close(file);
        
        GUI_VMEM_Free(file);
        
        if(i==0xff)
        {
          GUI_DEBUG("û���ҵ������ļ���,��ͼʧ��");
          continue;
        }
        
        if (PIC_Capture_Screen_To_BMP(recfilename) == TRUE)
        {
          GUI_DEBUG("��ͼ�ɹ�, %s", recfilename);
        }
        else
        {
          GUI_DEBUG("��ͼʧ��");
        }
      }
//      GUI_DEBUG("gui_app_thread");
      GUI_msleep(20);
    }
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
  if (Theme_Flag == 1)
  {
    if(lprc==NULL)
    {
      GetClientRect(hwnd,&rc);
    }
    else
    {
      CopyRect(&rc,lprc);
    }

    SetBrushColor(hdc,MapRGB(hdc,COLOR_DESKTOP_BACK_GROUND));
    FillRect(hdc,&rc);
      
    SetTextColor(hdc,MapRGB(hdc,255,255,255));
    
  //  SetFont(hdc, iconFont_64);
  //	DrawText(hdc,L" A B C D E \r\n F G H I J",-1,&rc,DT_LEFT|DT_VCENTER);
    SetFont(hdc, GB2312_32_Font);
    
    SetTextColor(hdc,MapRGB(hdc,255,255,255));
    rc.y +=10;
    //DrawText(hdc,L"emXGUI@Embedfire STM32F429 ",-1,&rc,DT_CENTER);
      
    /* ���� */
    GetClientRect(hwnd,&rc);
    SetBrushColor(hdc,MapRGB(hdc,82,85,82));
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT;
    rc.h = HEAD_INFO_HEIGHT;
    FillRect(hdc,&rc);
    
      /* ���� */ 
    SetFont(hdc, logoFont);
    /* ��ʾlogo */
    GetClientRect(hwnd,&rc);
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT-7;
    rc.h = HEAD_INFO_HEIGHT;
    
    SetTextColor(hdc,MapRGB(hdc,255,255,255)); 
    DrawText(hdc,L"B",-1,&rc,DT_LEFT|DT_VCENTER);
    
    
    GetClientRect(hwnd,&rc);
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT;
    rc.h = HEAD_INFO_HEIGHT;

    /* �ָ�Ĭ������ */
    SetFont(hdc, defaultFont);
    rc.x += 12;
    DrawText(hdc,L" Ұ��@emXGUI",-1,&rc,DT_LEFT|DT_VCENTER);

    GetClientRect(hwnd,&rc);
    rc.x = 214;
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT+10;
    rc.h = HEAD_INFO_HEIGHT;
    rc.w = 58;    
    /* ����ͼ������ */
    SetFont(hdc, controlFont_48);

    /* ����ͼ�� */
    SetTextColor(hdc,MapRGB(hdc,255,255,255)); 
  //  DrawText(hdc,L"D",-1,&rc,DT_TOP|DT_CENTER);
    DrawText(hdc,L"f",-1,&rc,DT_BOTTOM);

  // /* �ָ�Ĭ������ */
    SetFont(hdc, defaultFont);
    OffsetRect(&rc, 10, -7);
    DrawText(hdc,L"˵��",-1,&rc,DT_LEFT|DT_VCENTER);
    rc.x = 211;
    rc.w = 58;
    rc.h = 21;
    rc.y = 242;
    SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));
    DrawRoundRect(hdc, &rc, MIN(rc.w, rc.h)>>1);
  //  rc.y -= 20;
  //  DrawText(hdc,L"\r\n\r\n��ϸ",-1,&rc,DT_BOTTOM|DT_CENTER);
    GetClientRect(hwnd,&rc);
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT;
    rc.h = HEAD_INFO_HEIGHT;

    DrawText(hdc,L"www.firebbs.cn  ",-1,&rc,DT_RIGHT|DT_VCENTER); 
  }  
  else
  {
    /* ʹ��ͼƬ */
    rc.x = 0;
    rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT;
    rc.w = GUI_XSIZE;
    rc.h = HEAD_INFO_HEIGHT;
    
    BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_home_bk, rc.x, rc.y, SRCCOPY);

  }
}
extern GUI_SEM *Input_Sem;
/* ʹ��ר�õ��߳����������� */
#if 1
static	void	gui_input_thread(void *p)
{
	while(1)
	{
    GUI_SemWait(Input_Sem, 0xFFFFFFFF);
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
	switch(msg)
	{
    /* ���洴��ʱ,���������Ϣ,������������һЩ��ʼ������. */
		case	WM_CREATE:	
			   //����1��20ms��ʱ��������ѭ���¼�.
				 SetTimer(hwnd,1,20,TMR_START,NULL);

				//����App�߳�						
				{
#if (GUI_APP_BOOT_INTERFACE_EN )  
          
         GUI_Thread_Create(GUI_Boot_Interface_Dialog,  /* ������ں��� */
                              "Boot_Interface",/* �������� */
                              8*1024,  /* ����ջ��С */
                              NULL, /* ������ں������� */
                              5,    /* ��������ȼ� */
                              10); /* ����ʱ��Ƭ����������֧�� */
          GUI_Thread_Create(gui_input_thread,  /* ������ں��� */
                              "gui_input_thread",/* �������� */
                              1*1024,  /* ����ջ��С */
                              NULL, /* ������ں������� */
                              11,    /* ��������ȼ� */
                              10); /* ����ʱ��Ƭ����������֧�� */

          
        GUI_Thread_Create(gui_app_thread,  /* ������ں��� */
                            "GUI_APP",/* �������� */
                            2*1024,  /* ����ջ��С */
                            NULL, /* ������ں������� */
                            5,    /* ��������ȼ� */
                            10); /* ����ʱ��Ƭ����������֧�� */    
#endif
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
        
     /* ����Ƿ���������ϸ��һ�� */    
    case WM_LBUTTONDOWN:
		{

			POINT pt;
			RECT rc;

			pt.x =GET_LPARAM_X(lParam);
			pt.y =GET_LPARAM_Y(lParam);

			GetClientRect(hwnd,&rc);
      
      rc.y = GUI_YSIZE - HEAD_INFO_HEIGHT;
      rc.h = HEAD_INFO_HEIGHT;          

      /* ����������������Ϣ��slide window */
			if(PtInRect(&rc,&pt))
			{
        PostMessage(GetDlgItem(hwnd,ID_SLIDE_WINDOW), WM_MSG_FRAME_DOWN,0,0);
			}
		}
		break;

    /* �ͻ���������Ҫ������ */
		case	WM_ERASEBKGND:
		{   
      RECT rc = *(RECT*)lParam;
			HDC hdc =(HDC)wParam; 
      
        /* ������Դ������ɺ����ʾ�������棬�տ�ʼʱֻ��ʾ��ɫ */
       if(Load_state == TRUE)
       {
          _EraseBackgnd(hdc,NULL,hwnd);
       }
       else
       {
          SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
          FillRect(hdc, &rc);
       }
		}
		return TRUE;  

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

//  GUI_DEBUG("Create desktop");
	//�������洰��.
	hwnd = GUI_CreateDesktop(	WS_EX_LOCKPOS,
                              &wcex,
                              L"DESKTOP",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0,0,GUI_XSIZE,GUI_YSIZE,
                              NULL,0,NULL,NULL);

//	GUI_DEBUG("HWND_Desktop=%08XH\r\n",	hwnd);

	//��ʾ���洰��.
	ShowWindow(hwnd,SW_SHOW);

#if (GUI_SHOW_CURSOR_EN)
	//����ϵͳ�򿪹����ʾ(���԰�ʵ��������Ƿ���Ҫ).
	ShowCursor(TRUE);
#endif

	while(GetMessage(&msg,hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/********************************END OF FILE****************************/

