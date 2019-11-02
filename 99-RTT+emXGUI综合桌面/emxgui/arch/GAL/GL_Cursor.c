/**
  *********************************************************************
  * @file    GL_Cursor.c
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   �����ƽӿ�
  * @note    ���ڴ˶����Լ���������
  *********************************************************************
  * @attention
  * ����    :www.emXGUI.com
  *
  **********************************************************************
  */ 

#include	"gui_drv.h"

#define	CURSOR_W	20
#define	CURSOR_H	20

/*============================================================================*/

static	RECT rc_cursor;
static	RECT rc_cursor_clip;

static	int oldx =-1;
static	int oldy =-1;

static	int curx =-1;
static	int cury =-1;

static	int cur_show=FALSE;

static	u32	CursorShowNest=0;

#pragma pack(4)
static u16 cursor_buf[CURSOR_H][CURSOR_W]; //ARGB4444
#pragma	pack()

static u8  *cursor_bk_buf =NULL;

static const SURFACE *pSurfScreen=NULL;

/*============================================================================*/
/**
  * @brief  ���ع��
  */
static	void	_gdevHideCursor(int x,int y,int w,int h)
{

	if(pSurfScreen->Format == SURF_RGB565)
	{
		pSurfScreen->GL->DrawBitmap_RGB565(pSurfScreen,x,y,w,h,2*CURSOR_W,cursor_bk_buf);
	}

	if(pSurfScreen->Format == SURF_XRGB8888)
	{
		pSurfScreen->GL->DrawBitmap_XRGB8888(pSurfScreen,x,y,w,h,4*CURSOR_W,cursor_bk_buf);
	}

}

/**
  * @brief  ��ʾ���
  */
static	void	_gdevShowCursor(int x,int y,int w,int h)
{
	if(pSurfScreen->Format == SURF_RGB565)
	{
		pSurfScreen->GL->CopyBits(pSurfScreen,x,y,w,h,2*CURSOR_W,cursor_bk_buf);
	}

	if(pSurfScreen->Format == SURF_XRGB8888)
	{
		pSurfScreen->GL->CopyBits(pSurfScreen,x,y,w,h,4*CURSOR_W,cursor_bk_buf);
	}

	pSurfScreen->GL->DrawBitmap_ARGB4444(pSurfScreen,x,y,w,h,CURSOR_W*2,(u8*)cursor_buf);
}

/**
  * @brief  ���ع��
  * @param  ��
  * @retval ��
  */
static	void _CursorHide(void)
{
	RECT in_rc,rc;

	rc.x = oldx;
	rc.y = oldy;
	rc.w = CURSOR_W;
	rc.h = CURSOR_H;
	
 	if(rc.x<0)
	{		
		rc.w =MAX(0,rc.w+rc.x);
		rc.x =0;	
	}
		
	if(rc.y<0)
	{	
		rc.h =MAX(0,rc.h+rc.y);
		rc.y =0;	
	}
	
	IntersectRect(&in_rc,&rc_cursor_clip,&rc);
	_gdevHideCursor(in_rc.x,in_rc.y,in_rc.w,in_rc.h);
		
}

/**
  * @brief  ��ʾ���
  * @param  ��
  * @retval ��
  */
static	void _CursorShow(void)
{

    RECT in_rc,rc; 

    rc.x = curx;
    rc.y = cury;
    rc.w = CURSOR_W;
    rc.h = CURSOR_H;
	 
 	if(rc.x<0)
	{		
		rc.w =MAX(0,rc.w+rc.x);
		rc.x =0;	
	}
		
	if(rc.y<0)
	{	
		rc.h =MAX(0,rc.h+rc.y);
		rc.y =0;	
	}
  
	IntersectRect(&in_rc,&rc_cursor_clip,&rc);
	_gdevShowCursor(in_rc.x,in_rc.y,in_rc.w,in_rc.h);
	

}

/*============================================================================*/
/*============================================================================*/

#if 0
static	const u8 cursor_bitmap_16x16[16][16]=
{

"@               ",
"@@              ",
"@*@             ",
"@**@            ",
"@*-*@           ",
"@*--*@          ",
"@*---*@         ",
"@*----*@        ",
"@*-----*@       ",
"@*---**-*@      ",
"@*--*@@***@     ",
"@*-*@  @@**@    ",
"@**@     @@*@   ",
"@*@        @@@  ",
"@@           @@ ",
"@               ",

};
#endif

/**
  * @brief  ���������ݣ�cursor_bitmap_init��ת���ɶ�Ӧ����������
  */
static	const u8 cursor_bitmap_16x20[20][16]=
{

"@+             ",
"@@+            ",
"@*@+           ",
"@**@+          ",
"@*-*@+         ",
"@*--*@+        ",
"@*---*@+       ",
"@*----*@+      ",
"@*-----*@+     ",
"@*------*@+    ",
"@*-------*@+   ",
"@*-**--****@+  ",
"@**@@*-*@@@@@+ ",
"@*@+@*-*@+++++ ",
"@@+ +@*-*@+    ",
"@+   @*-*@+    ",
"      @*-*@+   ",
"      @***@+   ",
"       @@@@+   ",
"        +++    ",

};

