#include <emXGUI.h>
#include <string.h>
#include <stdio.h>
#include "x_libc.h"
#include "GUI_AppDef.h"
#include "netconf.h"
#include "GUI_Network_Dialog.h"
#include "tcp_echoclient.h"
#include "tcp_echoserver.h"
#include "udp_echoclient.h"

int		number_input_box(int x, int y, int w, int h,
							const WCHAR *pCaption,
							WCHAR *pOut,
							int MaxCount,
							HWND hwndParent);

/* ��ѡ�� ID */
#define ID_RB1    (0x1100 | (1<<16))
#define ID_RB2    (0x1101 | (1<<16))
#define ID_RB3    (0x1102 | (1<<16))

TaskHandle_t Network_Task_Handle;

int8_t NetworkTypeSelection = 0;

HWND Send_Handle;
HWND Receive_Handle;
HWND Network_Main_Handle;
              
uint8_t network_start_flag=0;

extern struct netif gnetif;
extern __IO uint8_t EthLinkStatus;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
DRV_NETWORK drv_network;
uint16_t bsp_result=0;

/**
  * @brief  ͨ�ö�ʱ��3�жϳ�ʼ��
  * @param  period : �Զ���װֵ��
  * @param  prescaler : ʱ��Ԥ��Ƶ��
  * @retval ��
  * @note   ��ʱ�����ʱ����㷽��:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=��ʱ������Ƶ��,ΪSystemCoreClock/2=90,��λ:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=period;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

#ifdef   AAAA// ��Backend_vidoplayer.c��ʵ��
/**
  * @brief  ��ʱ��3�жϷ�����
  * @param  ��
  * @retval ��
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		LocalTime+=10;//10ms����
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
#endif
void Network_Dispose_Task(void *p) 
{
  if(network_start_flag==0)
  {
    /* Configure ethernet (GPIOs, clocks, MAC, DMA) */
    if(ETH_BSP_Config()==1)
    {
      network_start_flag=0;
      bsp_result |=1;
      /* ��ʼ������ */
      SetTimer(Network_Main_Handle, 10, 100, TMR_SINGLE|TMR_START, NULL);
      vTaskSuspend(Network_Task_Handle);    // �����Լ� ����ִ�� 
    }
    else
    {
      network_start_flag=1;
      bsp_result &=~ 1;  
    }

  }
  
  if((drv_network.net_init==0)&&((bsp_result&1)==0))
  {     
    /* Initilaize the LwIP stack */
    LwIP_Init(); 

    drv_network.net_local_ip1  = (uint8_t)(gnetif.ip_addr.addr&0xFF);
    drv_network.net_local_ip2  = (uint8_t)((gnetif.ip_addr.addr>>8)&0xFF);
    drv_network.net_local_ip3  = (uint8_t)((gnetif.ip_addr.addr>>16)&0xFF);
    drv_network.net_local_ip4  = (uint8_t)((gnetif.ip_addr.addr>>24)&0xFF);
    drv_network.net_local_port = LOCAL_PORT;
    
    drv_network.net_remote_ip1  = DEST_IP_ADDR0;
    drv_network.net_remote_ip2  = DEST_IP_ADDR1;
    drv_network.net_remote_ip3  = DEST_IP_ADDR2;
    drv_network.net_remote_ip4  = DEST_IP_ADDR3;
    drv_network.net_remote_port = DEST_PORT;     
    
    drv_network.net_init=1;
  }
  
//  PostCloseMessage(GetDlgItem(Network_Main_Handle, ID_Hint_Win));
  if(bsp_result&1)
  {		
    char str[30];
    if(network_start_flag==2)
    {
      /* Configure ethernet (GPIOs, clocks, MAC, DMA) */
      if(ETH_BSP_Config()==1)
      {
        bsp_result |= 1;
        sprintf(str," ");  
      }
      else
      {
        bsp_result &=~ 1;    
        sprintf(str,"< must be restart Safari >"); 
      }
    }
    else
    {
      sprintf(str," ");  
    }
    network_start_flag=2;
  }
  InvalidateRect(Network_Main_Handle, NULL, TRUE);
  drv_network.net_connect=0;
  drv_network.net_type=0; 
  TIM3_Config(999,899);//10ms��ʱ�� 
  LocalTime=0;
  TIM_SetCounter(TIM3,0);
  EthLinkStatus=0;
  while(1)
  {
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);

    GUI_msleep(3);//WM_Exec();//
  }
}

