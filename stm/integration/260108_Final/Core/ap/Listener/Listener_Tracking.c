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

/**
 * @brief  송신/수신 버퍼 (인터럽트 모드용)
 * @note   샘플링 레이트 설정 후 Enable 필요
 */
uint8_t tx_buff[4] = {0xAA, 0x00, 0x00, 0x00};  // 송신 헤더 (Callback에서 접근)
uint8_t rx_buff[4] = {0};                       // 수신 버퍼 (Callback에서 접근)

/**
 * @brief  Button 핸들 정의
 * @note   각 버튼에 대한 핸들 선언
 */
hBtn hbtnStart;
hBtn hbtnStop;
hBtn hbtnClear;
hBtn hbtnTargetOn;
hBtn hbtnTargetLost;
hBtn hbtnTargetAimed;
hBtn hbtnDebug;
hBtn hbtnDebugPanPlus;
hBtn hbtnDebugPanMinus;

/**
 * @brief  Listener Tracking 초기화
 * @note   각 버튼 핸들 초기화, CS 핀 설정(GPIOC, PIN10, HIGH)
 */
void Listener_Tracking_Init() {
	Button_Init(&hbtnStart, BTN_START_GPIO, BTN_START_PIN);
	Button_Init(&hbtnStop, BTN_STOP_GPIO, BTN_STOP_PIN);
	Button_Init(&hbtnClear, BTN_CLEAR_GPIO, BTN_CLEAR_PIN);
	Button_Init(&hbtnTargetOn, BTN_TARGET_ON_GPIO, BTN_TARGET_ON_PIN);
	Button_Init(&hbtnTargetLost, BTN_TARGET_LOST_GPIO, BTN_TARGET_LOST_PIN);
	Button_Init(&hbtnTargetAimed, BTN_TARGET_AIMED_GPIO, BTN_TARGET_AIMED_PIN);
	Button_Init(&hbtnDebug, BTN_DEBUG_GPIO, BTN_DEBUG_PIN);
	Button_Init(&hbtnDebugPanPlus, BTN_DEBUG_PAN_PLUS_GPIO,
			BTN_DEBUG_PAN_PLUS_PIN);
	Button_Init(&hbtnDebugPanMinus, BTN_DEBUG_PAN_MINUS_GPIO,
			BTN_DEBUG_PAN_MINUS_PIN);
	// CS 핀 초기 상태: High (통신 대기)
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
	//.printf("[Listener] Tracking Listener Initialized\r\n");
}





/**
 * @brief  Listener Tracking 초기화
 * @note   각 버튼 핸들 초기화, CS 핀 설정(GPIOC, PIN10, HIGH)
 */
void Listener_Tracking_CheckButton() {
	if (Button_GetState(&hbtnStart) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_START, 0);
	} else if (Button_GetState(&hbtnStop) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_STOP, 0);
	} else if (Button_GetState(&hbtnClear) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_CLEAR, 0);
	} else if (Button_GetState(&hbtnTargetOn) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
	} else if (Button_GetState(&hbtnTargetLost) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
	} else if (Button_GetState(&hbtnTargetAimed) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
	} else if (Button_GetState(&hbtnDebug) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG, 0);
	} else if (Button_GetState(&hbtnDebugPanPlus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_PAN_PLUS, 0);
	} else if (Button_GetState(&hbtnDebugPanMinus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_PAN_MINUS, 0);
	}
}


/**
 * @brief  Listener Tracking TIM ISR
 * @note   Servo 타이머 인터럽트 핸들러
 */
void Listener_Tracking_TIM_ISR() {
	osMessagePut(trackingEventMsgBox, EVENT_SERVO_TICK, 0);
}


/**
 * @brief SPI 인터럽트 수신 시작
 * @note 직접 레지스터 설정으로 수신 시작
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
	Listener_Tracking_CheckButton();
}

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
