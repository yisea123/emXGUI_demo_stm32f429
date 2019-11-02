
/*
 *GUI_DEMO_Drawbitmap.c
 *2018��3��17������7:41:26
 *
 */

#include <emXGUI.h>
#include "emXGUI_JPEG.h"
#include "GUI_AppDef.h"
#include "emxgui_png.h"
//#include "cpuusage.h"

/* Ϊ0ʱ��ʾGUI�Ĺ�棬Ϊ1ʱ��ʾ������Ĺ�� */
u8 slogan_flag = 1;
 /*============================================================================*/
 /*============================================================================*/
const wchar_t string_slogan_gui[] = L" �� �ͺ� : ��ս��F429 V2������\r\n\r\n"\
L" �� ���� : www.embedFire.com\r\n\r\n"\
L" �� ��̳ : www.firebbs.cn\r\n\r\n"\
L" �� �Ա� : firestm32.taobao.com\r\n\r\n"\
L" �� ΢�Ź��ں� : ���ں�������Ұ����ӡ���\r\n\r\n"\
L"    ���ɹ�ע";
 
const wchar_t header_slogan_gui[] = L"emXGUI �й�����Ƕ��ʽͼ�ν���";

const wchar_t string_slogan_board[] = L" �� �ͺ� : ��ս��F429 V2������\r\n\r\n"\
L" �� ���� : www.embedFire.com\r\n\r\n"\
L" �� ��̳ : www.firebbs.cn\r\n\r\n"\
L" �� �Ա� : firestm32.taobao.com\r\n\r\n"\
L" �� ΢�Ź��ں� : ���ں�������Ұ����ӡ���\r\n\r\n"\
L"    ���ɹ�ע";
const wchar_t header_slogan_board[] = L"Ұ�� @ Ƕ��ʽ����ר�ҡ�Ϊ��ѧ����";
//extern const unsigned char gImage_0[];
/*============================================================================*/

/* �ⲿͼƬ */
extern const char res_slogan[];
/* �ⲿͼƬ���ݴ�С */
extern unsigned int res_slogan_size(void);
extern uint8_t Theme_Flag;   // �����־

#define GUI_DEMO_PIC             "gui_demo_pic.png"
#define GUI_EXPLAINDESKTOP_PIC   "explain_desktop.jpg"

/*============================================================================*/
/**
  * @brief  emXGUI slogan���溯����
			ͨ���޸ı����������ݿɸ���slogan
  * @param  hdc ��ͼ������
  * @param  lprc Ҫ���Ƶľ�������ΪNULLʱ���Զ�����hwnd�Ŀͻ���
  * @param  hwnd ���ڶ�����
  * @retval ��
  */
