/*
 * ListenerTracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "ListenerTracking.h"
#include "spi.h"
#include "main.h"
#include <stdio.h>

/* ========================================
 * 전역 변수 정의
 * ======================================== */
RxPacket_t g_rx_packet_tracking;  // SPI Callback에서 사용할 수신 버퍼

/* 송신/수신 버퍼 (인터럽트 모드용) */
uint8_t tx_buff[4] = {0xAA, 0x00, 0x00, 0x00};  // 송신 헤더 (Callback에서 접근)
uint8_t rx_buff[4] = {0};                       // 수신 버퍼 (Callback에서 접근)

/**
 * @brief Listener 초기화
 */
void Listener_Tracking_Init(void) {
    // CS 핀 초기 상태: High (통신 대기)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);

    printf("[Listener] Tracking Listener Initialized\r\n");
}

/**
 * @brief SPI 인터럽트 수신 시작
 * @note FPGA가 데이터를 보낼 준비가 되면 이 함수를 호출
 */
void Listener_Tracking_StartReceive(void) {
    printf("\r\n[DEBUG] === StartReceive called ===\r\n");
    printf("[DEBUG] SPI State BEFORE: %d (1=READY, 2=BUSY, 4=BUSY_TX, 8=BUSY_RX, 16=BUSY_TX_RX)\r\n", hspi1.State);
    printf("[DEBUG] SPI Lock BEFORE: %d (0=UNLOCKED, 1=LOCKED)\r\n", hspi1.Lock);
    printf("[DEBUG] SPI CR1: 0x%04X\r\n", hspi1.Instance->CR1);
    printf("[DEBUG] SPI CR2: 0x%04X\r\n", hspi1.Instance->CR2);
    printf("[DEBUG] SPI SR: 0x%04X\r\n", hspi1.Instance->SR);
    printf("[DEBUG] pTxBuffPtr=%p, pRxBuffPtr=%p, Size=%d\r\n",
           (void*)tx_buff, (void*)rx_buff, 4);

    // CS Low (통신 시작)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
    printf("[DEBUG] CS Low set\r\n");

    // [FIX] SPI 페리페럴 명시적 활성화 (SPE bit 설정)
    // CR1 레지스터의 bit 6 (SPE)가 0이면 SPI가 비활성화 상태
    if ((hspi1.Instance->CR1 & SPI_CR1_SPE) == 0) {
        printf("[DEBUG] SPI was disabled (SPE=0), manually enabling...\r\n");
        __HAL_SPI_ENABLE(&hspi1);
        printf("[DEBUG] SPI enabled, CR1 now: 0x%04X\r\n", hspi1.Instance->CR1);
    }

    // SPI 인터럽트 모드로 송수신 시작
    // 완료되면 HAL_SPI_TxRxCpltCallback()이 자동 호출됨
    printf("[DEBUG] Calling HAL_SPI_TransmitReceive_IT...\r\n");
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_IT(&hspi1, tx_buff, rx_buff, 4);
    printf("[DEBUG] HAL_SPI_TransmitReceive_IT returned\r\n");

    printf("[DEBUG] SPI Return Status: %d (0=OK, 1=ERROR, 2=BUSY, 3=TIMEOUT)\r\n", status);
    printf("[DEBUG] SPI State AFTER: %d\r\n", hspi1.State);
    printf("[DEBUG] SPI Lock AFTER: %d\r\n", hspi1.Lock);
    printf("[DEBUG] SPI CR2 AFTER: 0x%04X (bit 7=TXEIE, bit 6=RXNEIE, bit 5=ERRIE)\r\n", hspi1.Instance->CR2);
    printf("[DEBUG] SPI SR AFTER: 0x%04X\r\n", hspi1.Instance->SR);

    // [FIX] HAL 함수가 CR2를 설정하지 않는 버그 우회: 수동으로 인터럽트 활성화
    if (status == HAL_OK && hspi1.Instance->CR2 == 0x0000) {
        printf("[WORKAROUND] HAL function didn't enable interrupts, manually setting CR2...\r\n");
        printf("[WORKAROUND] hspi1.Instance address: %p, SPI1 base: %p\r\n",
               (void*)hspi1.Instance, (void*)SPI1);

        // State와 버퍼를 수동으로 설정
        printf("[WORKAROUND] Setting State to %d...\r\n", HAL_SPI_STATE_BUSY_TX_RX);
        hspi1.State = HAL_SPI_STATE_BUSY_TX_RX;
        printf("[WORKAROUND] State written, reading back: %d\r\n", hspi1.State);

        hspi1.pTxBuffPtr = tx_buff;
        hspi1.pRxBuffPtr = rx_buff;
        hspi1.TxXferCount = 4;
        hspi1.RxXferCount = 4;
        hspi1.TxXferSize = 4;
        hspi1.RxXferSize = 4;

        // [FIX] STM32 하드웨어 제약: SPE=1 상태에서는 CR2 수정 불가!
        // 해결: SPE를 먼저 비활성화 → CR2 설정 → SPE 재활성화
        printf("[WORKAROUND] CR2 BEFORE: 0x%04X, CR1: 0x%04X (SPE=%d)\r\n",
               hspi1.Instance->CR2, hspi1.Instance->CR1,
               (hspi1.Instance->CR1 & SPI_CR1_SPE) ? 1 : 0);

        // Step 1: SPI 비활성화 (SPE=0)
        printf("[WORKAROUND] Disabling SPI (SPE=0) to allow CR2 write...\r\n");
        __HAL_SPI_DISABLE(&hspi1);
        printf("[WORKAROUND] CR1 after disable: 0x%04X (SPE=%d)\r\n",
               hspi1.Instance->CR1, (hspi1.Instance->CR1 & SPI_CR1_SPE) ? 1 : 0);

        // Step 2: CR2 인터럽트 활성화 (SPE=0 상태에서)
        printf("[WORKAROUND] Setting CR2 interrupt bits (TXE|RXNE|ERR = 0x00E0)...\r\n");
        hspi1.Instance->CR2 = (SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
        printf("[WORKAROUND] CR2 after write: 0x%04X\r\n", hspi1.Instance->CR2);

        // Step 3: SPI 재활성화 (SPE=1)
        printf("[WORKAROUND] Re-enabling SPI (SPE=1)...\r\n");
        __HAL_SPI_ENABLE(&hspi1);
        printf("[WORKAROUND] CR1 after enable: 0x%04X (SPE=%d)\r\n",
               hspi1.Instance->CR1, (hspi1.Instance->CR1 & SPI_CR1_SPE) ? 1 : 0);

        printf("[WORKAROUND] Final - CR2: 0x%04X, State: %d\r\n",
               hspi1.Instance->CR2, hspi1.State);
    }

    if (status != HAL_OK) {
        printf("[ERROR] *** SPI IT Start FAILED! Status=%d ***\r\n", status);
        printf("[ERROR] This means HAL function returned early without starting transfer\r\n");
        // CS High (에러 시 복구)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
    } else {
        printf("[DEBUG] SPI IT Started OK - waiting for ISR callback...\r\n");
    }
    printf("[DEBUG] === StartReceive end ===\r\n\r\n");
}

/**
 * @brief Listener Tracking Execute (Queue에서 데이터 대기 및 처리)
 * @note freertos.c의 로직을 ap/ 레이어로 이동
 */
void Listener_Tracking_Excute(void) {
    extern osMessageQId g_queue_fpga_data;
    extern osThreadId Controller_TaskHandle;
    extern Model_Tracking_t g_tracking_model;

    uint32_t rx_data;

    // Queue에서 데이터 대기 (Blocking)
    if (xQueueReceive(g_queue_fpga_data, &rx_data, portMAX_DELAY) == pdTRUE)
    {
        // 수신된 데이터를 전역 패킷에 저장
        g_rx_packet_tracking.raw = rx_data;

        // Model 업데이트
        Listener_Tracking_ProcessData(&g_tracking_model);

        // Controller에게 알림
        xTaskNotifyGive(Controller_TaskHandle);

        // 다음 SPI 수신 준비
        Listener_Tracking_StartReceive();
    }
}

/**
 * @brief Queue로부터 받은 데이터를 Model에 업데이트
 * @param model: Tracking Model 포인터
 */
void Listener_Tracking_ProcessData(Model_Tracking_t* model) {
    // 전역 버퍼에서 데이터 복사
    model->rx_packet.raw = g_rx_packet_tracking.raw;

    // 받은 raw bytes 출력 (ISR에서 못하므로 여기서 출력)
    printf("[SPI RX] Bytes:[0x%02X 0x%02X 0x%02X 0x%02X] Raw32=0x%08lX | ",
           rx_buff[0], rx_buff[1], rx_buff[2], rx_buff[3],
           model->rx_packet.raw);

    // 헤더 검증
    if (Model_Tracking_ValidateRx(model)) {
        // 유효한 데이터 → XY 좌표 업데이트
        uint16_t x = model->rx_packet.fields.x_pos;
        uint16_t y = model->rx_packet.fields.y_pos;

        Model_Tracking_UpdateXY(model, x, y);

        // 파싱된 데이터 출력
        printf("Header=0x%02X | X=%4u | Y=%4u [OK]\r\n",
               model->rx_packet.fields.header, x, y);
    } else {
        // 헤더 오류
        printf("Header=0x%02X [INVALID!]\r\n", model->rx_packet.fields.header);
    }
}


