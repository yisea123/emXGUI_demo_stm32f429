#include "emXGUI.h"
#include "GUI_RECORDER_DIALOG.h"
#include "x_libc.h"
#include "string.h"
#include "ff.h"
#include "./mp3_player/Backend_mp3Player.h"
#include "Backend_Recorder.h"

//ͼ���������
recorder_icon_t record_icon[] = {

   {L"A",        {308, 412, 48, 48},   ID_RECORD_bPOWER},    // 0. ����
   {L"Q",        {718, 407, 64, 64},   ID_RECORD_BUGLE},     // 1. ���Ȱ�ť
   {L"S",        {446, 407, 64, 64},   ID_RECORD_BACK},      // 2. ��һ��
   {L"T",        {538, 405, 72, 72},   ID_RECORD_PLAY},      // 3. ����
   {L"V",        {642, 407, 64, 64},   ID_RECORD_NEXT},      // 4. ��һ��
   {L"U",        {79,  308, 64, 64},   ID_RECORD_STOP},      // 5. ֹͣ¼��
   {L"U",        {181, 308, 64, 64},   ID_RECORD_START},     // 6. ��ʼ¼��
   {L"U",        {181, 308, 72, 72},   ID_RECORD_PADNC},     // 7. ��ͣ����
   {L"O",        {740,  12, 36, 36},   ID_RECORD_EXIT},      // 8. �˳�

   {L"¼����",    {96,  85, 120, 30},   ID_RECORD_STATE},    // 9. ����¼��
   {L"00:00",    {106, 187,100, 30},   ID_RECORD_TIME},      // 10. ¼��ʱ��
   {L"00:00",    {302, 379, 66, 30},   ID_PLAY_TIME},        // 11. ����ʱ��
   {L"00:00",    {732, 379, 66, 30},   ID_PLAY_TOTAL_TIME},  // 12. ¼����ʱ��
  
   {L" ",        {325, 47, 406,301},   ID_RECORD_LIST},      // 13. �����б�
   {L" ",        {370, 378,355, 30},   ID_PLAY_PROGRESS},    // 14. ���Ž�����
   {L" ",        {369, 423, 74, 30},   ID_RECORD_sPOWER},    // 15. ����������
  
};

static HDC hdc_bk;
//���ֲ��������
HWND	Recorer_hwnd;
TaskHandle_t h_record;    // ����������ƾ��
TaskHandle_t h_play_record;

/* ������ MUSIC �ж���ģ�����ֱ���ã������¶��壬�Խ�Լ�ڴ� */
extern char music_playlist[MUSIC_MAX_NUM][FILE_NAME_LEN] __EXRAM;  // ����List
extern char music_lcdlist[MUSIC_MAX_NUM][MUSIC_NAME_LEN] __EXRAM;  // ��ʾlist
extern uint8_t  music_file_num;                   // �ļ�����
extern char music_name[FILE_NAME_LEN] __EXRAM;    // ����������
extern int play_index;      // ���Ÿ����ı��ֵ
extern HWND music_wnd_time; // �������������ھ��
extern uint8_t chgsch;      // ������������־λ

static char path[100];     // �ļ���Ŀ¼
static int  power = 20;    // ����ֵ

/**
  * @brief  scan_files �ݹ�ɨ��sd���ڵ�¼���ļ�
  * @param  path:��ʼɨ��·��
  * @retval result:�ļ�ϵͳ�ķ���ֵ
  */
static FRESULT scan_record_files(char* path) 
{ 
  FRESULT res; 		//�����ڵݹ���̱��޸ĵı���������ȫ�ֱ���	
  FILINFO fno; 
  DIR dir; 
  int i; 
  char *fn; 
  char file_name[FILE_NAME_LEN];
	WCHAR wbuf[128];	
	
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
      //if(strstr(path,"recorder")!=NULL)continue;      // �ӹ�¼���ļ�
      if (*fn == '.') continue; 											  // ���ʾ��ǰĿ¼������			
      if (fno.fattrib & AM_DIR) 
			{ 																							  // Ŀ¼���ݹ��ȡ
        sprintf(&path[i], "/%s", fn); 							    // �ϳ�����Ŀ¼��
        res = scan_record_files(path);									// �ݹ���� 
        if (res != FR_OK) 
					break; 																	     	// ��ʧ�ܣ�����ѭ��
        path[i] = 0; 
      } 
      else 
      { 
				//printf("%s/%s\r\n", path, fn);								// ����ļ���
				if(strstr(fn,".wav")||strstr(fn,".WAV"))      // �ж��Ƿ�wav�ļ�
				{
					if ((strlen(path)+strlen(fn)<FILE_NAME_LEN)&&(music_file_num<MUSIC_MAX_NUM))
					{
						sprintf(file_name, "%s/%s", path, fn);						
						memcpy(music_playlist[music_file_num], file_name, strlen(file_name) + 1);
             //     printf("%s\r\n", music_playlist[music_file_num]);
						//memcpy(music_lcdlist[music_file_num],fn,strlen(fn));
						x_mbstowcs_cp936(wbuf, fn, sizeof(wbuf));	    // ��Ansi�ַ�ת����GUI��unicode�ַ�.
						//��Listbox������һ��Item���¼�ļ������ļ�����.
						i = SendMessage(GetDlgItem(Recorer_hwnd, ID_RECORD_LIST), LB_GETCOUNT, 0, 0);
						SendMessage(GetDlgItem(Recorer_hwnd, ID_RECORD_LIST), LB_ADDSTRING, i, (LPARAM)wbuf);
						music_file_num++;//��¼�ļ�����
					}
				}//if mp3||wav
      }//else
     } //for
  } 
  return res; 
}

