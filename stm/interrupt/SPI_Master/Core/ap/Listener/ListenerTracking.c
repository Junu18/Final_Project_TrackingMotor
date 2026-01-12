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
#include <stdbool.h>

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
 * @brief SPI 인터럽트 수신 시작 (완전 수동 구현)
 * @note HAL 완전히 우회하여 직접 레지스터 설정
 *       수업 자료 방식: 직접 IRQ Handler 구현
 */
void Listener_Tracking_StartReceive(void) {
    printf("[StartRX] SPI interrupt mode starting...\r\n");

    // ========================================
    // 완전 수동 구현 (HAL 우회)
    // ========================================

    // 1. SPE disable (안전한 설정을 위해)
    SPI1->CR1 &= ~SPI_CR1_SPE;
    __DSB();

    // 2. CR2 인터럽트 enable (SPE=0 상태에서)
    SPI1->CR2 = SPI_CR2_RXNEIE | SPI_CR2_TXEIE | SPI_CR2_ERRIE;
    __DSB();

    // 3. SPE enable
    SPI1->CR1 |= SPI_CR1_SPE;
    __DSB();

    // 4. CS Low (FPGA 통신 시작)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);

    // 5. 첫 바이트 전송 (TXE 인터럽트 트리거)
    SPI1->DR = tx_buff[0];

    printf("[StartRX] Started! CR2=0x%X\r\n", (unsigned int)SPI1->CR2);
}

/**
 * @brief Listener Tracking Execute (수업 자료 방식)
 * @note ISR이 모든 처리를 담당 (osMessagePut 직접 호출)
 *       Listener Execute는 필요 없지만 Reference Architecture 유지
 */
void Listener_Tracking_Excute(void) {
    // ========================================
    // [수업 자료 패턴]
    // ISR이 직접 처리:
    //   1. CS 제어
    //   2. 데이터 조립
    //   3. Event 발행 (osMessagePut)
    //   4. SPI 재시작 (연속 수신)
    //
    // → Listener Execute는 할 일이 없음
    // ========================================

    // Reference Architecture 유지를 위해 함수는 존재하지만
    // 실제 로직은 ISR에서 모두 처리됨
}
