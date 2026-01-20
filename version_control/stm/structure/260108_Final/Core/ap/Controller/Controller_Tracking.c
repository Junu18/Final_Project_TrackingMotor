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

	if (currEvent == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
		HAL_UART_Transmit(&huart2, (uint8_t *)"compute\n", 8, 10);
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

	if (currEvent == EVENT_SPI_CMPLT) {
//        Controller_Tracking_Calculate();
	}
}

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

void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
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
}

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

/*
void Controller_Tracking_PushData() {
    static trackingState_t lastSentState = -1;
    trackingState_t currState = Model_GetTrackingState();

    // 1. 상태가 이전과 달라졌다면(IDLE -> SEARCH 등), 아래의 IDLE 가드 조건을 무시하고 무조건 패킷을 보냅니다.
    bool isStateChanged = (currState != lastSentState);

    // 2. 상태가 이전과 같으면서, 그 상태가 IDLE인 경우에만 전송을 하지 않고 리턴합니다.
    if (!isStateChanged && currState == TRACKING_IDLE) {
        return;
    }

    // 메시지 풀 할당 로직 시작
    tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
    if (pTrackingData != NULL) {
        // 현재 데이터(변경된 상태값 포함)를 복사
        memcpy(pTrackingData, &trackingData, sizeof(tracking_t));

        // Presenter에게 전달 시도
        if (osMessagePut(trackingDataMsgBox, (uint32_t)pTrackingData, 0) == osOK) {
            lastSentState = currState; // 전송 성공한 '최신 상태'를 기록
        } else {
            osPoolFree(poolTrackingData, pTrackingData);
        }
    } else {
        // 이 로그가 찍히면 Presenter가 처리를 못해 창고가 꽉 찬 것입니다.
        HAL_UART_Transmit(&huart2, (uint8_t *)"Alloc Fail\n", 11, 10);
    }
}
*/
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
