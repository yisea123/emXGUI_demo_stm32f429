#include "emXGUI.h"
//#include "cpuusage.h"
#include "string.h"
#include "ff.h"
#include "x_libc.h"
#include "emXGUI_JPEG.h"
#include "mp3Player.h"
#include "musiclist.h"

#define ID_SCROLLBAR_POWER   0x1104   //������
#define ID_BUTTON_Power      0x1000   //���� 
#define ID_BUTTON_List       0x1001   //����List
#define ID_BUTTON_Equa       0x1002   //������
#define ID_BUTTON_Folder     0x1003   //�ļ���
#define ID_BUTTON_BACK       0x1004   //��һ��
#define ID_BUTTON_START      0x1005   //��ͣ��
#define ID_BUTTON_NEXT       0x1006   //��һ��
 

#define MAX_item   3


extern unsigned char playlist[MUSIC_MAX_NUM][FILE_NAME_LEN];
extern unsigned char lcdlist[MUSIC_MAX_NUM][MUSIC_NAME_LEN];
extern void	GUI_MusicList_DIALOG(void);
extern uint8_t  file_num;
char path[100]="0:";

char* file_name[MAX_item] = { 
   "0:srcdata/�Ź���-��������.mp3", 
   "0:srcdata/Ҷ����-���Ż�.mp3",      
   "0:srcdata/�Ź���-ȫ������.mp3",
};




extern HFONT hFont_SDCARD;
extern HFONT hFont_SDCARD_100;
extern HFONT DEFAULT_FONT;
static BITMAP bm_0;
extern char timg[];
unsigned int tiger_size(void);
typedef struct{
   char *icon_name;//ͼ����
   RECT rc;        //λ����Ϣ
   BOOL state;     //״̬  
}icon_S;

icon_S music_icon[8] = {
   {"yinliang",         {5,402,72,72},        FALSE},
   {"yinyueliebiao",    {580,404,72,72},      FALSE},
   {"junhengqi",        {652,404,72,72},      FALSE},
   {"wenjianjia",       {724,404,72,72},      FALSE},
   {"zuoshangjiaolist", {20,20,40,40},        FALSE},
   {"shangyishou",      {128, 200, 72, 72},   FALSE},
   {"zanting/bofang",   {300, 140, 200, 200},   FALSE},
   {"xiayishou",        {600, 200, 72, 72}, FALSE},      
};

static void App_MusicList()
{
	static int thread=0;
	static int app=0;
   rt_thread_t h;
	if(thread==0)
	{  
      h=rt_thread_create("App_MusicList",(void(*)(void*))App_MusicList,NULL,10*1024,5,5);
      rt_thread_startup(h);				

      thread =1;
	}

	if(thread==1) //�߳��Ѵ�����?
	{
		if(app==0)
		{
			app=1;
			GUI_MusicList_DIALOG();

			app=0;
			thread=0;
		}
	}
}


rt_thread_t h_music;
int power;//����ֵ
static void App_PlayMusic(HWND hwnd)
{
	static int thread=0;
	static int app=0;
   HDC hdc;
   printf("music�߳�\n");
   char music_name[FILE_NAME_LEN]={0};
	if(thread==0)
	{  
      h_music=rt_thread_create("App_PlayMusic",(void(*)(void*))App_PlayMusic,NULL,5*1024,5,5);
      thread =1;
      rt_thread_startup(h_music);				
      rt_thread_suspend(h_music);
      rt_schedule();
      return;
	}

	while(thread) //�߳��Ѵ�����
	{
      
		if(app==0)
		{
			app=1;
         hdc = GetDC(hwnd);
         int i = 0;
         while(playlist[play_index][i]!='\0')
			{
				music_name[i]=playlist[play_index][i];
				i++;
			}
			music_name[i]='\0';
			mp3PlayerDemo(music_name, power, hdc);   
           
			app=0;
         ReleaseDC(hwnd, hdc);
         GUI_msleep(20);
		}
	}
   //
}


