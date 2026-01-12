/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/**
 * @brief model data 전역변수
 * @note  tracking_t 구조체는 Model/Model_Tracking.h에 정의
 *        각도 변환 게인을 조정하기 위한 상수 정의
 * @see   Model_Tracking.h
 */
#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10
#define SCREEN_CENTER_X  320  // 화면 중심 X (640/2)
#define SCREEN_CENTER_Y  240  // 화면 중심 Y (480/2)

#define ANGLE_GAIN       0.1f // 각도 변환 게인 (조정 가능)

/**
 * @brief model data 전역변수
 */
tracking_t trackingData;


/**
 * @brief Controller 초기화
 */
void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}


/**
 * @brief Controller Tracking 실행
 * @note Event 수신 → 상태 업데이트 → 신호 처리
 */
void Controller_Tracking_Excute() {

	// Event 수신 (non-blocking, timeout=0)
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;

	// 상태 업데이트
	Controller_Tracking_UpdateState(currEvent);

	// 신호 처리
	Controller_Tracking_HandleSignal(currEvent);
}


/**
 * @brief Controller Tracking 실행
 */
void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();
	static trackingState_t prevState = TRACKING_IDLE;

	if (state != prevState) {
		Controller_Tracking_PushData();
		prevState = state;
	}

	if (currEvent == EVENT_SERVO_TICK) {
		if (state != TRACKING_IDLE) {
			Controller_Tracking_PushData();
		}
	}

	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		// FPGA에서 받은 SPI 데이터 처리
		extern RxPacket_t g_rx_packet_tracking;
		extern uint8_t rx_buff[4];
		static uint32_t rx_count = 0;
		static uint32_t valid_count = 0;

		rx_count++;

		// 디버그 출력 (10000번에 한 번)
		if (rx_count % 10000 == 0) {
			printf("[FPGA] RX #%lu: [0x%02X 0x%02X 0x%02X 0x%02X] Raw=0x%08lX\r\n",
				   rx_count, rx_buff[0], rx_buff[1], rx_buff[2], rx_buff[3],
				   g_rx_packet_tracking.raw);
		}

		// 헤더 검증
		if (g_rx_packet_tracking.fields.header == 0x55) {
			uint16_t x = g_rx_packet_tracking.fields.x_pos;
			uint16_t y = g_rx_packet_tracking.fields.y_pos;

			valid_count++;

			// 좌표 업데이트 및 패킷 언패킹
			Controller_Tracking_Unpack();

			if (rx_count % 10000 == 0) {
				printf("[FPGA] Valid #%lu: X=%u, Y=%u\r\n", valid_count, x, y);
			}
		}
	}
}

/**
 * @brief Controller State update
 * @note  Controller FSM 구현
 */
void Controller_Tracking_UpdateState(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();

	switch (state) {
	case TRACKING_IDLE:
		Controller_Tracking_Idle(currEvent);
		break;
	case TRACKING_SEARCH:
		Controller_Tracking_Search(currEvent);
		break;
	case TRACKING_FOLLOW:
		Controller_Tracking_Follow(currEvent);
		break;
	case TRACKING_LOST:
		Controller_Tracking_Lost(currEvent);
		break;
	case TRACKING_AIMED:
		Controller_Tracking_Aimed(currEvent);
		break;
	}
	trackingState_t currState = Model_GetTrackingState();
	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[50];
	if (state != currState) {
		sprintf(str, "\r\n[EVENT: %d] STATE CHANGE: %s -> %s\r\n", currEvent,
				stateNames[state], stateNames[currState]);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 10);
	}
}


/**
 * @brief IDlE
 * @note  EVENT_START 수신 시 SEARCH 상태로 전이
 */
void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);			// start 이벤트 수신 시 SEARCH 상태로 전이
	}
}


/**
 * @brief SEARCH
 * @note  EVENT_STOP 수신 시 IDLE 상태로 전이
 *        EVENT_TARGET_ON 수신 시 FOLLOW 상태로 전이
 */
