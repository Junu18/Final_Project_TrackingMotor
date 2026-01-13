/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */


/**
 * @brief  include 파일
 */
#include <stdio.h>
#include <stdbool.h>
#include "Listener_Tracking.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "main.h"
#include "spi.h"


/**
 * @brief  전역 변수 정의
 * @note   SPI Callback에서 사용할 수신 버퍼
 */
RxPacket_t g_rx_packet_tracking;  

/* 송신/수신 버퍼 (인터럽트 모드용) */
uint8_t tx_buff[4] = {0xAA, 0x00, 0x00, 0x00};  // 송신 헤더
uint8_t rx_buff[4] = {0};                       // 수신 버퍼

/**
 * @brief  Listener Tracking 초기화
 * @note   CS 핀 설정 (GPIOC, PIN10, HIGH)
 */
void Listener_Tracking_Init() {
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
 * @brief  Listener Tracking Execute
 * @note   ISR이 모든 처리를 담당 (osMessagePut 직접 호출)
 */
void Listener_Tracking_Excute() {
    // ISR이 모든 처리를 담당하므로 여기서 할 일 없음
}