/**
  * @brief  ����¼������
  * @param  hwnd:�����ھ��
  * @retval ��
  */

/* �� Backend_Recorder.c �ж��� */
extern uint8_t Isread;              /* DMA������ɱ�־ */
extern uint8_t bufflag;             /* ���ݻ�����ѡ���־ */
extern uint32_t wavsize;            /* wav��Ƶ���ݴ�С */
extern FIL record_file __EXRAM;			/* file objects */
extern char recfilename[25]; 
extern WavHead rec_wav;             /* WAV�豸  */
extern FRESULT result; 
extern UINT bw;            					/* File R/W count */
extern REC_TYPE Recorder;           /* ¼���豸 */
extern uint16_t record_buffer0[RECBUFFER_SIZE] __EXRAM;  /* ���ݻ�����1 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */
extern uint16_t record_buffer1[RECBUFFER_SIZE] __EXRAM;  /* ���ݻ�����2 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */

static void App_Record(void *p)
{
  vTaskSuspend(h_record);    // �����߳�
  
	while(1) //�߳��Ѵ�����
	{     
		/* DMA������� */
		if(Isread==1)
		{
			Isread=0;
      if(bufflag==0)
        result=f_write(&record_file,record_buffer0,RECBUFFER_SIZE*2,(UINT*)&bw);//д���ļ�							
      else
        result=f_write(&record_file,record_buffer1,RECBUFFER_SIZE*2,(UINT*)&bw);//д���ļ�
      wavsize+=RECBUFFER_SIZE*2;	
		}
    GUI_Yield();
   }
//  GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}

/**
  * @brief  ����¼������
  * @param  hwnd:�����ھ��
  * @retval ��
  */
static int thread=0;
static void App_PlayRecord(HWND hwnd)
{
  HDC hdc;
                            
  thread =1;
  vTaskSuspend(h_play_record);    // �����߳�
  
	while(thread) //�߳��Ѵ�����
	{     
    int i = 0;      
     
    i = 0;
     //�õ�������Ŀ���ļ���
    while(music_playlist[play_index][i]!='\0')
    {
      music_name[i]=music_playlist[play_index][i];
      i++;
    }
    music_name[i]='\0';
    if (play_index != SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL, 0, 0))     // ��鵱ǰҪ���ŵ���Ŀ�ǲ����Ѿ�ѡ��
    {
      SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_SETCURSEL, play_index, 0);            // ���õ�ǰ���ŵĸ���Ϊѡ��״̬
    }

    if(strstr(music_name,".wav")||strstr(music_name,".WAV"))
    {
      wavplayer(music_name, power, hdc, hwnd);
    }

    printf("���Ž���\n");

    //�����������
    GUI_msleep(20);
	   
   }
   GUI_Thread_Delete(GUI_GetCurThreadHandle()); 
}

static void exit_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
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
		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));
	}
	else
	{ //��ť�ǵ���״̬

		SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));      //���û���ɫ
	}

  SetPenSize(hdc, 2);

  InflateRect(&rc, 0, -1);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 9;
  }
}

static void stop_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

  EnableAntiAlias(hdc, TRUE);
   
  if (ds->Style & WS_DISABLED)    // �����ǽ�ֹ��
  {
    SetBrushColor(hdc, MapRGB(hdc, 200, 200, 200));
    SetPenColor(hdc, MapRGB(hdc, 200, 200, 200));
  }
  else
  {
    if (ds->State & BST_PUSHED)
    { //��ť�ǰ���״̬
      SetPenColor(hdc, MapRGB(hdc, 185, 185, 185));
      SetBrushColor(hdc, MapRGB(hdc, 185, 185, 185));      //��������ɫ
    }
    else
    { //��ť�ǵ���״̬
      SetPenColor(hdc, MapRGB(hdc, 255, 255, 255));
      SetBrushColor(hdc, MapRGB(hdc, 255, 255, 255));
    }
  }
  
  /* ��Բ�� */
  SetPenSize(hdc, 2);
  DrawCircle(hdc, rc.x+rc.w/2, rc.y+rc.h/2, rc.w/2);

  /* ����Բ�Ǿ��� */
	InflateRect(&rc, -13, -13);
  FillRoundRect(hdc, &rc, 5);
  
  EnableAntiAlias(hdc, FALSE);
}

static void start_owner_draw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HDC hdc;
	RECT rc;

	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
	rc = ds->rc;     //button�Ļ��ƾ�����.

  EnableAntiAlias(hdc, TRUE);
 
  //���ð�������ɫ
	if (ds->Style & WS_DISABLED)    // �����ǽ�ֹ��
  {
    SetBrushColor(hdc, MapRGB(hdc, 180, 20, 20));      //��������ɫ
    SetPenColor(hdc, MapRGB(hdc, 200, 200, 200));
  }
  else
  {
    if (ds->State & BST_PUSHED)
    { //��ť�ǰ���״̬
      SetBrushColor(hdc, MapRGB(hdc, 180, 20, 20));      //��������ɫ
      SetPenColor(hdc, MapRGB(hdc, 200, 200, 200));
    }
    else
    { //��ť�ǵ���״̬
      SetBrushColor(hdc, MapRGB(hdc, 240, 10, 10));
      SetPenColor(hdc, MapRGB(hdc, 255, 255, 255));
    }
  }
  
  /* ��Բ�� */
	SetPenSize(hdc, 2);
  DrawCircle(hdc, rc.x+rc.w/2, rc.y+rc.h/2, rc.w/2);

  /* ����Բ�Ǿ��� */
  FillCircle(hdc, rc.x+rc.w/2, rc.x+rc.w/2, 27);
  
  EnableAntiAlias(hdc, FALSE);
}