//�˳���ť�ػ���
static void Ent_ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
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

// �ػ���ͨ��ť
static void Ent_Button_OwnerDraw(DRAWITEM_HDR *ds)
{
  HDC hdc;
  RECT rc;
  WCHAR wbuf[128];
  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
  hwnd = ds->hwnd;

  SetBrushColor(hdc, MapRGB(hdc, 0, 202, 252));
  EnableAntiAlias(hdc, ENABLE);
  FillRoundRect(hdc, &rc, 5);
  EnableAntiAlias(hdc, DISABLE);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
    OffsetRect(&rc, 1, 1);
		SetTextColor(hdc, MapRGB(hdc, 200, 200, 200));
	}
	else
	{ //��ť�ǵ���״̬

		SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));      //���û���ɫ
	}

  GetWindowText(hwnd, wbuf, 128);    // �õ�ԭ�ı�

  SetFont(hdc, defaultFont);
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);     // ��������(������ʾ)
}

/*
 * @brief  �ػ���ʾ���ȵ�͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Brigh_Textbox_OwnerDraw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc;
	WCHAR *wbuf;
  uint32_t WinTexeLen = 0;

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

  SetTextColor(hdc, MapRGB(hdc, 170, 170, 170));

  if (ds->ID == ID_TEXTBOX_Receive)
  {
    DrawText(hdc, L"������", -1, &rc, DT_VCENTER|DT_CENTER);  // ��������
  }
  else
  {
    SetPenColor(hdc, MapRGB(hdc, 121, 121, 121));

    EnableAntiAlias(hdc, ENABLE);
    DrawRoundRect(hdc, &rc, 7);
    EnableAntiAlias(hdc, DISABLE);
    DrawText(hdc, L"������", -1, &rc, DT_VCENTER|DT_CENTER);  // ��������
  }
  
  OffsetRect(&rc, 5, 5);
  SetTextColor(hdc, MapRGB(hdc, 0, 0, 0));
  WinTexeLen = GetWindowTextLength(hwnd);               // ��ȡ�ı�����

  if (WinTexeLen > 0)
  {
    wbuf = (WCHAR *)GUI_VMEM_Alloc(WinTexeLen*2);         // �����ı����� + ����Ϣ���ȵ��ڴ�
    GetWindowText(hwnd, wbuf, WinTexeLen+1);    // �õ�ԭ�ı�

    SetFont(hdc, defaultFont);
    DrawText(hdc, wbuf, -1, &rc, DT_TOP|DT_LEFT|DT_WORDBREAK);     // ��������( �������룬����룬���ַ��������α߽�ʱ,�Զ�����)
  }
}

extern void TCP_Echo_Init(void);
extern int SelectDialogBox(HWND hwndParent, RECT rc,const WCHAR *pText,const WCHAR *pCaption,const MSGBOX_OPTIONS *ops);
WCHAR I[128];

static LRESULT	win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
      RECT rc;
      GetClientRect(hwnd, &rc); 
      HWND Temp_Handle;
      
      xTaskCreate((TaskFunction_t )Network_Dispose_Task,      /* ������ں��� */
                  (const char*    )"Network Dispose Task",    /* �������� */
                  (uint16_t       )3*1024/4,                  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                  (void*          )NULL,                      /* ������ں������� */
                  (UBaseType_t    )5,                         /* ��������ȼ� */
                  (TaskHandle_t*  )&Network_Task_Handle);     /* ������ƿ�ָ�� */
                      
      CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY |WS_OWNERDRAW|WS_VISIBLE,
                  740, 10, 36, 36, hwnd, eID_Network_EXIT, NULL, NULL); 

      /* ����һ�鵥ѡ�� */
      rc.x = 415;
      rc.y = 86;
      rc.w = 155;
      rc.h = 28;
      CreateWindow(BUTTON,L"TCP Server",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB1,NULL,NULL);
      SendMessage(GetDlgItem(hwnd, ID_RB1&0xFFFF), BM_SETSTATE, BST_CHECKED, 0);    // Ĭ��ѡ��
      
      
      OffsetRect(&rc, rc.w+2, 0);
      CreateWindow(BUTTON,L"TCP Client",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB2,NULL,NULL);

      OffsetRect(&rc, rc.w, 0);
      rc.w = 80;
      CreateWindow(BUTTON,L"UDP",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB3,NULL,NULL);
      
      CreateWindow(BUTTON, L"δ����", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                  717, 218, 79, 30, hwnd, eID_LINK_STATE, NULL, NULL);
      
      /* ���ݷ����ı����� */
      rc.w = 384;
      rc.h = 224;
      rc.x = 412;
      rc.y = 251;
      Send_Handle = CreateWindow(TEXTBOX, L"��ã�������Ұ�𿪷��� ^_^", WS_TRANSPARENT | WS_VISIBLE|WS_OWNERDRAW, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Send, NULL, NULL);

      /* �������մ��� */
      rc.w = 400;
      rc.h = 380;
      rc.x = 5;
      rc.y = 55;
      Receive_Handle = CreateWindow(TEXTBOX, L"", WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Receive, NULL, NULL);
      
      rc.x = 720;
      rc.y = 440;
      rc.w = 71;
      rc.h = 30;
      CreateWindow(BUTTON, L"����", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                         rc.x,rc.y,rc.w,rc.h, hwnd, eID_Network_Send, NULL, NULL); 
                         
      rc.x = 289;
      rc.h = 30;
      rc.w = 111;
      rc.y = 440;
      CreateWindow(BUTTON, L"��ս���", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                         rc.x,rc.y,rc.w,rc.h, hwnd, eID_Receive_Clear,       NULL, NULL); 

      /* ���ݷ����ı����� */
      rc.w = 48;
      rc.h = 28;
      rc.x = 414;
      rc.y = 219;
      Temp_Handle = CreateWindow(TEXTBOX, L"192", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP1, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+9, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"168", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP2, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+9, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"000", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP3, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+9, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"138", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP4, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+9 , 0);
      rc.w = 69;
      Temp_Handle = CreateWindow(TEXTBOX, L"8080", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemotePort, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);



      break;
    } 
    case WM_TIMER:
    {
      int tmr_id;

      tmr_id = wParam;    // ��ʱ�� ID

      if (tmr_id == 10)    // ��̫����ʼ������
      {
        RECT RC;
        MSGBOX_OPTIONS ops;
        const WCHAR *btn[] ={L"ȷ��",L"ȡ��"};      //�Ի����ڰ�ť������

        ops.Flag =MB_ICONERROR;
        ops.pButtonText =btn;
        ops.ButtonCount =2;
        RC.w = 300;
        RC.h = 200;
        RC.x = (GUI_XSIZE - RC.w) >> 1;
        RC.y = (GUI_YSIZE - RC.h) >> 1;
        SelectDialogBox(hwnd, RC, L"��̫����ʼ��ʧ��\n�����¼�����ӡ�", L"����", &ops);    // ��ʾ������ʾ��
        PostCloseMessage(hwnd);                                                          // ���͹رմ��ڵ���Ϣ
      }
      
      break;
    }

    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      WCHAR tempstr[30];
      
      RECT rc =  {0, 0, GUI_XSIZE, GUI_YSIZE};
      // hdc_pointer = CreateMemoryDC(SURF_SCREEN, PANEL_W, PANEL_H);
      hdc = BeginPaint(hwnd, &ps);
      
      SetBrushColor(hdc, MapRGB(hdc, 255, 255, 255));
      FillRect(hdc, &rc);

      rc.h = 50;
      GradientFillRect(hdc, &rc, MapRGB(hdc, 1, 218, 254), MapRGB(hdc, 1, 168, 255), FALSE);
      SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
      DrawText(hdc, L"��̫��", -1, &rc, DT_VCENTER|DT_CENTER);

      SetPenColor(hdc, MapRGB(hdc, 121, 121, 121));

      rc.x = 5;
      rc.y = 55;
      rc.w = 400;
      rc.h = 420;
      EnableAntiAlias(hdc, ENABLE);
      DrawRoundRect(hdc, &rc, 7);     // ���ƽ����������
      EnableAntiAlias(hdc, DISABLE);
      
      SetFont(hdc, defaultFont);
      SetTextColor(hdc, MapRGB(hdc, 0x16, 0x9B, 0xD5));

      rc.x = 412;
      rc.y = 57;
      rc.w = 120;
      rc.h = 28;
      DrawText(hdc, L"ͨѶЭ�飺", -1, &rc, DT_LEFT|DT_TOP);

      // rc.x = 550;
      // rc.y = 36;
      // rc.w = 60;
      // rc.h = 30;
      // DrawText(hdc, L"���ݷ��ͣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      rc.w = 207;
      rc.h = 28;
      rc.x = 412;
      rc.y = 120;
      DrawText(hdc, L"����IP��ַ&�˿ڣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      rc.y = 188;
      DrawText(hdc, L"Զ��IP��ַ&�˿ڣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      SetTextColor(hdc, MapRGB(hdc, 10, 10, 10));
      x_wsprintf(tempstr, L"[%d.%d.%d.%d:%d]",drv_network.net_local_ip1,drv_network.net_local_ip2,\
                                       drv_network.net_local_ip3,drv_network.net_local_ip4,\
                                       drv_network.net_local_port);
      rc.w = 300;
      rc.h = 30;
      rc.x = 412;
      rc.y = 152;
      DrawText(hdc, tempstr, -1, &rc, DT_LEFT|DT_TOP);
      
      SetTextColor(hdc, MapRGB(hdc, 0x16, 0x9B, 0xD5));

      // rc.w = 120;
      // rc.h = 30;
      // rc.x = 10;
      // rc.y = 210;
      // DrawText(hdc, L"���ݽ��գ�", -1, &rc, DT_LEFT|DT_TOP);

      rc.w = 7;
      rc.h = 28;
      rc.x = 463-4;
      rc.y = 223;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      rc.x = 520-4;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      rc.x = 577-4;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      rc.x = 634-4;
      DrawText(hdc, L":", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      EndPaint(hwnd, &ps);
      break;
    }

    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_Network_EXIT:
          {
            Ent_ExitButton_OwnerDraw(ds);
            return TRUE;             
          }  

          case ID_TEXTBOX_Send:
          case ID_TEXTBOX_Receive:
          {
            Brigh_Textbox_OwnerDraw(ds);
            return TRUE;   
          }

          case eID_LINK_STATE:
          case eID_Network_Send:
          case eID_Receive_Clear:
          {
            Ent_Button_OwnerDraw(ds);
            return TRUE;   
          }
       }

       break;
    }

    //����TEXTBOX�ı�����ɫ�Լ�������ɫ
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
      
			if(id >= ID_TEXTBOX_RemoteIP1 && id <= ID_TEXTBOX_RemotePort)
			{
				CTLCOLOR *cr;
				cr =(CTLCOLOR*)lParam;
				cr->TextColor =RGB888(0, 0, 0);//������ɫ��RGB888��ɫ��ʽ)
				cr->BackColor =RGB888(255, 255, 255);//������ɫ��RGB888��ɫ��ʽ)
				cr->BorderColor =RGB888(10,10,10);//�߿���ɫ��RGB888��ɫ��ʽ)
				return TRUE;				
			}

      return FALSE;
		}

    case WM_NOTIFY:
    {
      u16 code, id;
      id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
      code=HIWORD(wParam);//��ȡ��Ϣ������    
      if(code == BN_CLICKED && id == eID_Network_EXIT)    // �˳���ť����
      {
        PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
        break;
      }
      if(code == BN_CLICKED && id == eID_Receive_Clear)    // ��ս��յĴ��ڵİ�ť������
      {
        SetWindowText(Receive_Handle, L"");
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP1)    // IP1 �༭�򱻰���
      {
        number_input_box(0, 0, 800, 480, L"IP1", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP1), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP2){    // IP2 �༭�򱻰���
        number_input_box(0, 0, 800, 480, L"IP2", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP2), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP3){    // IP3 �༭�򱻰���
        number_input_box(0, 0, 800, 480, L"IP3", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP3), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP4){    // IP4 �༭�򱻰���
        number_input_box(0, 0, 800, 480, L"IP4", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP4), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemotePort)    // �˿� �༭�򱻰���
      {
        number_input_box(0, 0, 800, 480, L"PORT", I, 5, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemotePort), I);
        break;
      }
      
      if(code == BN_CLICKED && id == eID_LINK_STATE)
      {
        if((bsp_result&1)||EthLinkStatus)
          {
            break;
          }
          if(drv_network.net_connect==0)
          {
            uint8_t connectflag;
            WCHAR wbuf[128];
            char buf[128];

            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP1), wbuf, 128);    // ��ȡ�ı�����ı�
            x_wcstombs_cp936(buf, wbuf, 128);                                    // �����ַ���תΪ���ַ���
            drv_network.net_remote_ip1 = x_atoi(buf);                            // �ַ���ת����
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP2), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip2 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP3), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip3 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP4), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip4 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemotePort), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_port = x_atoi(buf);
            drv_network.net_type=NetworkTypeSelection;
            switch(drv_network.net_type)
            {
              case 0:
                /*create tcp server */ 
                connectflag=tcp_echoserver_init(drv_network);
                break;
              case 1:
                /*connect to tcp server */
                connectflag=tcp_echoclient_connect(drv_network);
                break;
              case 2:
                /* Connect to tcp server */ 
                connectflag=udp_echoclient_connect(drv_network);		
                break;            
            }
            if(connectflag==0)    // ���ӳɹ�
            {
              drv_network.net_connect=1; 
              SetWindowText(GetDlgItem(hwnd, eID_LINK_STATE), L"������");
            }      
          }
          else
          {
            /* �Ͽ����� */
            SetWindowText(GetDlgItem(hwnd, eID_LINK_STATE), L"δ����");
            switch(drv_network.net_type)
            {
              case 0:
                tcp_echoserver_close();
                break;
              case 1:
                tcp_echoclient_disconnect();
                break;
              case 2:
                udp_echoclient_disconnect();	
                break;            
            }
            drv_network.net_connect=0;
          }
        }
        if(code == BN_CLICKED && id == eID_Network_Send)
        {
          if(drv_network.net_connect==1)          
          {
            WCHAR wbuf[128];
            char comdata[128];

            /* ��ȡ���ʹ��ڵ��ַ��� */
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_Send), wbuf, 128);
            x_wcstombs_cp936(comdata, wbuf, 128);

            /* ������Ϣ */
            switch(drv_network.net_type)
            {
              case 0:
                network_tcpserver_send((char *)comdata);
                break;
              case 1:
                network_tcpclient_send((char *)comdata);
                break;
              case 2:
                udp_echoclient_send((char *)comdata);
                break;            
            }
          }
        }
      
        /* ��ѡ��ť������ */
        if( (id >= (ID_RB1 & ~(1<<16))) && (id <= (ID_RB3 & ~(1<<16))))
        {
          if (code == BN_CLICKED)
          {
            NetworkTypeSelection = id & 3;
          }
        }

      break;
    } 

    case WM_DESTROY:
    { 
      GUI_Thread_Delete(Network_Task_Handle);    // ɾ�����紦������
      NetworkTypeSelection = 0;                  // ��λĬ�ϵ�ѡ��

      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
  
}

void GUI_NetworkDLG_Dialog(void)
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
	Network_Main_Handle = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                              &wcex,
                              L"GUI_ADC_CollectVoltage_Dialog",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0, 0, GUI_XSIZE, GUI_YSIZE,
                              NULL, NULL, NULL, NULL);

   //��ʾ������
	ShowWindow(Network_Main_Handle, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, Network_Main_Handle))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