static void CreateSlogan(HDC hdc, const RECT *lprc, HWND hwnd)
{
	RECT rc;
	JPG_DEC *dec;

	const wchar_t *p_header;
	const wchar_t *p_string;

	if (slogan_flag)
	{
		p_header = header_slogan_board;
		p_string = string_slogan_board;
	}
	else
	{
		p_header = header_slogan_gui;
		p_string = string_slogan_gui;
	}

	if (lprc == NULL)
	{
		GetClientRect(hwnd, &rc);
	}
	else
	{
		CopyRect(&rc, lprc);
	}

	/* ���� */
	GetClientRect(hwnd, &rc);
	rc.y = 0;
	rc.h = 0 + HEAD_INFO_HEIGHT;
	SetBrushColor(hdc, MapRGB(hdc, COLOR_DESKTOP_BACK_GROUND));
	FillRect(hdc, &rc);

	SetBrushColor(hdc, MapRGB(hdc, 82, 85, 82));
	rc.y = rc.y + rc.h;
	rc.h = GUI_YSIZE + 0 - rc.y;
	FillRect(hdc, &rc);

	/* ���� */

	SetFont(hdc, GB2312_32_Font);
	SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
	GetClientRect(hwnd, &rc);
	rc.h = HEAD_INFO_HEIGHT;

	DrawText(hdc, p_header, -1, &rc, DT_CENTER | DT_VCENTER);	

	GetClientRect(hwnd, &rc);
	rc.y += HEAD_INFO_HEIGHT + 40;

	/* ����� */
	SetFont(hdc, defaultFont);
	//      DrawText(hdc, SLOGAN, -1,&rc0,DT_LEFT);       

	DrawText(hdc, p_string, -1, &rc, DT_LEFT);

	SetTextColor(hdc, MapRGB(hdc, COLOR_DESKTOP_BACK_GROUND));
  
	SetTextColor(hdc, MapRGB(hdc, 250,250,250));
	rc.y = GUI_YSIZE - 60;
  rc.x = 180;
  rc.h = 50;
	DrawText(hdc, L"copyright @ ��ݸҰ����Ӽ������޹�˾", -1, &rc, DT_LEFT|DT_VCENTER);

	/* �Ҳ�ͼƬ */
#if 1
  /* ʹ����Դ�豸�е��ļ� */
  {
		BOOL res;
		u8 *pic_buf;
		u32 pic_size;
		PNG_DEC *png_dec;
		BITMAP png_bm;

		//res = FS_Load_Content(GUI_DEMO_PIC, (char**)&pic_buf, &pic_size);    // ��Դ�� SD ��
		res = RES_Load_Content(GUI_DEMO_PIC, (char**)&pic_buf, &pic_size);     // ��Դ���ⲿ FLASH
		if(res)
		{
			png_dec = PNG_Open(pic_buf);
			PNG_GetBitmap(png_dec, &png_bm);
			DrawBitmap(hdc, 473, 105, &png_bm, NULL);
			PNG_Close(png_dec);
		}
		/* �ͷ�ͼƬ���ݿռ� */
		RES_Release_Content((char **)&pic_buf);
  }
#else
	/* ����ͼƬ���ݴ���JPG_DEC��� */
	dec = JPG_Open(res_slogan, res_slogan_size());

	/* �������ڴ���� */
	JPG_Draw(hdc, 480, HEAD_INFO_HEIGHT + 40, dec);

	/* �ر�JPG_DEC��� */
	JPG_Close(dec);
  
#endif
}



