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
//        Controller_Tracking_Calculate();
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
	trackingData.isDetected = false;
	trackingData.isAimed = false;
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


 