/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"
#include <stdio.h>

Servo_t hServo;
static uint32_t g_servo_count = 0;

void Presenter_Tracking_Init() {
	Servo_Init(&hServo, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServo, 90.0f);
	Servo_Enable(&hServo);
	printf("[PRES] Init OK\r\n");
	for(volatile int i = 0; i < 100000; i++);
}

void Presenter_Tracking_Excute() {
	tracking_t *pTrackingData;
	osEvent evt;
	evt = osMessageGet(trackingDataMsgBox, 0);

	if (evt.status == osEventMessage) {
		pTrackingData = (tracking_t *)evt.value.p;
		g_servo_count++;

		// 매 100회마다 디버깅 출력 (자주 출력)
		if (g_servo_count % 100 == 0) {
			printf("[SERVO] Pan:%d\r\n", (int)pTrackingData->angle_pan);
		}

		// 서보모터 제어
		Servo_SetAngle(&hServo, pTrackingData->angle_pan);
		osPoolFree(poolTrackingData, pTrackingData);
	}
}
	}
}


/**
 * @brief Presenter Tracking 상태 업데이트
 * @note  서보모터 각도 설정
 */
void Presenter_Tracking_UpdateState(tracking_t *pTrackingData) {
	// ① 현재 추적 상태 조회 (Model에서)
	trackingState_t state = Model_GetTrackingState();


	// ② 서보모터 각도 설정
	Presenter_Tracking_ManageServoPower(state);

	// ③ IDLE이 아닐 때만 서보 각도 설정
	if (state != TRACKING_IDLE) {
		Servo_SetAngle(&hServoPan, pTrackingData->angle_pan);
		Servo_SetAngle(&hServoTilt, pTrackingData->angle_tilt);
	}
}

/**
 * @brief 서보모터 전원 관리
 * @note   서보모터 전원 관리
  - TRACKING_IDLE 상태 진입 시 서보모터 비활성화
  - TRACKING_IDLE 상태에서 벗어날 때 서보모터 활성화
 */

void Presenter_Tracking_ManageServoPower(trackingState_t currState) {
	static trackingState_t prevState = TRACKING_IDLE;					// 이전 상태 기억
	
	// 상태 변화 없으면 조기 리턴 (불필요한 호출 방지)
	if (currState == prevState)
		return;

	 // ① IDLE → 서보 모두 OFF		
	if (currState == TRACKING_IDLE) {
		Servo_Disable(&hServoPan);
		Servo_Disable(&hServoTilt);
	} 
	 // ② IDLE → (SEARCH/FOLLOW 등) → 서보 모두 ON
	else if (prevState == TRACKING_IDLE) {
		Servo_Enable(&hServoPan);
		Servo_Enable(&hServoTilt);
	}
	prevState = currState;							 // 다음 루프를 위해 현재 상태 저장
}

void Presenter_Tracking_DispLCD(tracking_t *pTrackingData) {
//	static int updateDivider = 0;
//	if (++updateDivider < 10)
//		return;
//	updateDivider = 0;

//	static int debugMessage = 0;
//	if (debugMessage == 0) {
//		LCD_WriteStringXY(0, 0, "0");
//		debugMessage = 1;
//	} else {
//		LCD_WriteStringXY(0, 0, "1");
//		debugMessage = 0;
//	}

	static int lastPan = -1, lastTilt = -1, lastX = -1, lastY = -1;
	static trackingState_t lastState = -1;

	int currPan = (int) pTrackingData->angle_pan;
	int currTilt = (int) pTrackingData->angle_tilt;
	int currX = (int) pTrackingData->x_pos;
	int currY = (int) pTrackingData->y_pos;
	trackingState_t currState = Model_GetTrackingState();

	if (currPan != lastPan || currTilt != lastTilt || currX != lastX
			|| currY != lastY || currState != lastState) {

		char str[20];

		char stateChar = (currState == TRACKING_IDLE) ? 'I' :
							(currState == TRACKING_SEARCH) ? 'S' : 'L';

		sprintf(str, "%c X:%03d Y:%03d   ", stateChar, currX, currY);
		LCD_WriteStringXY(0, 0, str);

		sprintf(str, "P:%03d  T:%03d     ", currPan, currTilt);
		LCD_WriteStringXY(1, 0, str);

		lastPan = currPan;
		lastTilt = currTilt;
		lastX = currX;
		lastY = currY;
		lastState = currState;
	}
}