/**
  * @brief  scan_files �ݹ�ɨ��sd���ڵĸ����ļ�
  * @param  path:��ʼɨ��·��
  * @retval result:�ļ�ϵͳ�ķ���ֵ
  */
static FRESULT scan_files (char* path) 
{ 
  FRESULT res; 		//�����ڵݹ���̱��޸ĵı���������ȫ�ֱ���	
  FILINFO fno; 
  DIR dir; 
  int i; 
  char *fn; 
  char file_name[FILE_NAME_LEN];	
	
#if _USE_LFN 
  static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//���ļ���֧��
  fno.lfname = lfn; 
  fno.lfsize = sizeof(lfn); 
#endif  
  res = f_opendir(&dir, path); //��Ŀ¼
  if (res == FR_OK) 
  { 
    i = strlen(path); 
    for (;;) 
    { 
      res = f_readdir(&dir, &fno); 										//��ȡĿ¼�µ�����
     if (res != FR_OK || fno.fname[0] == 0) break; 	//Ϊ��ʱ��ʾ������Ŀ��ȡ��ϣ�����
#if _USE_LFN 
      fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
      fn = fno.fname; 
#endif 
      if(strstr(path,"recorder")!=NULL)continue;       //�ӹ�¼���ļ�
      if (*fn == '.') continue; 											//���ʾ��ǰĿ¼������			
      if (fno.fattrib & AM_DIR) 
			{ 																							//Ŀ¼���ݹ��ȡ
        sprintf(&path[i], "/%s", fn); 							//�ϳ�����Ŀ¼��
        res = scan_files(path);											//�ݹ���� 
        if (res != FR_OK) 
					break; 																		//��ʧ�ܣ�����ѭ��
        path[i] = 0; 
      } 
      else 
		{ 
				//printf("%s%s\r\n", path, fn);								//����ļ���
				if(strstr(fn,".wav")||strstr(fn,".WAV")||strstr(fn,".mp3")||strstr(fn,".MP3"))//�ж��Ƿ�mp3��wav�ļ�
				{
					if ((strlen(path)+strlen(fn)<FILE_NAME_LEN)&&(file_num<MUSIC_MAX_NUM))
					{
						sprintf(file_name, "%s/%s", path, fn);						
						memcpy(playlist[file_num],file_name,strlen(file_name));
                 
						memcpy(lcdlist[file_num],fn,strlen(fn));						
						file_num++;//��¼�ļ�����
					}
				}//if mp3||wav
      }//else
     } //for
  } 
  return res; 
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
static void draw_scrollbar(HWND hwnd, HDC hdc, COLOR_RGB32 back_c, COLOR_RGB32 Page_c, COLOR_RGB32 fore_c)
{
	RECT rc;
	GetClientRect(hwnd, &rc);
	/* ���� */
	SetBrushColor(hdc, MapRGB888(hdc, back_c));
	FillRect(hdc, &rc);

	/* ������ */
	/* �߿� */
	InflateRect(&rc, 0, -rc.h >> 2);
	SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
	FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 1);

	InflateRect(&rc, -2, -2);
	SetBrushColor(hdc, MapRGB888(hdc, Page_c));
	FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 1);

	/* ���� */
	SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);

	SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
	rc.y += (rc.h >> 2) >> 1;
	rc.h -= rc.h >> 2;
	/* �߿� */
	FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
	InflateRect(&rc, -2, -2);

	SetBrushColor(hdc, MapRGB888(hdc, fore_c));
	FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
}