static LRESULT	WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	static int win_pos = 0;
	static HDC hdc_mem = NULL;
	static HDC hdc_mem_pic = NULL;

	switch (msg)
	{
	case WM_CREATE: //���ڴ���ʱ,���Զ���������Ϣ,��������һЩ��ʼ���Ĳ����򴴽��Ӵ���
	{
		GetClientRect(hwnd, &rc); //��ô��ڵĿͻ�������

  	/* �����ڴ���� */
		hdc_mem = CreateMemoryDC(SURF_SCREEN, rc.w, rc.h);
		hdc_mem_pic = CreateMemoryDC(SURF_SCREEN, rc.w, rc.h);

		BOOL res;
    u8 *jpeg_buf;
    u32 jpeg_size;
		JPG_DEC *dec;

    /* ��Դ�豸�м��� */
    res = RES_Load_Content(GUI_EXPLAINDESKTOP_PIC, (char **)&jpeg_buf, &jpeg_size);    /* ʹ��ͼƬ */
    //res = FS_Load_Content(GUI_EXPLAINDESKTOP_PIC, (char **)&jpeg_buf, &jpeg_size);
    if(res)
    {
      /* ����ͼƬ���ݴ���JPG_DEC��� */
      dec = JPG_Open(jpeg_buf, jpeg_size);

      /* �������ڴ���� */
      JPG_Draw(hdc_mem_pic, 0, 0, dec);

      /* �ر�JPG_DEC��� */
      JPG_Close(dec);
    }
    
    /* �ͷ�ͼƬ���ݿռ� */
    RES_Release_Content((char **)&jpeg_buf);

		/* ����slogan���ڴ��豸 */
		CreateSlogan(hdc_mem, NULL, hwnd);

		SetTimer(hwnd, 1, 5, TMR_START, NULL);

	}
	return TRUE;
	////

	case WM_NOTIFY: //WM_NOTIFY��Ϣ:wParam��16λΪ���͸���Ϣ�Ŀؼ�ID,��16λΪ֪ͨ��;lParamָ����һ��NMHDR�ṹ��.
	{
		u16 code;
		//      ,id;

		code = HIWORD(wParam); //���֪ͨ������.
//			id   =LOWORD(wParam); //��ò�������Ϣ�Ŀؼ�ID.

		if (code == BN_CLICKED)
		{
			PostCloseMessage(hwnd); //����WM_CLOSE��Ϣ�رմ���
		}
	}
	break;
	////
	case WM_TIMER:
	{
		/* ��ʱ����Ϣ���������������� */
		RECT rc;
		GetWindowRect(hwnd, &rc);
		SetForegroundWindow(hwnd);

		if (win_pos == 1)
		{
			if (rc.y > 0)
			{
				OffsetRect(&rc, 0, -(rc.h >> 3));
				rc.y = MAX(rc.y, 0);
				MoveWindow(hwnd, rc.x, rc.y, rc.w, rc.h, TRUE);
			}
			else
				KillTimer(hwnd, 1);
		}
		else
		{
			if (rc.y < (GUI_YSIZE))
			{
				OffsetRect(&rc, 0, (rc.h >> 3));
				rc.y = MIN(rc.y, (GUI_YSIZE));//
				MoveWindow(hwnd, rc.x, rc.y, rc.w, rc.h, TRUE);
			}
			else
				KillTimer(hwnd, 1);
		}
	}
	//			InvalidateRect(hwnd,&rc,FALSE);
	break;

	case WM_LBUTTONDOWN:
	{
		/* ����Ƿ��������ڣ����ǵĻ�ʹ�ܶ�ʱ�� ��������*/
		POINT pt;
		RECT rc;

		pt.x = GET_LPARAM_X(lParam);
		pt.y = GET_LPARAM_Y(lParam);

		GetClientRect(hwnd, &rc);

		if (PtInRect(&rc, &pt))
		{
			KillTimer(hwnd, 1);
			SetTimer(hwnd, 1, 2, TMR_START, NULL);
			win_pos = 0;
		}
	}
	break;

	case WM_ERASEBKGND:
	{
		//			HDC hdc=(HDC)wParam;

		//			GetClientRect(hwnd,&rc);
		//      SetBrushColor(hdc,MapRGB(hdc,0,99,166));
		//			FillRect(hdc,&rc);      
	}
	return TRUE;
	/////

	case WM_PAINT: //������Ҫ����ʱ�����Զ���������Ϣ.
	{
		PAINTSTRUCT ps;
		HDC hdc;
		//			WCHAR wbuf[128];
		RECT rc0;

		hdc = BeginPaint(hwnd, &ps);

		////�û��Ļ�������...
		GetClientRect(hwnd, &rc0);
		/* ���ڴ�����������Ļ */
		
		if (Theme_Flag == 0)
		{
			BitBlt(hdc, 0, 0, rc0.w, rc0.h, hdc_mem_pic, 0, 0, SRCCOPY);
		}
		else
		{
			BitBlt(hdc, 0, 0, rc0.w, rc0.h, hdc_mem, 0, 0, SRCCOPY);
		}
		EndPaint(hwnd, &ps);
		//////////
	}
	break;
	////

	case WM_DESTROY: //��������ʱ�����Զ���������Ϣ����������һЩ��Դ�ͷŵĲ���.
	{
		return PostQuitMessage(hwnd); //����PostQuitMessage��ʹ�������ڽ������˳���Ϣѭ��.
	}
	//		break;
			////

		/* ���յ����洰�ڴ����Ĵ�����Ϣ*/
	case WM_MSG_FRAME_DOWN:
	{
		/* ���¿�����ʱ������������ */
		KillTimer(hwnd, 1);
		SetTimer(hwnd, 1, 2, TMR_START, NULL);

		win_pos = 1;
	}
	break;

	default:
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	}

	return WM_NULL;
}

/*============================================================================*/

void	GUI_DEMO_SlideWindow(void *p)
//static void	AppMain(void)
{
	WNDCLASS	wcex;
	MSG msg;
	HWND	hwnd;

	/////
	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinProc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);

	//����������
	hwnd = CreateWindowEx(NULL,
		&wcex,
		L"SlideWindow",
		//								/*WS_MEMSURFACE|*/WS_CAPTION|WS_DLGFRAME|WS_BORDER|WS_CLIPCHILDREN,
		/*WS_MEMSURFACE|*/WS_CLIPCHILDREN|WS_OVERLAPPED,

		0, GUI_YSIZE, GUI_XSIZE, GUI_YSIZE,
		GetDesktopWindow(), ID_SLIDE_WINDOW, NULL, NULL);

	//��ʾ������
	ShowWindow(hwnd, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/*============================================================================*/
