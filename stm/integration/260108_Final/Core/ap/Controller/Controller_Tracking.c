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

// ========================================
// Averaging Buffer (평균값 계산용)
// ========================================
#define AVERAGING_BUFFER_SIZE 10

typedef struct {
	uint16_t x_buffer[AVERAGING_BUFFER_SIZE];
	uint16_t y_buffer[AVERAGING_BUFFER_SIZE];
	uint8_t buffer_index;
	uint8_t sample_count;
} AveragingBuffer_t;

static AveragingBuffer_t avg_buffer = {0};

/**
 * @brief 평균값 계산 함수
 * @note 10개 샘플 모아서 평균 계산
 */
void Controller_Tracking_Calculate_Average() {
	if (avg_buffer.sample_count < AVERAGING_BUFFER_SIZE) {
		return;  // 아직 샘플 부족
	}
	
	// 평균값 계산
	uint32_t sum_x = 0, sum_y = 0;
	for (int i = 0; i < AVERAGING_BUFFER_SIZE; i++) {
		sum_x += avg_buffer.x_buffer[i];
		sum_y += avg_buffer.y_buffer[i];
	}
	
	trackingData.x_pos = sum_x / AVERAGING_BUFFER_SIZE;
	trackingData.y_pos = sum_y / AVERAGING_BUFFER_SIZE;
	
	// 디버그 출력 (100회마다)
	static uint32_t avg_count = 0;
	avg_count++;
	if (avg_count % 100 == 0) {
		printf("[AVG] X:%u Y:%u (samples collected)\r\n", 
		       trackingData.x_pos, trackingData.y_pos);
	}
	
	// 버퍼 리셋
	avg_buffer.sample_count = 0;
	avg_buffer.buffer_index = 0;
}

/**
 * @brief 버퍼에 샘플 추가
 */
void Controller_Tracking_Add_Sample(uint16_t x, uint16_t y) {
	if (avg_buffer.sample_count < AVERAGING_BUFFER_SIZE) {
		avg_buffer.x_buffer[avg_buffer.buffer_index] = x;
		avg_buffer.y_buffer[avg_buffer.buffer_index] = y;
		avg_buffer.buffer_index++;
		avg_buffer.sample_count++;
	}
}

/**
 * @brief Controller 초기화
 */
void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
	printf("[CTRL] Init OK\r\n");
	for(volatile int i = 0; i < 100000; i++);
}

/**
 * @brief Controller Execute
 */
void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	
	static uint32_t loop_count = 0;
	loop_count++;
	if (loop_count % 500 == 0) {
		printf("[CTRL] Loop running (no event received)\r\n");
	}
	
	if (evt.status != osEventMessage) {
		return;
	}
	currEvent = evt.value.v;

	// 1. SPI에서 FPGA 데이터 수신 (1ms 주기)
	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		extern RxPacket_t g_rx_packet_tracking;
		
		// 헤더 검증
		if (g_rx_packet_tracking.fields.header == 0x55) {
			uint16_t x = g_rx_packet_tracking.fields.x_pos;
			uint16_t y = g_rx_packet_tracking.fields.y_pos;
			
			// 버퍼에 샘플 추가
			Controller_Tracking_Add_Sample(x, y);
			
			printf("[DATA] X:%u Y:%u (Buffer:%u/%u)\r\n", 
			       x, y, avg_buffer.sample_count, AVERAGING_BUFFER_SIZE);
		}
	}

	// 2. 20ms 주기 Servo 업데이트 (TIM3 인터럽트)
	if (currEvent == EVENT_SERVO_TICK) {
		// 평균값 계산
		Controller_Tracking_Calculate_Average();
		
		// 좌표 → 각도 계산
		Controller_Tracking_ComputeServoAngle();
		printf("[ANGLE] Pan:%d\r\n", (int)trackingData.angle_pan);
		
		// Presenter에 데이터 전송
		Controller_Tracking_PushData();
	}

	// 상태 업데이트
	Controller_Tracking_UpdateState(currEvent);
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
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	if (abs(diff_x) < 5) diff_x = 0;
	if (abs(diff_y) < 5) diff_y = 0;

	trackingData.angle_pan = CENTER_PAN + (diff_x * GAIN_X);
	trackingData.angle_tilt = CENTER_TILT + (diff_y * GAIN_Y);

	if (trackingData.angle_pan < 0) trackingData.angle_pan = 0;
	if (trackingData.angle_pan > 180) trackingData.angle_pan = 180;
	if (trackingData.angle_tilt < 0) trackingData.angle_tilt = 0;
	if (trackingData.angle_tilt > 180) trackingData.angle_tilt = 180;
}

void Controller_Tracking_PushData() {
	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t)pTrackingData, 0);
	}
}



