#include <emXGUI.h>

#ifndef __GUI_NETEORK_DIALOG
#define	__GUI_NETEORK_DIALOG

/* �ؼ� ID ���� */
enum
{   
   /* ���� ID */
  ID_TEXTBOX_Title      = 0x00,     // ������
  ID_TEXTBOX_Send        ,//0x01     // ������ʾ
  ID_TEXTBOX_Receive     ,//0x02     // ������ʾ
  ID_TEXTBOX_RemoteIP1   ,//0x07     // Զ��IP
  ID_TEXTBOX_RemoteIP2   ,//0x08     // Զ��IP
  ID_TEXTBOX_RemoteIP3   ,//0x09     // Զ��IP
  ID_TEXTBOX_RemoteIP4  ,//0x0A     // Զ��IP
  ID_TEXTBOX_RemotePort  ,//0x0B     // Զ�˶˿�
  
  /* ��ť ID */
  eID_Network_EXIT    ,//0x03
  eID_LINK_STATE      ,//0x04
  eID_Network_Send    ,//0x05
  eID_Receive_Clear   ,//0x06
  
  ID_Hint_Win,
  
};
extern HWND Network_Main_Handle;

#endif