void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Model_SetTrackingState(TRACKING_IDLE);				// stop 이벤트 수신 시 IDLE 상태로 전이
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW); 			// target 감지 시 FOLLOW 상태로 전이
	}
}


/**
 * @brief FOLLOW
 * @note  EVENT_TARGET_LOST 수신 시 LOST 상태로 전이
 *        EVENT_TARGET_AIMED 수신 시 AIMED 상태로 전이
 */
void Controller_Tracking_Follow(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);				// target 잃어버리면 LOST 상태로 전이
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);				// target 조준 완료 시 AIMED 상태로 전이
	}
}

/**
 * @brief LOST
 * @note  EVENT_TARGET_ON 수신 시 FOLLOW 상태로 전이
 */
void Controller_Tracking_Lost(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);    		// target 재감지 시 FOLLOW 상태로 전이
															// 왜 이게 재감지 이지?
	}
}


/**
 * @brief AIMED
 * @note  EVENT_CLEAR 수신 시 SEARCH 상태로 전이
 */
void Controller_Tracking_Aimed(uint16_t currEvent) {
	if (currEvent == EVENT_CLEAR) {
		Model_SetTrackingState(TRACKING_SEARCH);         // clear 이벤트 수신 시 SEARCH 상태로 전이
	}
}

/**
 * @brief Controller Tracking 데이터 리셋
 * @note  tracking_t 구조체의 모든 필드를 초기값으로 설정
 */
void Controller_Tracking_ResetData() {
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
	trackingData.is_Detected = false;
	trackingData.is_Aimed = false;
}


/**
 * @brief 서보모터 각도 계산
 * @note  현재 타겟 좌표를 기반으로 서보모터 각도 계산
 *        GAIN_X/Y 상수를 조정하여 반응 속도 변경 가능
 *        Deadzone 적용으로 떨림 방지
 */
void Controller_Tracking_ComputeServoAngle() {
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// Apply Deadzone  (Protect from vibration)
	if (abs(diff_x) < 5)
		diff_x = 0;
	if (abs(diff_y) < 5)
		diff_y = 0;

	trackingData.angle_pan += (float)diff_x * GAIN_X;
	trackingData.angle_tilt += (float)diff_y * GAIN_Y;
}


/**
 * @brief Tracking Data 전송
 * @note  Pool에서 할당된 Data를 Message Queue로 전송
 *        prevData와 비교하여 변경된 경우에만 전송
 *        Presenter에서 수신하여 서보모터 제어 및 LCD 업데이트 수행
 */

void Controller_Tracking_PushData() {
	static tracking_t prevData;

//	if (!memcmp(&trackingData, &prevData, sizeof(tracking_t)))
//		return;
	memcpy(&prevData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
	}
}


/**
 * @brief FPGA 패킷 언패킹 (Unpack received FPGA packet)
 * @note  RxPacket_t에서 데이터를 추출하여 tracking_t 구조체에 저장
 */
void Controller_Tracking_Unpack() {
	// 1. 외부 전역 변수 참조 (ISR에서 저장한 패킷)
	extern RxPacket_t g_rx_packet_tracking;

	// 2. 패킷에서 X, Y 좌표 추출
	uint16_t x = g_rx_packet_tracking.fields.x_pos;
	uint16_t y = g_rx_packet_tracking.fields.y_pos;

	// 3. tracking_t 구조체에 저장
	trackingData.x_pos = x;
	trackingData.y_pos = y;

	// 4. 수신 횟수 증가
	trackingData.rx_count++;

	// 5. 패킷 원본 데이터 저장 (디버깅용)
	trackingData.rx_packet.raw = g_rx_packet_tracking.raw;

	// 6. 타겟 감지 상태 업데이트
	// TODO: 팀원과 협의하여 isDetected, isAimed 비트 추가 필요 시 구현
	trackingData.is_Detected = true;  // 데이터 수신 = 타겟 감지됨

	// 7. 각도 계산 (필요 시 호출)
	// Controller_Tracking_ComputeServoAngle();
}



