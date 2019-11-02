/*
******************************************************************************
* @file    recorder.c
* @author  fire
* @version V1.0
* @date    2015-xx-xx
* @brief   WM8978�������ܲ���+mp3����
******************************************************************************
* @attention
*
* ʵ��ƽ̨:����  STM32 F429 ������  
* ��̳    :http://www.firebbs.cn
* �Ա�    :https://fire-stm32.taobao.com
*
******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "Bsp/usart/bsp_debug_usart.h"
//#include "Bsp/systick/bsp_SysTick.h"
#include "Bsp/wm8978/bsp_wm8978.h"
#include "ff.h" 
#include "./mp3_player/Backend_mp3Player.h"
#include "mp3dec.h"
#include "./mp3_player/Backend_musiclist.h"
#include "emXGUI.h"
#include "x_libc.h"
#include "./mp3_player/GUI_MUSICPLAYER_DIALOG.h"
#include "GUI_RECORDER_DIALOG.h"

#define Delay_ms GUI_msleep
/* �Ƽ�ʹ�����¸�ʽmp3�ļ���
 * �����ʣ�44100Hz
 * ��  ����2
 * �����ʣ�320kbps
 */

/* ������������Ƶ����ʱ�������������Ҫ������СΪ2304*16/8�ֽ�(16ΪPCM����Ϊ16λ)��
 * �������Ƕ���MP3BUFFER_SIZEΪ2304��ʵ�����������ΪMP3BUFFER_SIZE*2���ֽ�
 */
#define MP3BUFFER_SIZE  2304		
#define INPUTBUF_SIZE   3000	

static HMP3Decoder		Mp3Decoder;			/* mp3������ָ��	*/
static MP3FrameInfo		Mp3FrameInfo;		/* mP3֡��Ϣ  */
MP3_TYPE mp3player;/* mp3�����豸 */
static uint8_t Isread=0;           /* DMA������ɱ�־ */
static uint8_t bufflag=0;          /* ���ݻ�����ѡ���־ */
extern HFONT DEFAULT_FONT;
uint8_t inputbuf[INPUTBUF_SIZE]  __EXRAM;        /* �������뻺������1940�ֽ�Ϊ���MP3֡��С  */
static short outbuffer[2][MP3BUFFER_SIZE] __EXRAM;  /* ���������������Ҳ��I2S�������ݣ�ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */

FIL file __EXRAM;											/* file objects */
FRESULT result; 
UINT bw;            					/* File R/W count */
/*wav������*/
REC_TYPE Recorder;          /* ¼���豸 */
uint16_t buffer0[RECBUFFER_SIZE] __EXRAM;  /* ���ݻ�����1 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */
uint16_t buffer1[RECBUFFER_SIZE] __EXRAM;  /* ���ݻ�����2 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */
static WavHead rec_wav;            /* WAV�豸  */


uint32_t g_FmtList[FMT_COUNT][3] =
{
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_8k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_16k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_22k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_44k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_96k},
	{I2S_Standard_Phillips, I2S_DataFormat_16b, I2S_AudioFreq_192k},
};
RECT rc_MusicTimes = {285, 404,240,72};//����ʱ��
RECT rc_cli = {0, 380, 800, 20};//������
RECT rc_musicname = {180,4,400,72};//��������


const uint16_t recplaybuf[4]={0X0000,0X0000};//2��16λ����,����¼��ʱI2S Master����.ѭ������0.
/* �������ļ��ڵ��õĺ������� */
void MP3Player_I2S_DMA_TX_Callback(void);
extern int enter_flag;
//extern unsigned char music_lcdlist[MUSIC_MAX_NUM][MUSIC_NAME_LEN];
/**
  * @brief  ��ȡMP3ID3V2�ļ�ͷ�Ĵ�С
  * @param  ����MP3�ļ���ͷ�����ݣ�����10���ֽ�
  * @retval ID3V2�Ĵ�С
  */