char *cID3V2Fra;
long  ID3V2_len = 0, lID3V2Fra_length = 0;
JPG_DEC *dec;
static HDC hdc_mem_pic;//ר��ͼƬ��Ų�
void Read_APICFrame(int item)
{
   /* file objects */
   FIL     *file;												
   FRESULT res;  
   
   UINT br;  
   char  cID3V2_head[10] = {0}, cID3V2Fra_head[10] = {0};
   
   /* �ļ�����ռ� */
   file =(FIL*)GUI_VMEM_Alloc(sizeof(FIL));
   /* ���ļ� */		
   res = f_open(file, file_name[item], FA_OPEN_EXISTING | FA_READ );  
   

   //��ǩͷ
   f_read(file, cID3V2_head, 10, &br);
	if (x_strncmp(cID3V2_head, "ID3", 3) == 0)
   {
      ID3V2_len = (cID3V2_head[6] & 0x7f) << 21
                  | (cID3V2_head[7] & 0x7f) << 14
                  | (cID3V2_head[8] & 0x7f) << 7
                  | (cID3V2_head[9] & 0x7f);
   }  
  
   //��ȡAPIC֡
   while ((f_tell(file) + 10) <= ID3V2_len)
   {
 
      rt_memset(cID3V2Fra_head, 0, 10);
      f_read(file, cID3V2Fra_head, 10, &br);
      lID3V2Fra_length = (cID3V2Fra_head[4] * 0x100000000 +
                          cID3V2Fra_head[5] * 0x10000 +
                          cID3V2Fra_head[6] * 0x100 +
                          cID3V2Fra_head[7]);
      if (x_strncmp(cID3V2Fra_head, "APIC", 4) == 0)
		{
			cID3V2Fra = (char*)GUI_VMEM_Alloc(lID3V2Fra_length);
			if (cID3V2Fra != NULL)
			{
            f_read(file, cID3V2Fra, lID3V2Fra_length, &br);
			}
			break;
		}
      else
		{
         
			f_lseek(file, file->fptr+lID3V2Fra_length);
		}
   }
   f_close(file);
   GUI_VMEM_Free(file);
}