/***********************�ؼ��ػ溯��********************************/
/**
  * @brief  button_owner_draw ��ť�ؼ����ػ���
  * @param  ds:DRAWITEM_HDR�ṹ��
  * @retval NULL
  */

static void button_owner_draw(DRAWITEM_HDR *ds)
{
   HDC hdc; //�ؼ�����HDC
   HDC hdc_mem;//�ڴ�HDC����Ϊ������
   HWND hwnd; //�ؼ���� 
   RECT rc_cli, rc_tmp;//�ؼ���λ�ô�С����
   WCHAR wbuf[128];
	hwnd = ds->hwnd;
	hdc = ds->hDC; 
//   if(ds->ID ==  ID_BUTTON_START && show_lrc == 1)
//      return;
   //��ȡ�ؼ���λ�ô�С��Ϣ
   GetClientRect(hwnd, &rc_cli);
   //��������㣬��ʽΪSURF_ARGB4444
   hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);
   
	GetWindowText(ds->hwnd,wbuf,128); //��ð�ť�ؼ�������  
//   if(ds->ID == ID_BUTTON_Power || ds->ID == ID_BUTTON_MINISTOP){
//      SetBrushColor(hdc, color_bg);
//      FillRect(hdc, &rc_cli);
//   }
//   
//   SetBrushColor(hdc_mem,MapARGB(hdc_mem, 0, 255, 250, 250));
//   FillRect(hdc_mem, &rc_cli);

   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc_cli);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc_mem, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

   //���ż�ʹ��100*100������
   if(ds->ID == ID_RECORD_PLAY || ds->ID == ID_RECORD_PADNC)
      SetFont(hdc_mem, controlFont_72);
   else if(ds->ID == ID_RECORD_bPOWER)
      SetFont(hdc_mem, controlFont_48);
   else
      SetFont(hdc_mem, controlFont_64);

	//���ð�������ɫ
	if (ds->Style & WS_DISABLED)    // �����ǽ�ֹ��
  {
    SetTextColor(hdc_mem, MapARGB(hdc_mem, 250, 220, 220, 220));      //��������ɫ
  }
  else
  {
    if (ds->State & BST_PUSHED)
    { //��ť�ǰ���״̬
      SetTextColor(hdc_mem, MapARGB(hdc_mem, 250,105,105,105));      //��������ɫ
    }
    else
    { //��ť�ǵ���״̬
      SetTextColor(hdc_mem, MapARGB(hdc_mem, 250,250,250,250));
    }
  }
 
   DrawText(hdc_mem, wbuf,-1,&rc_cli,DT_VCENTER);//��������(���ж��뷽ʽ)
   
   BitBlt(hdc, rc_cli.x, rc_cli.y, rc_cli.w, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
   
   DeleteDC(hdc_mem);  
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
	RECT rc,rc_tmp;
   RECT rc_scrollbar;
	GetClientRect(hwnd, &rc);
	/* ���� */
   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

   rc_scrollbar.x = rc.x;
   rc_scrollbar.y = rc.h/2-1;
   rc_scrollbar.w = rc.w;
   rc_scrollbar.h = 2;
   
	SetBrushColor(hdc, MapRGB888(hdc, Page_c));
	FillRect(hdc, &rc_scrollbar);

	/* ���� */
	SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);

	SetBrushColor(hdc, MapRGB(hdc, 169, 169, 169));
	//rc.y += (rc.h >> 2) >> 1;
	//rc.h -= (rc.h >> 2);
	/* �߿� */
	//FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
	FillCircle(hdc, rc.x + rc.w / 2, rc.y + rc.h / 2, rc.h / 2 - 1);
   InflateRect(&rc, -2, -2);

	SetBrushColor(hdc, MapRGB888(hdc, fore_c));
	FillCircle(hdc, rc.x + rc.w / 2, rc.y + rc.h / 2, rc.h / 2 - 1);
   //FillRoundRect(hdc, &rc, MIN(rc.w, rc.h) >> 2);
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
   RECT rc,rc_tmp;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.

	/* ���� */
   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

	SetTextColor(hdc, MapRGB(hdc, 50, 50, 50));
	GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������
	DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
}

