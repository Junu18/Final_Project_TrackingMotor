/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include <stdio.h>

/**
 * @brief 각도 변환 게인 및 화면 설정
 * @note  조정 가능한 상수
 */
#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10
#define SCREEN_CENTER_X		320  // 화면 중심 X (640/2)
#define SCREEN_CENTER_Y		240  // 화면 중심 Y (480/2)
#define ANGLE_GAIN			0.1f // 각도 변환 게인

tracking_t trackingData;
static uint32_t g_event_rx_count = 0;

void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
	printf("[CTRL] Initialized\r\n");
}

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

void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	g_event_rx_count++;

	// 매 1000회마다 디버깅 출력
	if (g_event_rx_count % 1000 == 0) {
		printf("[CTRL] Event count: %ld\r\n", g_event_rx_count);
	}

	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		// SPI에서 받은 데이터 처리 및 언패킹
		Controller_Tracking_Unpack();
	}

	if (currEvent == EVENT_SERVO_TICK) {
		// 20ms 주기 Servo 업데이트
		Controller_Tracking_PushData();
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
	trackingData.angle = 90.0f;
}

void Controller_Tracking_ComputeServoAngle() {
	// 각도 계산 로직
}

void Controller_Tracking_Unpack() {
	// FPGA 패킷 언패킹
	extern RxPacket_t g_rx_packet_tracking;
	// 필요한 데이터 처리
}

void Controller_Tracking_PushData() {
	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t)pTrackingData, 0);
	}
}
