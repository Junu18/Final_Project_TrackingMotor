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

tracking_t trackingData;
static uint16_t currEvent;

void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
	Controller_Tracking_PushData();
}

void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, osWaitForever);
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;

	Controller_Tracking_UpdateState();
	Controller_Tracking_HandleSignal();
}

void Controller_Tracking_HandleSignal() {
	trackingState_t state = Model_GetTrackingState();
	if (currEvent == EVENT_SERVO_TICK && state != TRACKING_IDLE) {
		Controller_Tracking_PushData();
//        Controller_Tracking_RequestSPI();
	}

	if (currEvent == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
	}

	if (currEvent == EVENT_DEBUG_PAN_PLUS) {
		HAL_UART_Transmit(&huart2, (uint8_t *)"pan plus\n", 9, 1000);
		if (trackingData.x_pos <= MAX_X - DEBUG_GAIN_X_POS) {
			trackingData.x_pos += DEBUG_GAIN_X_POS;
		}
	}

	if (currEvent == EVENT_DEBUG_PAN_MINUS) {
		if (trackingData.x_pos >= MIN_X + DEBUG_GAIN_X_POS) {
			trackingData.x_pos -= DEBUG_GAIN_X_POS;
		}
	}

	if (currEvent == EVENT_SPI_CMPLT) {
//        Controller_Tracking_Calculate();
	}
}

void Controller_Tracking_UpdateState() {
	trackingState_t state = Model_GetTrackingState();

	switch (state) {
	case TRACKING_IDLE:
		Controller_Tracking_Idle();
		break;
	case TRACKING_SEARCH:
		Controller_Tracking_Search();
		break;
	case TRACKING_FOLLOW:
		Controller_Tracking_Follow();
		break;
	case TRACKING_LOST:
		Controller_Tracking_Lost();
		break;
	case TRACKING_AIMED:
		Controller_Tracking_Aimed();
		break;
	}
	trackingState_t currState = Model_GetTrackingState();
	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[50];
	if (state != currState) {
		sprintf(str, "\r\n[EVENT: %d] STATE CHANGE: %s -> %s\r\n", currEvent,
				stateNames[state], stateNames[currState]);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 1000);
	}
}

void Controller_Tracking_Idle() {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search() {
	if (currEvent == EVENT_STOP) {
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow() {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);
	}
}

void Controller_Tracking_Lost() {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed() {
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
}

void Controller_Tracking_PushData() {
	HAL_UART_Transmit(&huart2, (uint8_t *)"test\n", 5, 1000);
	static tracking_t prevData;

	if (!memcmp(&trackingData, &prevData, sizeof(tracking_t)))
		return;
	memcpy(&prevData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
	}
}

/*
 void Controller_Tracking_Calculate() {
 // 1. 패킷 데이터 가져오기 (비트필드 구조체 매핑)
 SpiPacket_t *packet = (SpiPacket_t *)spiRxBuffer;

 // 2. 상태 변화 감지 (Edge Detection)
 // [감지 비트 변화 감지]
 if (trackingData.isDetected == 0 && packet->isDetected == 1) {
 // 이전에 없었는데 지금 생겼다면? -> TARGET_ON!
 osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
 }
 else if (trackingData.isDetected == 1 && packet->isDetected == 0) {
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
