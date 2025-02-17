/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   RT-Thread 3.0 + STM32F429 + emxgui demo
  *********************************************************************
  * @attention
  *
  * 实验平台:野火 F429-挑战者 STM32 开发板
  * 官网    :www.embedfire.com
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
#include "board.h"
#include "rtthread.h"


/*
*************************************************************************
*                               变量
*************************************************************************
*/
/* 定义线程控制块 */
static rt_thread_t gui_thread = RT_NULL;

//static rt_thread_t get_cpu_use_thread = RT_NULL;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void gui_thread_entry(void* parameter);


/*
*************************************************************************
*                             main 函数
*************************************************************************
*/
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
    /* 
	 * 开发板硬件初始化，RTT系统初始化已经在main函数之前完成，
	 * 即在component.c文件中的rtthread_startup()函数中完成了。
	 * 所以在main函数中，只需要创建线程和启动线程即可。
	 */
	
	gui_thread =                          /* 线程控制块指针 */
    rt_thread_create( "gui",              /* 线程名字 */
                      gui_thread_entry,   /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      2048,                 /* 线程栈大小 */
                      3,                   /* 线程的优先级，数字优先级越大，逻辑优先级越小 */
                      1);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (gui_thread != RT_NULL)
        rt_thread_startup(gui_thread);
    else
        return -1;

    
//   get_cpu_use_thread =                          /* 线程控制块指针 */
//    rt_thread_create( "get_cpu_use",              /* 线程名字 */
//                      get_cpu_use_thread_entry,   /* 线程入口函数 */
//                      RT_NULL,             /* 线程入口函数参数 */
//                      512,                 /* 线程栈大小 */
//                      5,                   /* 线程的优先级 */
//                      20);                 /* 线程时间片 */
//                   
//    /* 启动线程，开启调度 */
//   if (get_cpu_use_thread != RT_NULL)
//        rt_thread_startup(get_cpu_use_thread);
//    else
//        return -1; 
}

/*
*************************************************************************
*                             线程定义
*************************************************************************
*/
extern void GUI_Startup(void);

static void gui_thread_entry(void* parameter)
{	 

  /* 执行本函数不会返回 */
	GUI_Startup();
	
  while (1)
  {
    LED1_ON;
    rt_thread_delay(500);   /* 延时500个tick */
    rt_kprintf("gui_thread running,LED1_ON\r\n");
    
    LED1_OFF;     
    rt_thread_delay(500);   /* 延时500个tick */		 		
    rt_kprintf("gui_thread running,LED1_OFF\r\n");
  }
}



/********************************END OF FILE****************************/