/**
  * @brief  ��ʼ������ͼ������
  * @param  pSurf Ҫ��ʾ�Ļ�ͼ����
  * @retval ��
  */
static	void	cursor_bitmap_init(void)
{
	int x,y;

	for(y=0;y<20;y++)
	{
		for(x=0;x<16;x++)
		{
			switch(cursor_bitmap_16x20[y][x])
			{
				case	'@':
						cursor_buf[y][x] =ARGB4444(15,0,0,0);
						break;
				case	'*':
						cursor_buf[y][x] =ARGB4444(15,15,15,15);
						break;
				case	'+':
						cursor_buf[y][x] =ARGB4444(6,6,6,6);
						break;
				case	'-':
						cursor_buf[y][x] =ARGB4444(8,15,15,15);
						break;
				default:
						cursor_buf[y][x] =ARGB4444(0,0,0,0);
			}
		}
	}
}

/**
  * @brief  ��ʼ�����
  * @param  pSurf Ҫ��ʾ�Ļ�ͼ����
  * @param  x y ��ʼ��ʾλ��
  * @retval �Ƿ�����
  */
BOOL	GL_CursorInit(const SURFACE *pSurf,int x,int y)
{
	int i;

	pSurfScreen =pSurf;

	cur_show = FALSE;
	CursorShowNest = 0;
	rc_cursor.x = x;
    rc_cursor.y = y;
	rc_cursor.w = CURSOR_W;
	rc_cursor.h = CURSOR_H;

	curx = rc_cursor.x;
	cury = rc_cursor.y;
	oldx = curx;
	oldy = cury;

	rc_cursor_clip.x = 0;
	rc_cursor_clip.y = 0;
	rc_cursor_clip.w = pSurfScreen->Width;
	rc_cursor_clip.h = pSurfScreen->Height;

	cursor_bitmap_init();

	i=0;
	if(pSurfScreen->Format == SURF_RGB565)
	{
		i =CURSOR_W*CURSOR_H*2;
	}
	if(pSurfScreen->Format == SURF_XRGB8888)
	{
		i =CURSOR_W*CURSOR_H*4;
	}

	cursor_bk_buf =GUI_GRAM_Alloc(i);

	if(cursor_bk_buf != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

/*============================================================================*/
/**
  * @brief  ��ʾ���
  * @param  bShow �Ƿ���ʾ
  * @retval ��ʾ�㼶ֵ
  */
int 	GL_ShowCursor(BOOL bShow)
{
#if(GUI_INPUT_DEV_EN)   
	if(bShow != FALSE)
	{
		if(CursorShowNest < 0xFFFF)
		{
			CursorShowNest++;
		}
		_CursorShow(); 
	}
	else
	{
		if(CursorShowNest > 0)
		{
			CursorShowNest--;
			if(CursorShowNest == 0)
			{
				_CursorHide();	
			}
		}
	
	}
	
	return CursorShowNest;
#else
  return 1;
#endif
}

/**
  * @brief  ��ȡ�������
  * @param  x[out] y[out] ��ȡ��������
  * @retval �Ƿ��ȡ����
  */
BOOL	GL_GetCursorPos(int *x,int *y)
{
	*x = curx;
	*y = cury;
	return	TRUE;
}

/**
  * @brief  ���ù������
  * @param  x y Ҫ���õ�������
  * @retval �Ƿ�����
  */
BOOL	GL_SetCursorPos(int x,int y)
{
	x = MAX(x,0);
	y = MAX(y,0);
	
    x = MIN(x,pSurfScreen->Width-1);
    y = MIN(y,pSurfScreen->Height-1);
   
    rc_cursor.x	=x;
    rc_cursor.y	=y;
    
    curx = x;
    cury = y;
    
   	if ((oldx != curx) || (oldy != cury))
   	{   		
        if(CursorShowNest > 0)
        {	
            _CursorHide();
            _CursorShow();                     
        }
		
		oldx = curx;
		oldy = cury;
    }
    
	return	TRUE;
}


BOOL	GL_SetCursorClipRect(const RECT *lprc)
{
	RECT in_rc,rc;
	
	int	res =FALSE;
	
	if(lprc == NULL)
	{
		return res;
	}
		
	rc.x = 0;
	rc.y = 0;
	rc.w = pSurfScreen->Width;
	rc.h = pSurfScreen->Height;
	if(IntersectRect(&in_rc,&rc,lprc))
	{
		
		CopyRect(&rc_cursor_clip,&in_rc);
			
		if(CursorShowNest > 0)
		{
			_CursorHide();
			_CursorShow();
				
			oldx = curx;
			oldy = cury;
		}
	
		res = TRUE;
	}
	return res;
}

void	GL_CursorCheck(const RECT *lprc) //
{
	if(CursorShowNest > 0)
	{
		if(cur_show == FALSE)
		{	
			if(lprc!=NULL)
			{
				if(IsIntersectRect(&rc_cursor,lprc))
				{
					_CursorHide();
					cur_show =TRUE;
				}
			}
		}
	}
}

void GL_CursorFix(void)
{
	if(CursorShowNest > 0)
	{
		if(cur_show == TRUE)
		{
			_CursorShow();

			oldx = curx;
			oldy = cury;

			cur_show =FALSE;
		}
	}
}

/*============================================================================*/
/*============================================================================*/

