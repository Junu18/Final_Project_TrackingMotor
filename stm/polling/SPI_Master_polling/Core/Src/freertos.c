/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Tracking System with SPI Interrupt Mode
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
#include "usart.h"
#include "../ap/Common/Common.h"
#include "../ap/Listener/Listener.h"
#include "../ap/Controller/Controller.h"
#include "../ap/Presenter/Presenter.h"
#include "../driver/SPI/SPI.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Variables */
/* Reference Architecture: Task 간 직접 참조 제거, Event/Data Queue 사용 */
extern UART_HandleTypeDef huart2;

extern osThreadId defaultTaskHandle;
extern osThreadId Listener_TaskHandle;
extern osThreadId Controller_TaskHandle;
extern osThreadId Presenter_TaskHandle;
/* USER CODE END Variables */

/* GetIdleTaskMemory */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void MX_FREERTOS_Init(void) {}

/* ========================================
 * Default Task (Idle)
 * ======================================== */
void StartDefaultTask(void const * argument) {
    for(;;) {
        osDelay(1000);
    }
}

/* ========================================
 * Listener Task (Reference Architecture)
 * ======================================== */
void Listener(void const * argument)
{
    Listener_Init();
    for(;;)
    {
        Listener_Excute();
        osDelay(1);  // Reference Architecture: 폴링 주기
    }
}

/* ========================================
 * Controller Task (Reference Architecture)
 * ======================================== */
void Controller(void const * argument)
{
    Controller_Init();
    for(;;)
    {
        Controller_Excute();
        osDelay(1);  // Reference Architecture: 폴링 주기
    }
}

/* ========================================
 * Presenter Task (Reference Architecture)
 * ======================================== */
void Presenter(void const * argument)
{
    Presenter_Init();
    for(;;)
    {
        Presenter_Excute();
        osDelay(1);  // Reference Architecture: 폴링 주기
    }
}
