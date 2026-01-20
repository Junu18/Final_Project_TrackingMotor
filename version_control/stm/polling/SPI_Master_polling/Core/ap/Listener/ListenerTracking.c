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
 *       ISR 콜백에서도 호출되어 연속 수신을 구현
 */
void Listener_Tracking_StartReceive(void) {
    // CS Low (통신 시작)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);

    // [FIX] SPI 페리페럴 명시적 활성화 (SPE bit 설정)
    if ((hspi1.Instance->CR1 & SPI_CR1_SPE) == 0) {
        __HAL_SPI_ENABLE(&hspi1);
    }

    // SPI 인터럽트 모드로 송수신 시작
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_IT(&hspi1, tx_buff, rx_buff, 4);

    // [FIX] HAL 함수가 CR2를 설정하지 않는 버그 우회
    if (status == HAL_OK && hspi1.Instance->CR2 == 0x0000) {
        // State와 버퍼를 수동으로 설정
        hspi1.State = HAL_SPI_STATE_BUSY_TX_RX;
        hspi1.pTxBuffPtr = tx_buff;
        hspi1.pRxBuffPtr = rx_buff;
        hspi1.TxXferCount = 4;
        hspi1.RxXferCount = 4;
        hspi1.TxXferSize = 4;
        hspi1.RxXferSize = 4;

        // STM32 하드웨어 제약: SPE=1 상태에서는 CR2 수정 불가
        // 해결: SPE 비활성화 → CR2 설정 → SPE 재활성화
        __HAL_SPI_DISABLE(&hspi1);
        hspi1.Instance->CR2 = (SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
        __HAL_SPI_ENABLE(&hspi1);
    }

    if (status != HAL_OK) {
        printf("[ERROR] SPI IT Start FAILED! Status=%d\r\n", status);
        // CS High (에러 시 복구)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
    }
}

/**
 * @brief Listener Tracking Execute (Reference Architecture)
 * @note Reference 방식: Listener는 폴링만 담당, 실제 데이터는 ISR에서 Event 발행
 *       우리는 SPI 인터럽트 방식이므로 Execute는 아무것도 하지 않음
 */
void Listener_Tracking_Excute(void) {
    // Reference Architecture:
    // - StopWatch: 버튼 폴링 → Event 발행
    // - Tracking: SPI 인터럽트 → ISR에서 Event 발행
    //
    // 따라서 여기서는 아무것도 하지 않음
    // (osDelay는 freertos.c에서 처리)
}