uint32_t mp3_GetID3V2_Size(unsigned char *buf)
{
 uint32_t ID3V2_size;
	
 if(buf[0] == 'I' && buf[1] == 'D' && buf[2] =='3')//����ID3V2
 {
 	 ID3V2_size = (buf[6]<<21) | (buf[7]<<14) | (buf[8]<<7) | buf[9];
 }
 else//������ID3V2
	 ID3V2_size = 0;

 return ID3V2_size;

}

/**
  * @brief   MP3��ʽ��Ƶ����������
  * @param  mp3file��MP3�ļ���
  * @param  vol������
  * @param  hdc����Ļ��ͼ������ 
  * @retval ��
  */
uint8_t NUM = 0;
static uint16_t curtime,alltime;//��ʵĵ�ǰ��ʱ���Լ���ʱ�䳤��
void mp3PlayerDemo(HWND hwnd, const char *mp3file, uint8_t vol, uint8_t vol_horn, HDC hdc)
{
	uint8_t *read_ptr=inputbuf;
	uint32_t frames=0;//������֡����26msһ֡��
   static uint8_t timecount = 0;
   DWORD pos;//��¼���ֱ���
	int err=0, i=0, outputSamps=0;	
   uint32_t time_sum = 0; //���㵱ǰ�Ѳ��ŵ���ʱ��λ��
	int	read_offset = 0;				/* ��ƫ��ָ�� */
	int	bytes_left = 0;					/* ʣ���ֽ��� */	
	uint16_t frame_size;//MP3֡�Ĵ�С
	uint32_t ID3V2_size;//MP3��ID3V2�Ĵ�С
   static uint8_t lyriccount=0;//���index��¼
   WCHAR wbuf[128];//�����ı�����
	mp3player.ucFreq=I2S_AudioFreq_Default;
	mp3player.ucStatus=STA_IDLE;
	mp3player.ucVolume = vol;//���� WM8978������ֵ
   int ooo = 0;
  NUM++;
	result=f_open(&file,mp3file,FA_READ);
	if(result!=FR_OK)
	{
		printf("Open mp3file :%s fail!!!->%d\r\n",mp3file,result);
		result = f_close (&file);
    if(time2exit == 1)
    {
      lyriccount=0;  
      GUI_SemPost(exit_sem);
    }
		return;	/* ֹͣ���� */
	}
	printf("��ǰ�����ļ� -> %s\n",mp3file);
	
	//��ʼ��MP3������
	Mp3Decoder = MP3InitDecoder();	
	if(Mp3Decoder==0)
	{
		printf("��ʼ��helix������豸\n");
		return;	/* ֹͣ���� */
	}
	printf("��ʼ����...\n");
	
	Delay_ms(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
	wm8978_Reset();		/* ��λWM8978����λ״̬ */

   WCHAR wbuf1[3];
   HWND  wnd = GetDlgItem(hwnd, ID_BUTTON_BUGLE);
   
   GetWindowText(wnd, wbuf1, 3);
   if (wbuf1[0] == L'P')    // �жϵ�ǰ
   {
      wm8978_CfgAudioPath(DAC_ON, SPK_ON);                        // ����Ϊ���������
   }
   else
   {
      wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);    // ����Ϊ�������
   }
	// /* ����WM8978оƬ������ΪDAC�����Ϊ���� */
	// wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);

	/* ����������������ͬ���� */
	wm8978_SetOUT1Volume(mp3player.ucVolume);
   
  wm8978_SetOUT1Volume(vol_horn);    // ��������������ֵ

	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
	wm8978_CfgAudioIF(I2S_Standard_Phillips, 16);
	
	/*  ��ʼ��������I2S  */
	I2S_Stop();
	I2S_GPIO_Config();
	I2Sx_Mode_Config(I2S_Standard_Phillips,I2S_DataFormat_16b,mp3player.ucFreq);	
	I2S_DMA_TX_Callback=MP3Player_I2S_DMA_TX_Callback;
	I2Sx_TX_DMA_Init((uint16_t *)outbuffer[0],(uint16_t *)outbuffer[1],MP3BUFFER_SIZE);	
	
	bufflag=0;
	Isread=0;
	
	mp3player.ucStatus = STA_PLAYING;		/* ����״̬ */
   result=f_read(&file,inputbuf,INPUTBUF_SIZE,&bw);
	if(result!=FR_OK)
	{
		printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
		MP3FreeDecoder(Mp3Decoder);
		return;
	}

   //��ȡID3V2�Ĵ�С����ƫ������λ��
	ID3V2_size = mp3_GetID3V2_Size(inputbuf);
	f_lseek(&file,ID3V2_size);	
	result=f_read(&file,inputbuf,INPUTBUF_SIZE,&bw);
	if(result!=FR_OK)
	{
		printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
		MP3FreeDecoder(Mp3Decoder);
		return;
	}   
	x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
   SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB5), wbuf);    
	read_ptr=inputbuf;
	bytes_left=bw;
   
   if(vol != 0)
   {
      wm8978_OutMute(0);
   }
   //������icon������ʱ������Ϊ����ģʽ
   else
   {                
      wm8978_OutMute(1);//����
   }
            
   
	/* ����������ѭ���� */
	while(mp3player.ucStatus == STA_PLAYING)
	{
		//Ѱ��֡ͬ�������ص�һ��ͬ���ֵ�λ��
		read_offset = MP3FindSyncWord(read_ptr, bytes_left);			
		//û���ҵ�ͬ����
		if(read_offset < 0)																		
		{
			result=f_read(&file,inputbuf,INPUTBUF_SIZE,&bw);
			if(result!=FR_OK)
			{
				printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
				break;
			}
			read_ptr=inputbuf;
			bytes_left=bw;
			continue;
		}
		
		read_ptr += read_offset;					//ƫ����ͬ���ֵ�λ��
		bytes_left -= read_offset;				//ͬ����֮������ݴ�С	
		if(bytes_left < 1024)							//��������
		{
			/* ע������ط���Ϊ���õ���DMA��ȡ������һ��Ҫ4�ֽڶ���  */
			i=(uint32_t)(bytes_left)&3;									//�ж϶�����ֽ�
			if(i) i=4-i;														//��Ҫ������ֽ�
			memcpy(inputbuf+i, read_ptr, bytes_left);	//�Ӷ���λ�ÿ�ʼ����
			read_ptr = inputbuf+i;										//ָ�����ݶ���λ��
			//��������
			result = f_read(&file, inputbuf+bytes_left+i, INPUTBUF_SIZE-bytes_left-i, &bw);
			bytes_left += bw;										//��Ч��������С
		}
		//��ʼ���� ������mp3����ṹ�塢������ָ�롢��������С�������ָ�롢���ݸ�ʽ
		err = MP3Decode(Mp3Decoder, &read_ptr, &bytes_left, outbuffer[bufflag], 0);	
      time_sum +=26;//ÿ֡26ms      
		frames++;	
		//������
		if (err != ERR_MP3_NONE)									
		{
			switch (err)
			{
				case ERR_MP3_INDATA_UNDERFLOW:
					printf("ERR_MP3_INDATA_UNDERFLOW\r\n");
					result = f_read(&file, inputbuf, INPUTBUF_SIZE, &bw);
					read_ptr = inputbuf;
					bytes_left = bw;
					break;		
				case ERR_MP3_MAINDATA_UNDERFLOW:
					/* do nothing - next call to decode will provide more mainData */
					printf("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
					break;		
				default:
					printf("UNKNOWN ERROR:%d\r\n", err);		
					// ������֡
					if (bytes_left > 0)
					{
						bytes_left --;
						read_ptr ++;
					}	
					break;
			}
			Isread=1;
		}
		else		//�����޴���׼�������������PCM
		{
			MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);		//��ȡ������Ϣ				
			/* �����DAC */
			outputSamps = Mp3FrameInfo.outputSamps;							//PCM���ݸ���
			if (outputSamps > 0)
			{
				if (Mp3FrameInfo.nChans == 1)	//������
				{
					//������������Ҫ����һ�ݵ���һ������
					for (i = outputSamps - 1; i >= 0; i--)
					{
						outbuffer[bufflag][i * 2] = outbuffer[bufflag][i];
						outbuffer[bufflag][i * 2 + 1] = outbuffer[bufflag][i];
					}
					outputSamps *= 2;
				}//if (Mp3FrameInfo.nChans == 1)	//������
			}//if (outputSamps > 0)
			
			/* ���ݽ�����Ϣ���ò����� */
			if (Mp3FrameInfo.samprate != mp3player.ucFreq)	//������ 
			{
				mp3player.ucFreq = Mp3FrameInfo.samprate;
				
            //����ÿ֡�Ĵ�С
            frame_size = (((Mp3FrameInfo.version == MPEG1)? 144:72)*Mp3FrameInfo.bitrate)/Mp3FrameInfo.samprate+Mp3FrameInfo.paddingBit;
            
            //���ݹ�ʽ�������ʱ��
            alltime=(((file.fsize-ID3V2_size-128)/frame_size)*26+1000)/1000;
            //������������б�Ͳ���ʾʱ��
            if(enter_flag == 0){
               //��ȡ��Ļ��385��404������ɫ              
               x_wsprintf(wbuf, L"%02d:%02d",alltime/60,alltime%60);
               SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB1), wbuf);                
               x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
               SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB5), wbuf);                
               //���rc_MusicTimes���ε�����
               //ClrDisplay(hdc, &rc_MusicTimes, color);
               //�����ı�
               //DrawText(hdc, wbuf, -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            }
				printf(" \r\n Bitrate       %dKbps", Mp3FrameInfo.bitrate/1000);
				printf(" \r\n Samprate      %dHz", mp3player.ucFreq);
				printf(" \r\n BitsPerSample %db", Mp3FrameInfo.bitsPerSample);
				printf(" \r\n nChans        %d", Mp3FrameInfo.nChans);
				printf(" \r\n Layer         %d", Mp3FrameInfo.layer);
				printf(" \r\n Version       %d", Mp3FrameInfo.version);
				printf(" \r\n OutputSamps   %d", Mp3FrameInfo.outputSamps);
				printf("\r\n");
				//I2S_AudioFreq_Default = 2��������֡��ÿ�ζ�Ҫ������
				if(mp3player.ucFreq >= I2S_AudioFreq_Default)	
				{
					//���ݲ������޸�I2S����
					I2Sx_Mode_Config(I2S_Standard_Phillips,I2S_DataFormat_16b,mp3player.ucFreq);
					I2Sx_TX_DMA_Init((uint16_t *)outbuffer[0],(uint16_t *)outbuffer[1],outputSamps);
				}
				I2S_Play_Start();
			}
		}//else ��������
		
		if(file.fptr==file.fsize) 		//mp3�ļ���ȡ��ɣ��˳�
		{
         //�����и�״̬
         mp3player.ucStatus=STA_SWITCH;
         //������Ŀ����1
         play_index++;
         //printf("%d, %d\n", play_index, music_file_num);
         //����Ϊ�б�ѭ������
         if(play_index >= music_file_num) play_index = 0;
         if(play_index < 0) play_index = music_file_num - 1;
         //����ػ��ƽ�������ֵ
         InvalidateRect(MusicPlayer_hwnd, &rc_cli, TRUE);
         SendMessage(music_wnd_time, SBM_SETVALUE, TRUE, 0); //����λ��ֵ
         //�����ʼ���
         lyriccount=0;
         I2S_Stop();   
         MP3FreeDecoder(Mp3Decoder);
         f_close(&file);	
			break;
		}	

		while(Isread==0)
		{
         //����δ����
         if(chgsch == 0)
         {
            if(timecount>=20)
            {
               //��ǰֵ
               curtime = time_sum/1000; 
               //������������б�����ʾ
               if(enter_flag == 0)
               {
                  //�������ʱ����ʾ�͸�����ֵ���ʾ
                  //ClrDisplay(hdc, &rc_MusicTimes, color);
                  //ClrDisplay(hdc, &rc_musicname, color);
                  //���ַ�����ת��Ϊ���ַ�����
                   //x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
//                  DrawText(hdc, wbuf, -1, &rc_musicname, DT_SINGLELINE | DT_CENTER | DT_VCENTER);//��������
                  //������ʱ���ʽ�������wbuf
//                  x_wsprintf(wbuf, L"%02d:%02d",curtime/60,curtime%60);
//                  DrawText(hdc, wbuf, -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);//��������
                  if(ooo == 0)//ȷ��ֻ��ˢ��һ��
                  {
                     x_wsprintf(wbuf, L"%02d:%02d",alltime/60,alltime%60);
                     SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB1), wbuf);                
                     x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
                     SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB5), wbuf);       
                     ooo=1;
                  }
                  x_wsprintf(wbuf, L"%02d:%02d",curtime/60,curtime%60);
                  SetWindowText(GetDlgItem(MusicPlayer_hwnd, ID_TB2), wbuf);                       
                  //���½�����
                  SendMessage(music_wnd_time, SBM_SETVALUE, TRUE, curtime*255/alltime);
                  InvalidateRect(MusicPlayer_hwnd, &rc_cli, FALSE);   
                  
                  
                  lrc.curtime = curtime;  
                  if(lrc.flag == 1){
                     //+100����ǰ��ʾ����ʾ��Ҫ����һ��ʱ��
                     if((lrc.oldtime <= lrc.curtime*100+100)&&(lrc.indexsize>7))
                     {
                        //��ʾ��ǰ�еĸ��
                        x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount]-1], LYRIC_MAX_SIZE);
                        SetWindowText(wnd_lrc3,wbuf);
                        //��ʾ��i-1�еĸ�ʣ�ǰһ�У�
                        if(lyriccount>0)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-1]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc2,wbuf);
                        }
                        else
                           SetWindowText(wnd_lrc2,L" ");
                        //��ʾ��i-2�еĸ�ʣ�ǰ���У�
                        if(lyriccount>0)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-2]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc1,wbuf);
                        }
                        else
                           SetWindowText(wnd_lrc1,L" ");
                        //��ʾ��i+1�еĸ�ʣ���һ�У�   
                        if(lyriccount < lrc.indexsize-1)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+1]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc4,wbuf);                    
                        }
                        else
                           SetWindowText(wnd_lrc4,L" ");
                        //��ʾ��i+2�еĸ�ʣ�����У�   
                        if(lyriccount < lrc.indexsize-2)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+2]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc5,wbuf);                    
                        }
                        else
                           SetWindowText(wnd_lrc5,L" ");
                                  
                     do{
                        lyriccount++;					
                        if(lyriccount>=lrc.indexsize)
                        {
                           lrc.oldtime=0xffffff;
                           break;
                        }
                        lrc.oldtime=lrc.time_tbl[lyriccount];
                        }while(lrc.oldtime<=(lrc.curtime*100));
                     }                  
               
                  }
                  //�Ҳ�������ļ�
                  else
                  {
                     
                     SetWindowText(wnd_lrc3,L"����SDCard������Ӧ�ĸ���ļ�(*.lrc)");
                     SetWindowText(wnd_lrc1,L" ");
                     SetWindowText(wnd_lrc2,L" ");
                     SetWindowText(wnd_lrc4,L" ");
                     SetWindowText(wnd_lrc5,L" ");
                  }
                  
               }
               
               timecount=0;
            }
         }
         else
         {
           uint8_t temp=0;	
               
           //���ݽ�������������λ��				
           temp=SendMessage(music_wnd_time, SBM_GETVALUE, NULL, NULL);        
               
           //�����������ʾ��ʱ��
           time_sum = (float)alltime/255*temp*1000;  	
           //����ʱ������ļ�λ�ò���ת����λ��
           pos = ID3V2_size + (time_sum/26)*frame_size;
           result = f_lseek(&file,pos);
           lrc.oldtime=0;
           lyriccount=0;
           chgsch=0;           
         }
		}
		Isread=0;
      
      
      timecount++;
	}
   lyriccount=0;
	I2S_Stop();   
	MP3FreeDecoder(Mp3Decoder);
	f_close(&file);	
  if(time2exit == 1)
  {
    lyriccount=0;
    I2S_Stop();   
    MP3FreeDecoder(Mp3Decoder);
    f_close(&file);	 
    GUI_SemPost(exit_sem);
  }
}

