/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"


/**
 * @brief 서보모터 초기화 및 중앙 위치 설정
 * @note  Servo_t 구조체는 ServoMotor.h에 정의
 */
Servo_t hServoPan;
Servo_t hServoTilt;


/**
 * @brief Presenter 초기화
 * @note  
 */
void Presenter_Tracking_Init() {
	LCD_Init(&hi2c1);
	Servo_Init(&hServoPan, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServoPan, CENTER_PAN);
	Servo_Disable(&hServoPan);

	Servo_Init(&hServoTilt, &htim3, TIM_CHANNEL_2);
	Servo_SetAngle(&hServoTilt, CENTER_TILT);
	Servo_Disable(&hServoTilt);
}


/**
 * @brief Presenter Tracking Execute
 * @note  Data 수신 → 서보 업데이트 → LCD 업데이트 → Pool 해제
 */
void Presenter_Tracking_Excute() {
	static int freeCount = 0;

	// ① Controller에서 보낸 메시지(포인터) 수신 (대기)
	osEvent evt = osMessageGet(trackingDataMsgBox, osWaitForever);
	tracking_t *pTrackingData;
	 // 메시지 수신 실패 시 조기 리턴
	if (evt.status != osEventMessage)
		return;

	// ② 포인터 추출 (Controller가 보낸 tracking_t* 주소)
	// 포인터를 추출 하는 이유는 메시지 큐에 구조체 전체를 복사하는 것은 비효율적이기 때문
	pTrackingData = (tracking_t*) evt.value.p;

	// ③ 상태 갱신 + 서보 제어
	Presenter_Tracking_UpdateState(pTrackingData);
//	Presenter_Tracking_DispLCD(pTrackingData);

	// ⑤ 메모리 반환 (Controller가 할당한 메모리 풀 블록 해제)
	if (osPoolFree(poolTrackingData, pTrackingData) == osOK) {
		freeCount++;
		if (freeCount % 10 == 0) { // 너무 자주 찍히면 느려지니까 10번에 한 번만
			char buf[20];
			sprintf(buf, "Free OK: %d\r\n", freeCount);
			HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 10);
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
