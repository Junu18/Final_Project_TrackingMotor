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

#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10

// [파일 내부 정적 변수]
tracking_t trackingData;
bool isReadyToPush = false;  // true일때 presenter로 전송

void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}

void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;

	Controller_Tracking_UpdateState(currEvent);
	Controller_Tracking_HandleSignal(currEvent);
}

void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();

	if (currEvent == EVENT_SERVO_TICK) {
//		if (isReadyToPush) {
//		}
			Controller_Tracking_PushData();
	}

	if (currEvent == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
		HAL_UART_Transmit(&huart2, (uint8_t*) "compute\n", 8, 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_PLUS) {
		if (trackingData.x_pos <= MAX_X - DEBUG_GAIN_X_POS) {
			trackingData.x_pos += DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_MINUS) {
		if (trackingData.x_pos >= MIN_X + DEBUG_GAIN_X_POS) {
			trackingData.x_pos -= DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}
	if (currEvent == EVENT_DEBUG_TILT_PLUS) {
		if (trackingData.y_pos <= MAX_Y - DEBUG_GAIN_Y_POS) {
			trackingData.y_pos += DEBUG_GAIN_Y_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current Y_POS: %d\r\n", trackingData.y_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}

	if (currEvent == EVENT_DEBUG_TILT_MINUS) {
		if (trackingData.y_pos >= MIN_Y + DEBUG_GAIN_Y_POS) {
			trackingData.y_pos -= DEBUG_GAIN_Y_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current Y_POS: %d\r\n", trackingData.y_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}
	if (currEvent == EVENT_SPI_CMPLT) {
//        Controller_Tracking_Calculate();
	}
}

/**
 * @brief 상태 관리와 이벤트 처리를 통합 수행하는 메인 함수
 */
void Controller_Tracking_UpdateState(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();

	// 1. 상태별 실행 함수 호출 (전이 및 의사결정)
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
	// 2. 공통 전송 창구 (20ms 틱 박자에 맞춰서 준비된 데이터가 있으면 Push)
//	if (currEvent == EVENT_SERVO_TICK && isReadyToPush) {
//		Controller_Tracking_PushData();
//	}
}

void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		trackingData.motorEnable = true;
		isReadyToPush = true;
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Controller_Tracking_ResetData();
		trackingData.motorEnable = false;
		isReadyToPush = true;
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);
	}
}

void Controller_Tracking_Lost(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed(uint16_t currEvent) {
	if (currEvent == EVENT_CLEAR) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_ResetData() {
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
	trackingData.isDetected = false;
	trackingData.isAimed = false;
	trackingData.motorEnable = false;
}

void Controller_Tracking_ComputeServoAngle() {
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// Apply Deadzone  (Protect from vibration)
	if (abs(diff_x) < 5)
		diff_x = 0;
	if (abs(diff_y) < 5)
		diff_y = 0;

	trackingData.angle_pan += (float) diff_x * GAIN_X;
	trackingData.angle_tilt += (float) diff_y * GAIN_Y;
	isReadyToPush = true;
}

void Controller_Tracking_PushData() {
	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		if (osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0)
				== osOK) {
			isReadyToPush = false;
		} else {
			osPoolFree(poolTrackingData, pTrackingData);
		}
	}
}

/*
 void Controller_Tracking_Calculate() {
 // 1. 패킷 데이터 가져오기 (비트필드 구조체 매핑)
 SpiPacket_t *packet = (SpiPacket_t*) spiRxBuffer;

 // 2. 상태 변화 감지 (Edge Detection)
 // [감지 비트 변화 감지]
 if (trackingData.isDetected == 0 && packet->isDetected == 1) {
 // 이전에 없었는데 지금 생겼다면? -> TARGET_ON!
 osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
 } else if (trackingData.isDetected == 1 && packet->isDetected == 0) {
 // 있었는데 없어졌다면? -> TARGET_LOST!
 osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
 }

 // [조준 비트 변화 감지]
 if (trackingData.isAimed == 0 && packet->isAimed == 1) {
 // 조준이 완료된 시점에 딱 한 번! -> TARGET_AIMED!
 osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
 }

 // 3. 현재 정보를 이전 정보로 업데이트 (다음 루프 비교용)
 trackingData.isDetected = packet->isDetected;
 trackingData.isAimed = packet->isAimed;
 trackingData.x_pos = packet->offsetX;
 trackingData.y_pos = packet->offsetY;

 // 4. 각도 계산 및 전송 (Follow 상태일 때만 의미가 있겠죠?)
 Controller_Tracking_ComputeServoAngle();
 }
 */