/*
 * @brief  �Զ��廬�������ƺ���
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

	hdc_mem = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);
	hdc_mem1 = CreateMemoryDC(SURF_SCREEN, rc_cli.w, rc_cli.h);   
         
   	
	//���ư�ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem1, 0, RGB888( 250, 250, 250), RGB888( 255, 255, 255));
	//������ɫ���͵Ĺ�����
	draw_scrollbar(hwnd, hdc_mem, 0, RGB888(	50, 205, 50), RGB888(50, 205, 50));
   SendMessage(hwnd, SBM_GETTRACKRECT, 0, (LPARAM)&rc);   

	//��
	BitBlt(hdc, rc_cli.x, rc_cli.y, rc.x, rc_cli.h, hdc_mem, 0, 0, SRCCOPY);
	//��
	BitBlt(hdc, rc.x + rc.w, 0, rc_cli.w - (rc.x + rc.w) , rc_cli.h, hdc_mem1, rc.x + rc.w, 0, SRCCOPY);

	//���ƻ���
	if (ds->State & SST_THUMBTRACK)//����
	{
      BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem1, rc.x, 0, SRCCOPY);
		
	}
	else//δѡ��
	{
		BitBlt(hdc, rc.x, 0, rc.w, rc_cli.h, hdc_mem, rc.x, 0, SRCCOPY);
	}
	//�ͷ��ڴ�MemoryDC
	DeleteDC(hdc_mem1);
	DeleteDC(hdc_mem);
}

/*
 * @brief  �ػ��б�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void listbox_owner_draw(DRAWITEM_HDR *ds)
{
	HWND hwnd;
	HDC hdc;
	RECT rc,rc_tmp;
	int i,count,cursel;
	WCHAR wbuf[128];
	hwnd =ds->hwnd;
	hdc =ds->hDC;

//	hdc =CreateMemoryDC(SURF_ARGB4444,ds->rc.w,ds->rc.h); //����һ���ڴ�DC����ͼ.

	rc =ds->rc;

   /* ���� */
   GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
   GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
   WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
   
   BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);
  
  if (!SendMessage(hwnd,LB_GETCOUNT,0,0))
  {
    /* �б�Ϊ�գ���ʾ��ʾ��ϢȻ��ֱ�ӷ��� */
    DrawText(hdc, L"��û��¼���ļ���", -1, &rc, DT_CENTER|DT_VCENTER);
    return;
  }

	i=SendMessage(hwnd,LB_GETTOPINDEX,0,0);
	count=SendMessage(hwnd,LB_GETCOUNT,0,0);
	cursel=SendMessage(hwnd,LB_GETCURSEL,0,0);

	while(i<count)
	{
		SendMessage(hwnd,LB_GETITEMRECT,i,(LPARAM)&rc);
		if(rc.y > ds->rc.h)
		{
			break;
		}

    SetTextColor(hdc, MapRGB(hdc,50,10,10));

    if(i==cursel)
    {
      SetTextColor(hdc, MapRGB(hdc, 250, 10, 10));
    }
    else
    {
      SetTextColor(hdc, MapRGB(hdc, 10, 10, 10));
    }

    /* ��ǰ���ͼ�� */
    // SetBrushColor(hdc, MapRGB(hdc, 10, 10, 10));
    // EnableAntiAlias(hdc, TRUE);
    // FillPolygon(hdc, 0, 0, pt, 5);
    // EnableAntiAlias(hdc, FALSE);

    SendMessage(hwnd, LB_GETTEXT, i, (LPARAM)wbuf);

    DrawText(hdc, wbuf, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);      // ��ʾ�б��ı�
    
    SetPenColor(hdc, MapRGB(hdc, 10, 10, 10));
    HLine(hdc, rc.x, rc.y + rc.h - 1, rc.x + rc.w);                         // ��һ����

    i++; 
  }
}

