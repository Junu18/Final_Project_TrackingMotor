/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Cleaned for Task Conflicts)
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "../ap/Common/Common.h"
#include "../ap/Listener/ListenerTracking.h"
#include "../ap/Model/ModelTracking.h"
#include "../ap/Controller/ControllerTracking.h"
#include "../ap/Presenter/PresenterTracking.h"
#include "../driver/SPI/SPI.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* 핸들 참조 */
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;

/* RTOS 핸들 정의 */
osThreadId defaultTaskHandle;
osThreadId Listener_TaskHandle;
osThreadId Controller_TaskHandle;
osThreadId Presenter_TaskHandle;

/* USER CODE BEGIN PV */
/* Reference Architecture: Queue는 Model에서 관리 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* 태스크 함수 선언 (프로토타입만 남깁니다) */
void StartDefaultTask(void const * argument);
void Listener(void const * argument);
void Controller(void const * argument);
void Presenter(void const * argument);

/* USER CODE BEGIN 0 */
/**
  * @brief printf 리다이렉션: usart.c 수정 없이 UART2로 출력
  */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 10);
    return len;
}

/* ========================================
 * SPI 수신 완료 콜백 (인터럽트 모드)
 * ======================================== */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // 1. CS 핀 올리기 (통신 종료)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);

        // 2. 수신된 4바이트 데이터를 32비트로 조립
        extern uint8_t rx_buff[4];  // ListenerTracking.c 변수 참조
        extern RxPacket_t g_rx_packet_tracking;

        g_rx_packet_tracking.raw = (rx_buff[0] << 24) |
                                   (rx_buff[1] << 16) |
                                   (rx_buff[2] << 8)  |
                                    rx_buff[3];

        // 3. Event 발행 (Reference Architecture)
        extern osMessageQId trackingEventMsgBox;
        extern void Listener_Tracking_StartReceive(void);

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        osMessagePut(trackingEventMsgBox, EVENT_FPGA_DATA_RECEIVED, 0);

        // 4. 연속 수신: 다음 SPI 전송 즉시 시작 (FPGA 계속 받기!)
        Listener_Tracking_StartReceive();

        // 5. 컨텍스트 스위칭 필요 시 수행
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* 하드웨어 초기화 */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  // UART 테스트 메시지 (시스템 시작 확인)
  printf("\r\n");
  printf("========================================\r\n");
  printf("  STM32 System Initialized\r\n");
  printf("  UART2 Working - Baud: 115200\r\n");
  printf("========================================\r\n");
  printf("\r\n");

  SPI_Init();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* Reference Architecture: Model에서 Queue/Pool 초기화 */
  extern void Model_Tracking_QueueInit(void);
  Model_Tracking_QueueInit();
  /* USER CODE END RTOS_QUEUES */

  /* 태스크 생성 */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  osThreadDef(Listener_Task, Listener, osPriorityNormal, 0, 128);
  Listener_TaskHandle = osThreadCreate(osThread(Listener_Task), NULL);

  osThreadDef(Controller_Task, Controller, osPriorityNormal, 0, 128);
  Controller_TaskHandle = osThreadCreate(osThread(Controller_Task), NULL);

  osThreadDef(Presenter_Task, Presenter, osPriorityNormal, 0, 512);
  Presenter_TaskHandle = osThreadCreate(osThread(Presenter_Task), NULL);

  MX_FREERTOS_Init();
  osKernelStart();

  while (1) {}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
}

void Error_Handler(void) { __disable_irq(); while (1) {} }

/* USER CODE BEGIN 4 */
/* [수정 완료] 여기에 있던 빈 함수 본문들을 모두 삭제했습니다.
   이제 freertos.c에 있는 실제 함수들과 충돌하지 않습니다. */
/* USER CODE END 4 */
