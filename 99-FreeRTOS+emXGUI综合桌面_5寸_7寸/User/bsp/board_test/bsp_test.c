/**
  ******************************************************************************
  * @file    bsp_test.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   �弶����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32 F429 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./board_test/bsp_test.h"
#include "board.h"
#include "./i2c/i2c.h"
#include "./mpu6050/bsp_mpu_exti.h"
#include "inv_mpu.h"
#include "sdio/bsp_sdio_sd.h"
#include "netconf.h"

#define INIT    1    // ���Բ��ڳ�ʼʱ����

typedef struct
{
  uint16_t wm8978_test_state  : 1;
  uint16_t flash_test_state   : 1;
  uint16_t sdram_test_state   : 1;
  uint16_t mpu6050_test_state : 1;
  uint16_t eeprom_test_state  : 1;
  uint16_t sd_test_state      : 1;
  uint16_t network_test       : 1;
  uint16_t Retain             : 9;
}bsp_test_t;

#define  DATA_SIZE    50
const uint8_t test_data_write[DATA_SIZE] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10,1,2,3,4,5,6,7,8,9,10};    // д���ݻ�����
uint8_t test_data_read[DATA_SIZE];    // �����ݻ�����

uint8_t wm8978_test(void)
{
  if (wm8978_Reset()==0)
  {
    return TEST_FAIL;
  }
  else
  {
    return TEST_SUCCESS;
  }
}

uint8_t flash_test(void)
{
  if (SPI_FLASH_Init() == 0)
  {
    return TEST_SUCCESS;
  }
  else
  {
    return TEST_FAIL;
  }
}

uint8_t sdram_test(void)
{
  
#if INIT
  //SDRAM_Init();
#endif
  
  /* д������ */
  for(uint16_t xC=0; xC<65535; xC++)
  {
    *(__IO uint16_t*) (SDRAM_BANK_ADDR + xC*2) = xC;
  }
  
  /* �����ݲ�У�� */
  for(uint16_t xC=0; xC<65535; xC++)
  {
    if (*(__IO uint16_t*) (SDRAM_BANK_ADDR + xC*2) != xC)
    {
      return TEST_FAIL;
    }
  }
  
  return TEST_SUCCESS;

}

uint8_t mpu6050_test(void)    // �õ�����ϵͳ����ʱ�ŵ�����
{
  struct int_param_s int_param;
  
#if INIT
  // EXTI_MPU_Config();
  I2cMaster_Init();
#endif
  
  if (mpu_init(&int_param) == 0)
  {
    return TEST_SUCCESS;
  }
  else
  {
    return TEST_FAIL;
  }
}

uint8_t eeprom_test(void)
{
  if (ee_CheckOk() == 1)    // �� ee_CheckOk() ���ʼ��
  {
    return TEST_SUCCESS;
  }
  else
  {
    return TEST_FAIL;
  }
}

uint8_t sd_test(void)
{
  if (SD_Init() == SD_OK)
  {
    SD_DeInit();
    return TEST_SUCCESS;
  }
  else
  {
    SD_DeInit();
    return TEST_FAIL;
  }
}

extern struct netif gnetif;
extern uint8_t network_start_flag;

uint8_t network_test(void)    // ��ʼ��ʱ��ϳ��ŵ�����
{
  /* Configure ethernet (GPIOs, clocks, MAC, DMA) */
  if(ETH_BSP_Config() == 0)
  {
    network_start_flag = 1;    // ��ǳ�ʼ�ɹ�������̫��APP�оͲ��ڳ�ʼ��
    return TEST_SUCCESS;
  }
  else
  {
    network_start_flag = 0;
    return TEST_FAIL;
  }
}

uint8_t Comprehensive_Test(void)
{
  bsp_test_t BSP_Test_State;
  if (wm8978_test() == TEST_SUCCESS)
  {
    BSP_Test_State.wm8978_test_state = 1;
    printf("WM8978���ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.wm8978_test_state = 0;
    printf("WM8978���ʧ�ܣ�\r\n");
  }
  
  if (flash_test() == TEST_SUCCESS)
  {
    BSP_Test_State.flash_test_state = 1;
    printf("SPI FLASH ���ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.flash_test_state = 0;
    printf("SPI FLASH ���ʧ�ܣ�\r\n");
  }

  if (sdram_test() == TEST_SUCCESS)
  {
    BSP_Test_State.sdram_test_state = 1;
    printf("SDRAM ���ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.sdram_test_state = 0;
    printf("SDRAM ���ʧ�ܣ�\r\n");
  }
  
//  if (mpu6050_test() == TEST_SUCCESS)
//  {
//    BSP_Test_State.mpu6050_test_state = 1;
//    printf("MPU6050 ���ɹ���");
//  }
//  else
//  {
//    BSP_Test_State.mpu6050_test_state = 0;
//    printf("MPU6050 ���ʧ�ܣ�");
//  }
  
  if (eeprom_test() == TEST_SUCCESS)
  {
    BSP_Test_State.eeprom_test_state  = 1;
    printf("EEPROM ���ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.eeprom_test_state  = 0;
    printf("EEPROM ���ʧ�ܣ�\r\n");
  }
  
  if (sd_test() == TEST_SUCCESS)
  {
    BSP_Test_State.sd_test_state  = 1;
    printf("SD �����ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.sd_test_state  = 0;
    printf("SD �����ʧ�ܣ�\r\n");
  }
  
  if (network_test() == TEST_SUCCESS)
  {
    BSP_Test_State.network_test  = 1;
    printf("��̫�����ɹ���\r\n");
  }
  else
  {
    BSP_Test_State.network_test  = 0;
    printf("��̫�����ʧ�ܣ�\r\n");
  }
  
  return TEST_FAIL;
}
/*********************************************END OF FILE**********************/
