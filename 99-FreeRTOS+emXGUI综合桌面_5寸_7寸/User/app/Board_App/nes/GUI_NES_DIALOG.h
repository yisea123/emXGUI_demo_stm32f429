#pragma once

#include "emXGUI.h"

typedef struct
{ 
  HWND hwnd;
  u16* buf;       //��ʾ������
  
  GUI_SEM* Exit_Sem;
  
  int frame;
  
  
}NES_DIALOG_Typedef;