//�����ػ�
static void button_owner_draw(DRAWITEM_HDR *ds)
{
   HDC hdc; 
   HDC hdc_mem;
   HWND hwnd;  
   RECT rc_cli;
   WCHAR wbuf[128];
	hwnd = ds->hwnd;
	hdc = ds->hDC; 
   GetClientRect(hwnd, &rc_cli);
   hdc_mem = CreateMemoryDC(SURF_ARGB4444, rc_cli.w, rc_cli.h);
   
	GetWindowText(ds->hwnd,wbuf,128); //��ð�ť�ؼ�������  
  
   SetBrushColor(hdc_mem,MapARGB(hdc_mem, 0, 255, 250, 250));
   FillRect(hdc_mem, &rc_cli);
   
   if(ds->ID == ID_BUTTON_START)
      SetFont(hdc_mem, hFont_SDCARD_100);
   else
      SetFont(hdc_mem, hFont_SDCARD);
   SetTextColor(hdc_mem, MapARGB(hdc_mem, 250,250,250,250));

	if((ds->State & BST_PUSHED) && (ds->ID == ID_BUTTON_NEXT || ds->ID == ID_BUTTON_BACK || ds->ID == ID_BUTTON_List))
	{ //��ť�ǰ���״̬
		SetTextColor(hdc_mem, MapARGB(hdc_mem, 250,70,130,180));      //��������ɫ     
	}
 
   DrawText(hdc_mem, wbuf,-1,&rc_cli,DT_VCENTER);//��������(���ж��뷽ʽ)
   
   BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
   
   DeleteDC(hdc_mem);  
}
/*
 * @brief  �Զ���ص�����
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void scrollbar_owner_draw(DRAWITEM_HDR *ds)
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
   //ClrDisplay(hdc, NULL, ARGB8888(0,205,92,92));
	hdc_mem = CreateMemoryDC(SURF_ARGB4444, rc_cli.w, rc_cli.h);
	hdc_mem1 = CreateMemoryDC(SURF_ARGB4444, rc_cli.w, rc_cli.h);

	//���ư�ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem1, ARGB8888(0,205,92,92), ARGB8888(255,250, 250, 250), ARGB8888(255,255, 255, 255));
	//������ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem, ARGB8888(0,205,92,92), ARGB8888(255,50, 205, 50), ARGB8888(255,50, 205, 50));
   SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);
	//��
	BitBlt(hdc, rc_cli.x, rc_cli.y, rc.x, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
	//��
	BitBlt(hdc, rc.x + rc.w, 0, rc_cli.w - (rc.x + rc.w), rc_cli.h, hdc_mem1, rc.x + rc.w, 0, SRCCOPY);

	//���ƻ���
	if (ds->State & SST_THUMBTRACK)//����
	{
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem, rc.x, 0, SRCCOPY);
	}
	else//δѡ��
	{
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem1, rc.x, 0, SRCCOPY);
	}
	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem1);
	DeleteDC(hdc_mem);
}

static uint16_t pic_width,pic_height;//ר��ͼƬ�ķֱ���
int music_cache = 0;
int cache = 1;
HDC hdc_mem_pic2;
int tise = 0;
static void Get_CaptureInfo_thread(void *argv)
{ 
   rt_thread_t h; 
  while (1)
  {
   /*******************��ȡר��ͼƬ******************/      
   Read_APICFrame(music_cache);   //��ȡAPIC��ǩ֡����
   JPG_DEC *dec;
   /* ����ͼƬ���ݴ���JPG_DEC���*/
   dec = JPG_Open(cID3V2Fra, lID3V2Fra_length);
   /* ��ȡͼƬ�ļ���Ϣ */
   JPG_GetImageSize(&pic_width, &pic_height,dec); 
      
   if(pic_width >= GUI_XSIZE) pic_width = GUI_XSIZE;
   if(pic_height >= GUI_YSIZE) pic_height = GUI_YSIZE;
   if(tise == 0){
   /* �����ڴ���� */
   hdc_mem_pic =CreateMemoryDC(SURF_SCREEN,pic_width,pic_height);  
   hdc_mem_pic2 = CreateMemoryDC(SURF_SCREEN,pic_width,pic_height);     
   tise = 1;
   }
   /* �������ڴ���� */
   switch(cache)
   {
      case 1:
      {
         JPG_Draw(hdc_mem_pic, 0, 0, dec); 
         break;
      }
      case 2:
      {
         JPG_Draw(hdc_mem_pic2, 0, 0, dec);
         break;
      }
   }     
   /* �ر�JPG_DEC��� */
   JPG_Close(dec); 
   GUI_VMEM_Free(cID3V2Fra);
   GUI_DEBUG("��%d֡\r", music_cache);
   /*******************��ȡר��ͼƬ******************/      
   rt_thread_suspend(h);
   rt_schedule();
  }
}
BOOL use_cache = FALSE;//�Ƿ�ʹ��Cache��ֻ���л���������1
SCROLLINFO sif;/*���û������Ĳ���*/
s32 old_scrollbar_value;//�������ľ�ֵ
int flag = 0;//������־λ
int enter_flag = 0;
HFONT hfont;
static	LRESULT	win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static HWND wnd;//���������ֵ
   static HWND wnd_power;//����icon
   RECT rc_MusicTimes = {285, 404,240,72};//����ʱ��
   RECT rc_musicname = {0,4,800,72};//��������
   static int music_item = 0;//��������
	switch (msg)
	{
   case WM_CREATE:
   {   
     
      /*********************������******************/
      sif.cbSize = sizeof(sif);
		sif.fMask = SIF_ALL;
		sif.nMin = 0;
		sif.nMax = 63;
		sif.nValue = 20;//��ʼ����ֵ
		sif.TrackSize = 30;//����ֵ
		sif.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
      wnd = CreateWindow(SCROLLBAR, L"SCROLLBAR_R", WS_OWNERDRAW|WS_TRANSPARENT | WS_VISIBLE, 
                         80, 431, 150, 30, hwnd, ID_SCROLLBAR_POWER, NULL, NULL);
      SendMessage(wnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);
      /*********************������******************/       
      wnd_power = CreateWindow(BUTTON,L"A",WS_OWNERDRAW |WS_VISIBLE,
                               music_icon[0].rc.x,music_icon[0].rc.y,
                               music_icon[0].rc.w,music_icon[0].rc.h,
                               hwnd,ID_BUTTON_Power,NULL,NULL);
      
      App_PlayMusic(hwnd);
      CreateWindow(BUTTON,L"G",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[1].rc.x,music_icon[1].rc.y,music_icon[1].rc.w,music_icon[1].rc.h,hwnd,ID_BUTTON_List,NULL,NULL);
      
      
      CreateWindow(BUTTON,L"E",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[2].rc.x,music_icon[2].rc.y,music_icon[2].rc.w,music_icon[2].rc.h,hwnd,ID_BUTTON_Equa,NULL,NULL);
      CreateWindow(BUTTON,L"B",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[3].rc.x,music_icon[3].rc.y,music_icon[3].rc.w,music_icon[3].rc.h,hwnd,ID_BUTTON_Folder,NULL,NULL);
      CreateWindow(BUTTON,L"F",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[5].rc.x,music_icon[5].rc.y,music_icon[5].rc.w,music_icon[5].rc.h,hwnd,ID_BUTTON_BACK,NULL,NULL);//ID_BUTTON_NEXT        
      CreateWindow(BUTTON,L"C",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[7].rc.x,music_icon[7].rc.y,music_icon[7].rc.w,music_icon[7].rc.h,hwnd,ID_BUTTON_NEXT,NULL,NULL);
      CreateWindow(BUTTON,L"I",WS_OWNERDRAW |WS_VISIBLE,
                   music_icon[6].rc.x,music_icon[6].rc.y,music_icon[6].rc.w,music_icon[6].rc.h,hwnd,ID_BUTTON_START,NULL,NULL);                         
      //h=rt_thread_create("Get_Info",Get_CaptureInfo_thread, NULL,1*2048,1,5);
		//rt_thread_startup(h); 
      scan_files(path);
      /*********************Ӳ��************************/

//      else
//      {
//         LISTBOX_AddString(hItem,"no music file");
//         LISTBOX_SetSel(hItem,-1);
//      }
      
      break;
   }
   /*******************�������ػ�************************/
	case WM_DRAWITEM:
	{
		DRAWITEM_HDR *ds;
		ds = (DRAWITEM_HDR*)lParam;
		if (ds->ID == ID_SCROLLBAR_POWER)
		{
			scrollbar_owner_draw(ds);
			return TRUE;
		}
      if (ds->ID >= ID_BUTTON_Power && ds->ID<= 0x1010)
		{
			button_owner_draw(ds);
			return TRUE;
		}
	}
   /*�������ƶ�������Ϣ*/
 	case WM_NOTIFY: {
		NMHDR *nr;
		u16 ctr_id;
      HWND sub_wnd;
		ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID.     
		nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.
		if (ctr_id == ID_SCROLLBAR_POWER)
		{
			NM_SCROLLBAR *sb_nr;
			sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
			switch (nr->code)
			{
				case SBN_THUMBTRACK: //R�����ƶ�
				{
					power= sb_nr->nTrackValue; //��û��鵱ǰλ��ֵ
               
               wm8978_SetOUT1Volume(power); 
					SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, power); //����λ��ֵ
					/*InvalidateRect(hwnd, NULL, TRUE);*/
				}
				break;
			}
		}
      u16 code,id;
      id  =LOWORD(wParam);
      code=HIWORD(wParam);
      HDC hdc;
      if(code == BN_CLICKED)
      { 
         switch(id)
         {
            case ID_BUTTON_Power:
            {
               music_icon[0].state = ~music_icon[0].state;
               InvalidateRect(hwnd, &music_icon[0].rc, TRUE);
               if(music_icon[0].state == FALSE)
               {
                  wm8978_OutMute(0);
                  
                  sif.nValue = power;
                  SendMessage(wnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);     
                  EnableWindow(wnd, ENABLE);
                  SetWindowText(wnd_power, L"A");
               }
               else
               {                
                  wm8978_OutMute(1);//����
                  old_scrollbar_value = SendMessage(wnd, SBM_GETVALUE, TRUE, TRUE);
                  sif.nValue = 0;
                  SendMessage(wnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);
                  EnableWindow(wnd, DISABLE);                
                  SetWindowText(wnd_power, L"J");
               }
               break;
            }
            case ID_BUTTON_START:
            {
               sub_wnd = GetDlgItem(hwnd, ID_BUTTON_START);
               music_icon[6].state = ~music_icon[6].state;
               InvalidateRect(hwnd, &music_icon[6].rc, TRUE);
               
               if(music_icon[6].state == FALSE)
               {
                  rt_thread_suspend(h_music);
                  I2S_Play_Stop();
                  SetWindowText(sub_wnd, L"I");
               }
               else
               {
                  
                  rt_thread_resume(h_music);
                  I2S_Play_Start();
                  SetWindowText(sub_wnd, L"H");
               }               
               break;
            }
            case ID_BUTTON_List:
            {
               enter_flag = 1;
               App_MusicList();
               break;
            }                    
            case ID_BUTTON_NEXT:
            {
               WCHAR wbuf[128];
               COLORREF color;
               static int i = 0;
               play_index++;
               if(play_index >= file_num) play_index = 0;
               if(play_index < 0) play_index = file_num - 1;
               mp3player.ucStatus = STA_SWITCH;
               hdc = GetDC(hwnd);
               i++;              
               color = GetPixel(hdc, 385, 404);  
               x_mbstowcs_cp936(wbuf, lcdlist[play_index], FILE_NAME_LEN);
               ClrDisplay(hdc, &rc_musicname, color);
               DrawText(hdc, wbuf, -1, &rc_musicname, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
               ClrDisplay(hdc, &rc_MusicTimes, color);
               DrawText(hdc, L"00:00 / 00:00", -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
               ReleaseDC(hwnd, hdc);
               break;
            }
            case ID_BUTTON_BACK:
            {
               play_index--;
               if(play_index > file_num) play_index = 0;
               if(play_index < 0) play_index = file_num - 1;
               mp3player.ucStatus = STA_SWITCH;   
             
               break;
            }
         }
         
      }	
      break;
	}  
   /* �ͻ���������Ҫ������ */
	case	WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam; 
      RECT rc;//���ڴ�С
      //HDC hdc_tmp;//�����
      HDC hdc_mem;//�����
      HDC hdc_pic_zoom;//ר��ͼƬ���Ų�  
		GetClientRect(hwnd, &rc); //��ÿͻ�������.
      
      
      //����ռ䣨��ͼƬ���Ƶ���һ��ͼ�㣬����͸��Բ����copy����Ļhdc��
      hdc_mem = CreateMemoryDC(SURF_ARGB4444, rc.w, rc.h);
      hdc_pic_zoom = CreateMemoryDC(SURF_SCREEN,rc.w, rc.h);       
      
      //����
      SetBrushColor(hdc_mem, MapARGB(hdc_mem,255, 205,92,92));
      FillRect(hdc_mem, &rc);  

      //͸��Բ
      SetBrushColor(hdc_mem, MapARGB(hdc_mem, 0, 255, 255, 255));
      FillCircle(hdc_mem, 400, 240, 135);
      if(use_cache == FALSE)
         /* ���ڴ�����������Ļ */
         StretchBlt( hdc_pic_zoom,250,90,310,310,
                     hdc_mem_pic,0,0,pic_width,pic_height,
                     SRCCOPY);
      else
         StretchBlt( hdc_pic_zoom,250,90,310,310,
                     hdc_mem_pic2,0,0,pic_width,pic_height,
                     SRCCOPY);         

//      BitBlt(hdc, 0, 0, rc.w, rc.h, hdc_tmp, 0, 0, SRCCOPY);
      BitBlt(hdc_pic_zoom, 0, 0, 800, 480, hdc_mem, 0, 0, SRCCOPY);  
      
      BitBlt(hdc, 0, 0, 800, 480, hdc_pic_zoom, 0, 0, SRCCOPY);
      DeleteDC(hdc_pic_zoom);
      DeleteDC(hdc_mem);     

      //DeleteDC(hdc_tmp);//�ͷ��ڴ�
		return TRUE;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		HDC hdc;//��Ļhdc
      HDC hdc_mem;//������
      //HDC hdc_text;//ͼ��/�����
		hdc = BeginPaint(hwnd, &ps);
		RECT rc_top = {0 ,0, 800, 80};//�ϱ���
		RECT rc_bot = {0 ,400, 800, 80};//�±���
      RECT rc_cli;//�ͻ�������
      WCHAR wbuf[128];
      GetClientRect(hwnd, &rc_cli);//��ȡ�ͻ���λ����Ϣ
      
      hdc_mem = CreateMemoryDC(SURF_ARGB4444, rc_cli.w, rc_cli.h);//����ռ�
//      hdc_text = CreateMemoryDC(SURF_ARGB4444, rc_cli.w, rc_cli.h);
      
      x_mbstowcs_cp936(wbuf, lcdlist[play_index], FILE_NAME_LEN);
      DrawText(hdc, wbuf, -1, &rc_musicname, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
      //DrawRect(hdc, &rc_MusicTimes);
      //SetFont(hdc, old_font);
      DrawText(hdc, L"00:00 / 00:00", -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
      //SetFont(hdc, old_font);
      //����㱳����͸����
//      SetBrushColor(hdc_text, MapARGB(hdc_text, 0, 0, 0, 0));
//		FillRect(hdc_text, &rc_cli);      
      
      /*�ϱ���Ŀ*/
		SetBrushColor(hdc_mem, MapARGB(hdc_mem, 50, 0, 0, 0));
		FillRect(hdc_mem, &rc_top);
		BitBlt(hdc, rc_top.x, rc_top.y, rc_top.w, rc_top.h, hdc_mem, rc_top.x, rc_top.y, SRCCOPY);
      /*�±���Ŀ*/
		SetBrushColor(hdc_mem, MapARGB(hdc_mem, 50, 0, 0, 0));
		FillRect(hdc_mem, &rc_bot);
		BitBlt(hdc, rc_bot.x, rc_bot.y, rc_bot.w, rc_bot.h, hdc_mem, rc_bot.x, rc_bot.y, SRCCOPY);      
               
      //DeleteDC(hdc_text);  
      DeleteDC(hdc_mem);
		EndPaint(hwnd, &ps);
            
		break;
	}   
   case WM_DESTROY:
	{
		DeleteDC(hdc_mem_pic);
      DeleteDC(hdc_mem_pic2);
      flag = 0;//�ر�
		return PostQuitMessage(hwnd);	
	}
	default:
		return	DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return	WM_NULL;
}




void	GUI_MUSICPLAYER_DIALOG(void)
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

	//����������
	hwnd = CreateWindowEx(WS_EX_NOFOCUS,
		&wcex,
		L"GUI_MUSICPLAYER_DIALOG",
		WS_VISIBLE,
		0, 0, GUI_XSIZE, GUI_YSIZE,
		NULL, NULL, NULL, NULL);

	//��ʾ������
	ShowWindow(hwnd, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
      if(mp3player.ucStatus == STA_SWITCH)
      {
         //play_index++;
         //printf("��Ҫ����\n");  
      }
      GUI_msleep(20);
	}

}