extern int SelectDialogBox(HWND hwndParent, RECT rc, const WCHAR *pText, const WCHAR *pCaption, const MSGBOX_OPTIONS *ops);
static LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   static uint8_t  BUGLE_STATE = 0;     // ����״̬
	 static uint32_t Record_Timer = 0;    // ¼����ʱ��
  
   switch(msg){
      case WM_CREATE:
      {
         u8 *jpeg_buf;
         u32 jpeg_size;
         JPG_DEC *dec;
         BOOL res = NULL;

         res = RES_Load_Content(GUI_RECORDER_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
         //res = FS_Load_Content(GUI_RECORDER_BACKGROUNG_PIC, (char**)&jpeg_buf, &jpeg_size);
         hdc_bk = CreateMemoryDC(SURF_SCREEN, GUI_XSIZE, GUI_YSIZE);
         if(res)
         {
            /* ����ͼƬ���ݴ���JPG_DEC��� */
            dec = JPG_Open(jpeg_buf, jpeg_size);

            /* �������ڴ���� */
            JPG_Draw(hdc_bk, 0, 0, dec);

            /* �ر�JPG_DEC��� */
            JPG_Close(dec);
         }
         /* �ͷ�ͼƬ���ݿռ� */
         RES_Release_Content((char **)&jpeg_buf); 

//         exit_sem = GUI_SemCreate(0,1);//����һ���ź���        

         for (uint8_t xC=0; xC<9; xC++)     // 0~7 ����ť
         {
            /* ѭ��������ť */
            CreateWindow(BUTTON, record_icon[xC].icon_name, WS_OWNERDRAW | WS_VISIBLE | WS_TRANSPARENT,
                         record_icon[xC].rc.x, record_icon[xC].rc.y,
                         record_icon[xC].rc.w,record_icon[xC].rc.h,
                         hwnd, record_icon[xC].id, NULL, NULL); 
         }
          
         ShowWindow(GetDlgItem(hwnd, ID_RECORD_PADNC), SW_HIDE);     // �������ؼ�������ͣ�İ�ť
         EnableWindow(GetDlgItem(hwnd, ID_RECORD_STOP), DISABLE);    // ����ֹͣ��ť

         for (uint8_t xC=9; xC<13; xC++)    // 8~11 ���ı���
         {
            /* ѭ�������ı��� */
            CreateWindow(TEXTBOX, record_icon[xC].icon_name, WS_OWNERDRAW | WS_VISIBLE,
                         record_icon[xC].rc.x, record_icon[xC].rc.y,
                         record_icon[xC].rc.w,record_icon[xC].rc.h,
                         hwnd, record_icon[xC].id, NULL, NULL); 
         }

         /*********************����������******************/
         SCROLLINFO sif;
         sif.cbSize = sizeof(sif);
         sif.fMask = SIF_ALL;
         sif.nMin = 0;
         sif.nMax = 255;
         sif.nValue = 0;//��ʼֵ
         sif.TrackSize = 30;//����ֵ
         sif.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������          
         music_wnd_time = CreateWindow(SCROLLBAR, record_icon[14].icon_name, WS_OWNERDRAW | WS_VISIBLE,
                           record_icon[14].rc.x, record_icon[14].rc.y, record_icon[14].rc.w,
                           record_icon[14].rc.h, hwnd, record_icon[14].id, NULL, NULL);
         SendMessage(music_wnd_time, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif);         

         /*********************����ֵ������******************/
         SCROLLINFO sif_power;
         sif_power.cbSize = sizeof(sif_power);
         sif_power.fMask = SIF_ALL;
         sif_power.nMin = 0;
         sif_power.nMax = 63;//�������ֵΪ63
         sif_power.nValue = 20;//��ʼ����ֵ
         sif_power.TrackSize = 30;//����ֵ
         sif_power.ArrowSize = 0;//���˿��Ϊ0��ˮƽ��������
         
         HWND wnd;
         wnd = CreateWindow(SCROLLBAR, record_icon[15].icon_name, WS_OWNERDRAW,
                           record_icon[15].rc.x, record_icon[15].rc.y, record_icon[15].rc.w,
                           record_icon[15].rc.h, hwnd, record_icon[15].id, NULL, NULL);
         SendMessage(wnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&sif_power);
    
         BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
         xReturn = xTaskCreate((TaskFunction_t )(void(*)(void*))App_PlayRecord,  /* ������ں��� */
                            (const char*    )"App_PlayMusic",          /* �������� */
                            (uint16_t       )5*1024/4,                   /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )hwnd,                     /* ������ں������� */
                            (UBaseType_t    )5,                        /* ��������ȼ� */
                            (TaskHandle_t  )&h_play_record);           /* ������ƿ�ָ�� */

				if(xReturn != pdPASS )
				{
					GUI_ERROR("GUI rec play Thread Create failed");
				} 
                            
        xReturn = xTaskCreate((TaskFunction_t )(void(*)(void*))App_Record,  /* ������ں��� */
                            (const char*    )"Record Task",       /* �������� */
                            (uint16_t       )4*1024/4,              /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
                            (void*          )NULL,                /* ������ں������� */
                            (UBaseType_t    )5,                   /* ��������ȼ� */
                            (TaskHandle_t  )&h_record);           /* ������ƿ�ָ�� */

				if(xReturn != pdPASS )
				{
					GUI_ERROR("GUI rec Thread Create failed");
				} 

         /* ����¼���ļ��б� */
         CreateWindow(LISTBOX, record_icon[13].icon_name, WS_OWNERDRAW |  WS_VISIBLE | LBS_NOTIFY,
                           record_icon[13].rc.x, record_icon[13].rc.y, record_icon[13].rc.w,
                           record_icon[13].rc.h, hwnd, record_icon[13].id, NULL, NULL);
         
         PostAsyncMessage(hwnd, eMSG_SCAN_FILE, 0, 0);     // ִ��һ���ļ�ɨ��

         break;
      }

      case WM_TIMER:
      {
        int tmr_id;
				WCHAR wbuf[10];

				tmr_id = wParam;    // ��ʱ�� ID

				if (tmr_id == ID_Record_Timer)    // ÿ��¼����ʱ��ʱ��
				{
					Record_Timer++;
					x_wsprintf(wbuf, L"%02d:%02d", Record_Timer / 60, Record_Timer % 60);
          SetWindowText(GetDlgItem(hwnd, ID_RECORD_TIME), wbuf);
				}
      }  
			break;     
      
      case eMSG_SCAN_FILE:
      {
        HWND wnd = GetDlgItem(hwnd, ID_RECORD_LIST);
               
        SendMessage(wnd, LB_RESETCONTENT, 0, 0);
        scan_record_files(path);
        SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_SETCURSEL, 0, 0);    // Ĭ��ѡ�е�һ��
        InvalidateRect(wnd, NULL, TRUE);
      }
      break;
      
      case WM_NOTIFY:
      {
         u16 code,  id, ctr_id;;
         id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
         code=HIWORD(wParam);//��ȡ��Ϣ������
         ctr_id = LOWORD(wParam); //wParam��16λ�Ƿ��͸���Ϣ�Ŀؼ�ID. 
         
         NMHDR *nr;        

         //���͵���
        if(code == BN_CLICKED)
        {
          switch (id)
          {
            /* �˳���ť���� */
            case ID_RECORD_EXIT:
            {
              PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
            }
            break;
            
            /* ��һ�� */
            case ID_RECORD_BACK:
            {
              int i, num;
              WCHAR wbuf[3];
              HWND  wnd = GetDlgItem(hwnd, ID_RECORD_PLAY);
              
              i = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL,0,0);     // ��õ�ǰѡ����
              num = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCOUNT,0,0);    // ���������
              SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_SETCURSEL, i == 0 ? num-1 : --i, 0);     // ������õ�ǰѡ����
            
              GetWindowText(wnd, wbuf, 3);
              if (wbuf[0] == L'T')
              {
                vTaskResume(h_play_record);            // ��ǰ�ǹ���ģ��ָ�����
                SetWindowText(wnd, L"U");
                EnableWindow(GetDlgItem(hwnd, ID_RECORD_START), DISABLE);     // ���ÿ�ʼ¼����ť
              }
                
              I2S_Play_Start();
              play_index = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL,0,0);    // ��õ�ǰѡ����
              mp3player.ucStatus = STA_SWITCH;
            }
            break;
            
            /* ��һ�� */
            case ID_RECORD_NEXT:
            {
              int i, num;
              WCHAR wbuf[3];
              HWND  wnd = GetDlgItem(hwnd, ID_RECORD_PLAY);
              
              i = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL,0,0);     // ��õ�ǰѡ����
              num = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCOUNT,0,0);    // ���������
              SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_SETCURSEL, i==(num-1) ? 0 : ++i, 0);     // ������õ�ǰѡ����
              
              GetWindowText(wnd, wbuf, 3);
              if (wbuf[0] == L'T')
              {
                vTaskResume(h_play_record);            // ��ǰ�ǹ���ģ��ָ�����
                SetWindowText(wnd, L"U");
                EnableWindow(GetDlgItem(hwnd, ID_RECORD_START), DISABLE);     // ���ÿ�ʼ¼����ť
              }
                
              I2S_Play_Start();
              play_index = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL,0,0);    // ��õ�ǰѡ����
              mp3player.ucStatus = STA_SWITCH;
            }
            break;
            
            /* ����¼�� */
            case ID_RECORD_PLAY:
            {
							WCHAR wbuf[3];
              HWND  wnd = GetDlgItem(hwnd, ID_RECORD_PLAY);
              
							GetWindowText(wnd, wbuf, 3);
              
							if (wbuf[0] == L'U')
							{
                vTaskSuspend(h_play_record);
                I2S_Play_Stop(); 
								SetWindowText(wnd, L"T");
								EnableWindow(GetDlgItem(hwnd, ID_RECORD_START), ENABLE);      // ʹ�ܿ�ʼ¼����ť
							}
							else
							{
                vTaskResume(h_play_record);
                I2S_Play_Start();
								SetWindowText(wnd, L"U");
								EnableWindow(GetDlgItem(hwnd, ID_RECORD_START), DISABLE);     // ���ÿ�ʼ¼����ť
							}
            }
            break;
            
            /* ��ʼ¼�� */
            case ID_RECORD_START:
						{
              int i = 0;
              DIR dir;
              
              /*  ���·�������ڣ������ļ���  */
              result = f_opendir(&dir,RECORDERDIR);
              while(result != FR_OK)
              {
                i++;
                if (i > 5)
                {
                  /* ��ʼ��ʧ�� */
                  RECT RC;
                  MSGBOX_OPTIONS ops;
                  const WCHAR *btn[] = { L"ȷ��",L"ȡ��" };      //�Ի����ڰ�ť������

                  ops.Flag = MB_ICONERROR;
                  ops.pButtonText = btn;
                  ops.ButtonCount = 2;
                  RC.w = 300;
                  RC.h = 200;
                  RC.x = (GUI_XSIZE - RC.w) >> 1;
                  RC.y = (GUI_YSIZE - RC.h) >> 1;
                  SelectDialogBox(hwnd, RC, L"û�м�⵽SD��\n��ȷ��SD�Ѳ��롣", L"����", &ops);    // ��ʾ������ʾ��
                  break;
                }
                f_mkdir(RECORDERDIR);
                result = f_opendir(&dir,RECORDERDIR);
              }
              
              if (i >= 5)break;
              
              /* Ѱ�Һ����ļ��� */
              for(i=1;i<0xff;++i)
              {
                sprintf(recfilename,"0:/recorder/rec%03d.wav",i);
                result=f_open(&record_file,(const TCHAR *)recfilename,FA_READ);
                if(result==FR_NO_FILE)break;					
              }
              f_close(&record_file);
              
              if(i==0xff)
              {
                Recorder.ucStatus =STA_ERR;
                break;
              }
              /* ��ʼ¼�� */
              vTaskResume(h_record);            // �ָ�¼������
              StartRecord(recfilename);
							SetTimer(hwnd, ID_Record_Timer, 1000, TMR_START, NULL);                    // ����¼����ʱ��ʱ��
							/* ��ť���� */
              SetWindowText(GetDlgItem(hwnd, ID_RECORD_BUGLE), L"Q");
							SetWindowText(GetDlgItem(hwnd, ID_RECORD_STATE), L"����¼��");             // ����¼����״̬
							ShowWindow(GetDlgItem(hwnd, ID_RECORD_START), SW_HIDE);                    // �������ؿ�ʼ¼���İ�ť
							ShowWindow(GetDlgItem(hwnd, ID_RECORD_PADNC), SW_SHOW);                    // ������ʾ��������ͣ�İ�ť
							SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_LOCKCURSEL, TRUE, 0);     // �����б��������޸�ѡ�е���
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_PLAY), DISABLE);                   // ���ò��Ű�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_BACK), DISABLE);                   // ������һ�װ�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_NEXT), DISABLE);                   // ������һ�װ�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_STOP), ENABLE);                    // ʹ��ֹͣ¼����ť
            }
            break;

            case ID_RECORD_STOP:
            {
              vTaskSuspend(h_play_record);            // ����¼������
              KillTimer(hwnd, ID_Record_Timer);       // ɾ��¼����ʱ��ʱ��
              Record_Timer = 0;
              SetWindowText(GetDlgItem(hwnd, ID_RECORD_TIME), L"00:00");
              /* ����¼������Ҫ��WAV�ļ������������ */
              if(Recorder.ucStatus == STA_RECORDING)
              {
                I2Sxext_Recorde_Stop();
                I2S_Play_Stop();
                rec_wav.size_8=wavsize+36;
                rec_wav.datasize=wavsize;
                result=f_lseek(&record_file,0);
                result=f_write(&record_file,(const void *)&rec_wav,sizeof(rec_wav),&bw);
                result=f_close(&record_file);
                printf("¼������\r\n");
              }
              //ucRefresh = 1;
              Recorder.ucStatus = STA_IDLE;		/* ����״̬ */
              I2S_Stop();		/* ֹͣI2S¼���ͷ��� */
              wm8978_Reset();	/* ��λWM8978����λ״̬ */
              
              /* �ļ���¼���� */		
              char *cbuf = NULL;
              WCHAR wbuf[128];
              int i = 0;
              memcpy(music_playlist[music_file_num], recfilename, strlen(recfilename) + 1);
              printf("%s\r\n", music_playlist[music_file_num]);
              cbuf = strstr(recfilename, "/");    // ������һ�� / 
              cbuf = strstr(cbuf+1, "/");         // �����ڶ��� /
              if (cbuf != NULL)
              { 
              }
              x_mbstowcs_cp936(wbuf, cbuf + 1, sizeof(wbuf));
              //��Listbox������һ��Item��
              i = SendMessage(GetDlgItem(Recorer_hwnd, ID_RECORD_LIST), LB_GETCOUNT, 0, 0);
              SendMessage(GetDlgItem(Recorer_hwnd, ID_RECORD_LIST), LB_ADDSTRING, i, (LPARAM)wbuf);
              music_file_num++;//��¼�ļ�����
              InvalidateRect(GetDlgItem(Recorer_hwnd, ID_RECORD_LIST), NULL, TRUE);    // ��Ч���б�ʹ���ػ�
              
              /* ֹͣ¼�� */
							SetWindowText(GetDlgItem(hwnd, ID_RECORD_STATE), L"¼����");                // ����¼����״̬
							SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_LOCKCURSEL, FALSE, 0);     // �����б���������޸�ѡ�е���
							ShowWindow(GetDlgItem(hwnd, ID_RECORD_START), SW_SHOW);                     // ��ʾ��ʼ¼���İ�ť
							ShowWindow(GetDlgItem(hwnd, ID_RECORD_PADNC), SW_HIDE);                     // ���ؼ�������ͣ�İ�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_BACK), ENABLE);                     // ������һ�װ�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_NEXT), ENABLE);                     // ������һ�װ�ť
              EnableWindow(GetDlgItem(hwnd, ID_RECORD_PLAY), ENABLE);                     // ʹ�ܲ��Ű�ť
							EnableWindow(GetDlgItem(hwnd, ID_RECORD_STOP), DISABLE);                    // ����ֹͣ¼����ť
            }
            break;

						/* ��ͣ����¼����ť */
            case ID_RECORD_PADNC:
            {
							WCHAR wbuf[3];
              HWND  wnd = GetDlgItem(hwnd, ID_RECORD_PADNC);
              
							GetWindowText(wnd, wbuf, 3);
							if (wbuf[0] == L'U')
							{
								KillTimer(hwnd, ID_Record_Timer);       // ɾ��¼����ʱ��ʱ��
                vTaskSuspend(h_record);    // ����¼������
                I2Sxext_Recorde_Stop();    // ֹͣ¼��
								SetWindowText(GetDlgItem(hwnd, ID_RECORD_STATE), L"����ͣ");          // ����¼����״̬
								SetWindowText(wnd, L"T");
							}
							else
							{
								SetTimer(hwnd, ID_Record_Timer, 1000, TMR_START, NULL);               // ����¼����ʱ��ʱ��
                vTaskResume(h_record);      // �ָ�¼������
                I2Sxext_Recorde_Start();    // ����¼��
								SetWindowText(GetDlgItem(hwnd, ID_RECORD_STATE), L"����¼��");         // ����¼����״̬
								SetWindowText(wnd, L"U");
							}
            }
            break;
            
            /* ��Ƶ���ѡ��ť */
            case ID_RECORD_BUGLE:
            {
							WCHAR wbuf[3];
              HWND  wnd = GetDlgItem(hwnd, ID_RECORD_BUGLE);
              
							GetWindowText(wnd, wbuf, 3);
							if (wbuf[0] == L'P')
							{
								SetWindowText(wnd, L"Q");
                wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);    // ����Ϊ�������
							}
							else
							{
								SetWindowText(wnd, L"P");
                wm8978_CfgAudioPath(DAC_ON, SPK_ON);                        // ����Ϊ���������
							}
            }
            break;
            
            /* �������ڰ�ť������ */
            case ID_RECORD_bPOWER:
            {
							if (BUGLE_STATE == 1)
							{
                BUGLE_STATE = 0;
								ShowWindow(GetDlgItem(hwnd, ID_RECORD_sPOWER), SW_HIDE); //��������
							}
							else
							{
                BUGLE_STATE = 1;
								ShowWindow(GetDlgItem(hwnd, ID_RECORD_sPOWER), SW_SHOW); //������ʾ
							}
            }
            break;
            
          }
        }
        else if (LBN_SELCHANGE == code)   // �б�ѡ����ı�
        {
          if (id == ID_RECORD_LIST)
          {
            
            WCHAR wbuf[3];
            HWND  wnd = GetDlgItem(hwnd, ID_RECORD_PLAY);
            
            GetWindowText(wnd, wbuf, 3);
            if (wbuf[0] == L'T')
            {
              vTaskResume(h_play_record);            // ��ǰ�ǹ���ģ��ָ�����
              SetWindowText(wnd, L"U");
						  EnableWindow(GetDlgItem(hwnd, ID_RECORD_START), DISABLE);     // ���ÿ�ʼ¼����ť
            }
              
            I2S_Play_Start();
            play_index = SendMessage(GetDlgItem(hwnd, ID_RECORD_LIST), LB_GETCURSEL,0,0);    // ��õ�ǰѡ����
            mp3player.ucStatus = STA_SWITCH;
          }
        }
         
         /* ���������� */
          nr = (NMHDR*)lParam; //lParam����������NMHDR�ṹ�忪ͷ.
          //����������case
          if (ctr_id == ID_RECORD_sPOWER)
          {
             NM_SCROLLBAR *sb_nr;
             sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
             static int ttt = 0;
             switch (nr->code)
             {
                case SBN_THUMBTRACK: //R�����ƶ�
                {
                   uint16_t power;
                   power= sb_nr->nTrackValue; //�õ���ǰ������ֵ
                   if(power == 0) 
                   {
                      wm8978_OutMute(1);//����
                      SetWindowText(GetDlgItem(hwnd, ID_RECORD_bPOWER), L"J");
                      ttt = 1;
                   }
                   else
                   {
                      if(ttt == 1)
                      {
                         ttt = 0;
                         SetWindowText(GetDlgItem(hwnd, ID_RECORD_bPOWER), L"A");
                      }
                       wm8978_OutMute(0);
                       wm8978_SetOUT1Volume(power);//����WM8978������ֵ
                   } 
                   SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, power); //����SBM_SETVALUE����������ֵ
                }
                break;
             }
          }

          //����������case
          if (ctr_id == ID_PLAY_PROGRESS)
          {
             NM_SCROLLBAR *sb_nr;
             int i = 0;
             sb_nr = (NM_SCROLLBAR*)nr; //Scrollbar��֪ͨ��Ϣʵ��Ϊ NM_SCROLLBAR��չ�ṹ,���渽���˸������Ϣ.
             switch (nr->code)
             {
                case SBN_THUMBTRACK: //R�����ƶ�
                {
                   i = sb_nr->nTrackValue; //��û��鵱ǰλ��ֵ                
                   SendMessage(nr->hwndFrom, SBM_SETVALUE, TRUE, i); //���ý���ֵ
                   //��λ���������λ��
                   chgsch = 1;
                }
                break;
             }
          }         
      
      break;
    }

      //�ػ��ƺ�����Ϣ
      case WM_DRAWITEM:
      {
         DRAWITEM_HDR *ds;
         ds = (DRAWITEM_HDR*)lParam;        
         if(ds->ID == ID_RECORD_EXIT)
         {
            exit_owner_draw(ds);
            return TRUE;
         }
         else if (ds->ID == ID_RECORD_STOP)
         {
            stop_owner_draw(ds);    // �ػ�ֹͣ¼����ť
            return TRUE;
         }
         else if (ds->ID == ID_RECORD_START)
         {
            start_owner_draw(ds);    // �ػ濪ʼ¼����ť
            return TRUE;
         }
         else if (ds->ID == ID_RECORD_sPOWER || ds->ID == ID_PLAY_PROGRESS)   
         {
            scrollbar_owner_draw(ds);    // �������ػ�
            return TRUE;
         }
         else if (ds->ID >= ID_RECORD_bPOWER && ds->ID<= ID_RECORD_PADNC)
         {
            button_owner_draw(ds);    // ��ť�ػ�
            return TRUE;
         }
         else if (ds->ID >= ID_RECORD_STATE && ds->ID<= ID_PLAY_TIME)
         {
            Brigh_Textbox_OwnerDraw(ds);    // �ػ�͸���ı�
            return TRUE;
         }
         else if (ds->ID == ID_RECORD_LIST)
         {
            listbox_owner_draw(ds);    // �ػ��б�
            return TRUE;
         }
         return FALSE;
      }     
      
      //���ƴ��ڽ�����Ϣ
      case WM_PAINT:
      {
        PAINTSTRUCT ps;
        HDC hdc;//��Ļhdc
        RECT rc = {471, 13, 115, 34};

        //��ʼ����
        hdc = BeginPaint(hwnd, &ps); 

        DrawText(hdc, L"¼���ļ�", -1, &rc, NULL);//��������(���ж��뷽ʽ)

        EndPaint(hwnd, &ps);
        break;
      }
      
      case WM_ERASEBKGND:
      {
         HDC hdc =(HDC)wParam;
         RECT rc =*(RECT*)lParam;
         //GetClientRect(hwnd, &rc_cli);//��ȡ�ͻ���λ����Ϣ
         SetBrushColor(hdc, MapRGB(hdc, 250,0,0));
         FillRect(hdc, &rc); 
         
         BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc.x, rc.y, SRCCOPY);         
 
         return TRUE;
      }

      //�رմ�����Ϣ����case
      case WM_CLOSE:
      {        
        Record_Timer = 0;     // ��λ¼����ʱ
        mp3player.ucStatus = STA_IDLE;
        DestroyWindow(hwnd);
        return TRUE;	
      }
    
      //�رմ�����Ϣ����case
      case WM_DESTROY:
      {        
        BUGLE_STATE = 0;
        DeleteDC(hdc_bk);
        vTaskDelete(h_record);
        vTaskDelete(h_play_record);
        music_file_num = 0;   // ��λ�ļ���¼
        thread = 0;
        I2S_Stop();		        /* ֹͣI2S¼���ͷ��� */
        wm8978_Reset();	      /* ��λWM8978����λ״̬ */ 

        return PostQuitMessage(hwnd);		
      }
      
      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
     
   return WM_NULL;
}


void GUI_RECORDER_DIALOG(void)
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
	Recorer_hwnd = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                                    &wcex,
                                    L"GUI RECORDER DIALOG",
                                    WS_VISIBLE|WS_CLIPCHILDREN|WS_OVERLAPPED,
                                    0, 0, GUI_XSIZE, GUI_YSIZE,
                                    NULL, NULL, NULL, NULL);

	//��ʾ������
	ShowWindow(Recorer_hwnd, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, Recorer_hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