/* DMA��������жϻص����� */
/* �����������Ѿ�������ɣ���Ҫ�л��������������»��������ݲ��� 
   ͬʱ��ȡWAV�ļ�������䵽�Ѳ�������  */
void MP3Player_I2S_DMA_TX_Callback(void)
{
	if(I2Sx_TX_DMA_STREAM->CR&(1<<19)) //��ǰʹ��Memory1����
	{
		bufflag=0;                       //���Խ����ݶ�ȡ��������0
	}
	else                               //��ǰʹ��Memory0����
	{
		bufflag=1;                       //���Խ����ݶ�ȡ��������1
	}
	Isread=1;                          // DMA������ɱ�־
}





/**
  * @brief  ����WM8978��STM32��I2S��ʼ������
  * @param  ��
  * @retval ��
  */

/**
  * @brief   WAV��ʽ��Ƶ����������
	* @note   
  * @param  ��
  * @retval ��
  */
void wavplayer(const char *wavfile, uint8_t vol, HDC hdc, HWND hwnd)
{
	static uint8_t timecount;//��¼ʱ��
  WCHAR wbuf[128];
  char ooo = 0;
	mp3player.ucStatus=STA_IDLE;    /* ��ʼ����Ϊ����״̬  */
	Recorder.ucFmtIdx=3;           /* ȱʡ������I2S��׼��16bit���ݳ��ȣ�44K������  */
	Recorder.ucVolume=vol;          /* ȱʡ��������  */
   
  DWORD pos;//��¼���ֱ���
  static uint8_t lyriccount=0;//���index��¼   
   
	/*  ��ʼ��������I2S  */
	I2S_Stop();
	I2S_GPIO_Config();
	I2Sx_Mode_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
	I2Sxext_Mode_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
	
	I2S_DMA_TX_Callback=MP3Player_I2S_DMA_TX_Callback;
	I2S_Play_Stop();
	
	bufflag=0;
	Isread=0;
   if(mp3player.ucStatus == STA_IDLE)
   {						
      printf("��ǰ�����ļ� -> %s\n",wavfile);
	
      result=f_open(&file,wavfile,FA_READ);
      if(result!=FR_OK)
      {
         printf("����Ƶ�ļ�ʧ��!!!->%d\r\n",result);
         result = f_close (&file);
         Recorder.ucStatus = STA_ERR;
         return;
      }
      //��ȡWAV�ļ�ͷ
      result = f_read(&file,&rec_wav,sizeof(rec_wav),&bw);
      
      //������������б�Ͳ���ʾʱ��
      if(enter_flag == 0){
         //��ȡ��Ļ��385��404������ɫ             
        
         mp3player.ucFreq =  rec_wav.dwSamplesPerSec;
         mp3player.ucbps =  mp3player.ucFreq*32;   
         alltime=file.fsize*8/mp3player.ucbps;
        
//         x_wsprintf(wbuf, L"00:00 / %02d:%02d",alltime/60,alltime%60);
//         //���rc_MusicTimes���ε�����
///         ClrDisplay(hdc, &rc_MusicTimes, color);
//         //�����ı�
//         DrawText(hdc, wbuf, -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
      }   
      //�ȶ�ȡ��Ƶ���ݵ�������
      result = f_read(&file,(uint16_t *)buffer0,RECBUFFER_SIZE*2,&bw);
      result = f_read(&file,(uint16_t *)buffer1,RECBUFFER_SIZE*2,&bw);
      
      Delay_ms(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
      I2S_Stop();			/* ֹͣI2S¼���ͷ��� */
      wm8978_Reset();		/* ��λWM8978����λ״̬ */	

      mp3player.ucStatus = STA_PLAYING;		/* ����״̬ */

      /* ����WM8978оƬ������ΪDAC�����Ϊ���� */
      
      WCHAR wbuf[3];
      HWND  wnd = GetDlgItem(hwnd, ID_RECORD_BUGLE);
      
      GetWindowText(wnd, wbuf, 3);
      if (wbuf[0] == L'P')    // �жϵ�ǰ
      {
        wm8978_CfgAudioPath(DAC_ON, SPK_ON);                        // ����Ϊ���������
      }
      else
      {
        wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);    // ����Ϊ�������
      }
     
      /* ����������������ͬ���� */
      wm8978_SetOUT1Volume(Recorder.ucVolume);
      /* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
      wm8978_CfgAudioIF(I2S_Standard_Phillips, 16);
      
      I2Sx_Mode_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
      I2Sxext_Mode_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
      
      I2Sxext_RX_DMA_Init(&recplaybuf[0],&recplaybuf[1],1);
      DMA_ITConfig(I2Sxext_RX_DMA_STREAM,DMA_IT_TC,DISABLE);//������������ж�
      I2Sxext_Recorde_Stop();
      
      I2Sx_TX_DMA_Init(buffer0,buffer1,RECBUFFER_SIZE);		
      I2S_Play_Start();
   }
   /* ����������ѭ���� */
   while(mp3player.ucStatus == STA_PLAYING){
   /* DMA������� */
      if(Isread==1)
      {
         Isread=0;
         //�޸Ľ�����
         if(chgsch==0) 
         {  
           if(timecount>=10)      
           { 
               curtime=file.fptr*8/mp3player.ucbps;                                        //��ȡ��ǰ���Ž���(��λ��s)
               if(enter_flag == 0){
                  //�������ʱ����ʾ�͸�����ֵ���ʾ
//                  ClrDisplay(hdc, &rc_MusicTimes, color);
//                  ClrDisplay(hdc, &rc_musicname, color);
                  //���ַ�����ת��Ϊ���ַ�����
//                   x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
//                  DrawText(hdc, wbuf, -1, &rc_musicname, DT_SINGLELINE | DT_CENTER | DT_VCENTER);//��������
                  //������ʱ���ʽ�������wbuf
//                  x_wsprintf(wbuf, L"%02d:%02d",curtime/60,curtime%60,alltime/60,alltime%60);
//                  DrawText(hdc, wbuf, -1, &rc_MusicTimes, DT_SINGLELINE | DT_CENTER | DT_VCENTER);//��������
                  //���½�����
                  
                  if(ooo == 0)//ȷ��ֻ��ˢ��һ��
                  {
                     x_wsprintf(wbuf, L"%02d:%02d",alltime/60,alltime%60);
                     SetWindowText(GetDlgItem(hwnd, ID_TB1), wbuf);                
//                     x_mbstowcs_cp936(wbuf, music_lcdlist[play_index], FILE_NAME_LEN);
//                     SetWindowText(GetDlgItem(hwnd, ID_TB5), wbuf);       
                     ooo=1;
                  }
                  x_wsprintf(wbuf, L"%02d:%02d",curtime/60,curtime%60);
                  SetWindowText(GetDlgItem(hwnd, ID_TB2), wbuf);  
                  
                  SendMessage(music_wnd_time, SBM_SETVALUE, TRUE, curtime*255/alltime);
                  InvalidateRect(music_wnd_time, NULL, TRUE);   
                  //InvalidateRect(GetDlgItem(hwnd, ID_TB2), NULL, TRUE); 

                  lrc.curtime = curtime;  
                  if(lrc.flag == 1){
                     //+100����ǰ��ʾ����ʾ��Ҫ����һ��ʱ��
                     if((lrc.oldtime <= lrc.curtime*100+100)&&(lrc.indexsize>7))
                     {
                        //��ʾ��ǰ�еĸ��
                        x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount]-1], LYRIC_MAX_SIZE);
                        SetWindowText(wnd_lrc3,wbuf);
                        //��ʾ��i-1�еĸ�ʣ�ǰһ�У�
                        if(lyriccount>0)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-1]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc2,wbuf);
                        }
                        else
                           SetWindowText(wnd_lrc2,L" ");
                        //��ʾ��i-2�еĸ�ʣ�ǰ���У�
                        if(lyriccount>0)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-2]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc1,wbuf);
                        }
                        else
                           SetWindowText(wnd_lrc1,L" ");
                        //��ʾ��i+1�еĸ�ʣ���һ�У�   
                        if(lyriccount < lrc.indexsize-1)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+1]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc4,wbuf);                    
                        }
                        else
                           SetWindowText(wnd_lrc4,L" ");
                        //��ʾ��i+2�еĸ�ʣ�����У�   
                        if(lyriccount < lrc.indexsize-2)
                        {
                           x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+2]-1], LYRIC_MAX_SIZE);
                           SetWindowText(wnd_lrc5,wbuf);                    
                        }
                        else
                           SetWindowText(wnd_lrc5,L" ");
                                  
                     do{
                        lyriccount++;					
                        if(lyriccount>=lrc.indexsize)
                        {
                           lrc.oldtime=0xffffff;
                           break;
                        }
                        lrc.oldtime=lrc.time_tbl[lyriccount];
                        }while(lrc.oldtime<=(lrc.curtime*100));
                     }                  
               
                  }
                  //�Ҳ�������ļ�
                  else
                  {
                     
                     SetWindowText(wnd_lrc3,L"����SDCard������Ӧ�ĸ���ļ�(*.lrc)");
                     SetWindowText(wnd_lrc1,L" ");
                     SetWindowText(wnd_lrc2,L" ");
                     SetWindowText(wnd_lrc4,L" ");
                     SetWindowText(wnd_lrc5,L" ");
                  }                  
               }   
               
               timecount=0;  
           }                              
         } 
         else
         {
           uint8_t temp=0;
          
           temp=SendMessage(music_wnd_time, SBM_GETVALUE, NULL, NULL);  
           pos=file.fsize/255*temp;
           if(pos<sizeof(WavHead))pos=sizeof(WavHead);
           if(rec_wav.wBitsPerSample==24)temp=12;
           else temp=8;
           if((pos-sizeof(WavHead))%temp)
           {
             pos+=temp-(pos-sizeof(WavHead))%temp;
           }        
           f_lseek(&file,pos);
           lrc.oldtime=0;
           lyriccount=0;
           chgsch=0;         
         }
         timecount++;
         if(bufflag==0)
            result = f_read(&file,buffer0,RECBUFFER_SIZE*2,&bw);	
         else
            result = f_read(&file,buffer1,RECBUFFER_SIZE*2,&bw);
         /* ������ɻ��ȡ����ֹͣ���� */
         if((result!=FR_OK)||(file.fptr==file.fsize))
         {
            //�����и�״̬
            mp3player.ucStatus=STA_SWITCH;
            //������Ŀ����1
            play_index++;
            //printf("%d, %d\n", play_index, music_file_num);
            //����Ϊ�б�ѭ������
            if(play_index >= music_file_num) play_index = 0;
            if(play_index < 0) play_index = music_file_num - 1;
            printf("��������߶�ȡ�����˳�...\r\n");
            I2S_Play_Stop();
            file.fptr=0;
            f_close(&file);
            I2S_Stop();		/* ֹͣI2S¼���ͷ��� */
            wm8978_Reset();	/* ��λWM8978����λ״̬ */							
         }		    
      }
   }
			
      mp3player.ucStatus = STA_SWITCH;		/* ����״̬ */
      file.fptr=0;
      f_close(&file);
      lrc.oldtime=0;
      lyriccount=0;      
      I2S_Stop();		/* ֹͣI2S¼���ͷ��� */
      wm8978_Reset();	/* ��λWM8978����λ״̬ */
		
	
}
/***************************** (END OF FILE) *********************************/
