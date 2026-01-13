/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include <stdio.h>
#include <stdlib.h>

tracking_t trackingData;
static uint32_t g_event_rx_count = 0;
 */
tracking_t trackingData;


/**
 * @brief Controller 초기화
 */
void Controller_Tracking_Init() {
	// TIM3 Base Interrupt 시작 (20ms 주기로 Servo 업데이트)
	// → SPI 인터럽트와 타이밍 분리
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}


/**
void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
	printf("[CTRL] Init OK\r\n");
	for(volatile int i = 0; i < 100000; i++);
}

void Controller_Tracking_Excute() {
	// Event 수신 (non-blocking, timeout=0)
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	
	// [DEBUG] 이벤트 받는지 확인
	static uint32_t loop_count = 0;
	loop_count++;
	if (loop_count % 500 == 0) {
		printf("[CTRL] Loop running (no event received)\r\n");
	}
	
	if (evt.status != osEventMessage) {
		return;  // 이벤트 없음 (정상)
	}
	currEvent = evt.value.v;
	printf("[CTRL] Event RX: %d\r\n", currEvent);  // 이벤트 받을 때마다 프린트

	// 상태 업데이트
	Controller_Tracking_UpdateState(currEvent);

	// 신호 처리
	Controller_Tracking_HandleSignal(currEvent);
}

void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	g_event_rx_count++;

	// 매 1000회마다 디버깅 출력
	if (g_event_rx_count % 1000 == 0) {
		printf("[CTRL] Evt=%d\r\n", currEvent);
	}

	// 1. SPI에서 FPGA 데이터 수신
	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		// 패킷에서 x, y 좌표 추출
		Controller_Tracking_Unpack();
		printf("[DATA] X:%d Y:%d\r\n", trackingData.x_pos, trackingData.y_pos);
	}

	// 2. 20ms 주기 Servo 업데이트 (TIM3 인터럽트)
	if (currEvent == EVENT_SERVO_TICK) {
		// 좌표 → 각도 계산
		Controller_Tracking_ComputeServoAngle();
		printf("[ANGLE] Pan:%d\r\n", (int)trackingData.angle_pan);
		
		// Presenter에 데이터 전송
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
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
	trackingData.is_Detected = false;
	trackingData.is_Aimed = false;
}

void Controller_Tracking_ComputeServoAngle() {
	// 화면 중심과의 차이 계산
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// 데드존 적용 (떨림 방지)
	if (abs(diff_x) < 5) diff_x = 0;
	if (abs(diff_y) < 5) diff_y = 0;

	// 각도 계산: 중심에서 ±90도 범위
	trackingData.angle_pan = CENTER_PAN + (diff_x * GAIN_X);
	trackingData.angle_tilt = CENTER_TILT + (diff_y * GAIN_Y);

	// 각도 범위 제한 (0~180도)
	if (trackingData.angle_pan < 0) trackingData.angle_pan = 0;
	if (trackingData.angle_pan > 180) trackingData.angle_pan = 180;
	if (trackingData.angle_tilt < 0) trackingData.angle_tilt = 0;
	if (trackingData.angle_tilt > 180) trackingData.angle_tilt = 180;
}

void Controller_Tracking_Unpack() {
	// SPI ISR에서 저장한 FPGA 패킷 참조
	extern RxPacket_t g_rx_packet_tracking;
	
	// 패킷에서 헤더 검증 (0x55)
	if (g_rx_packet_tracking.fields.header != 0x55) {
		trackingData.rx_error_count++;
		return;
	}

	// 좌표 추출
	trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
	trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
	trackingData.rx_packet.raw = g_rx_packet_tracking.raw;
	trackingData.rx_count++;
	trackingData.is_Detected = true;
}

void Controller_Tracking_PushData() {
	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t)pTrackingData, 0);
	}
}



