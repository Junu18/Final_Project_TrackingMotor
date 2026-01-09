/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Polling Scenario for SPI 32-bit Debugging
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "usart.h" // UART 핸들
#include "../ap/Common/Common.h"
#include "../ap/Listener/Listener_SPITest.h"
#include "../ap/Model/Model_SPITest.h"
#include "../ap/Presenter/Presenter_SPITest.h"
#include "../ap/Controller/Controller_SPITest.h"
#include "../driver/SPI/SPI.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
extern Model_SPITest_t g_spi_test_model;
extern UART_HandleTypeDef huart2;

extern osThreadId defaultTaskHandle;
extern osThreadId Listener_TaskHandle;
extern osThreadId Controller_TaskHandle;
extern osThreadId Presenter_TaskHandle;
/* USER CODE END Variables */

/* GetIdleTaskMemory 함수들 (생략 가능하나 유지) */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void MX_FREERTOS_Init(void) {}

/* 다른 태스크들은 현재 사용 안 함 */
void StartDefaultTask(void const * argument) { for(;;) { osDelay(1); } }
void Listener(void const * argument) { for(;;) { osDelay(1000); } }
void Controller(void const * argument) { for(;;) { osDelay(1000); } }

/**
* @brief Presenter_Task: 폴링 방식으로 32비트 데이터 검증
*/
void Presenter(void const * argument)
{
  /* 초기화 */
  Model_SPITest_Init(&g_spi_test_model);
  Listener_SPITest_Init();

  osDelay(500);
  printf("finish___");
  printf("\r\n--- SPI Polling Mode (8-bit Fix) ---\r\n");

  for(;;)
  {
    /* 1. SPI 데이터 요청 (여기서 CS Low -> 32clk -> CS High 발생) */
    Listener_SPITest_RequestData();

    /* 2. 모델 업데이트 */
    Presenter_SPITest_SyncData(&g_spi_test_model);

    /* 3. PC 터미널로 출력 */
    // Raw 값이 0x00000000이 아닌 값이 나와야 성공!
    printf("[SPI] Raw: 0x%08lX | X: %lu | Y: %lu\r\n",
           g_spi_test_model.rx_packet.raw,
           g_spi_test_model.rx_packet.fields.x_pos,
           g_spi_test_model.rx_packet.fields.y_pos);

    osDelay(200); // 0.2초마다 반복
  }
}
