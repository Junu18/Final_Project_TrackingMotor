/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"

Servo_t hServo_x;
Servo_t hServo_y;


void Presenter_Tracking_Init() {
	LCD_Init(&hi2c1);

	//x축 모터 초기화
	Servo_Init(&hServo_x, &htim3, TIM_CHANNEL_1);
	Servo_Start(&hServo_x);
	Servo_SetAngle(&hServo_x, 90.0f);

	//y축 모터 초기화
	Servo_Init(&hServo_y, &htim3, TIM_CHANNEL_2);
	Servo_Start(&hServo_y);
	Servo_SetAngle(&hServo_y, 90.0f);
}

//Excute 계속 반복
void Presenter_Tracking_Excute() {

	// x,y 모터각도 구조체 선언
	tracking_t *pTrackingData;
	osEvent evt; //데이터 요청응답 변수선언

	//osEvent
	evt = osMessageGet(trackingDataMsgBox, osWaitForever); //메세시 받음

	//만약 이벤트 메세지를 받았다면
	if (evt.status == osEventMessage)
	{
		//받은 주소를 xy좌표 설계도(tracking_t)에 맞게 해석
		pTrackingData = (tracking_t *)evt.value.p;

		//LCD에 x,y 각도 표시(1행, 2행)
		char str_x[50], str_y[20];
		sprintf(str_x, "X_Angle : %03d", (int)pTrackingData-> x_angle);
		sprintf(str_y, "Y_Angle : %03d", (int)pTrackingData -> y_angle);
		LCD_WriteStringXY(0, 0, str_x);
		LCD_WriteStringXY(1, 0, str_y);

		// 6. 각 모터 핸들에 맞춰 계산된 각도 적용
		Servo_SetAngle(&hServo_x, (float)pTrackingData ->x_angle);
		Servo_SetAngle(&hServo_y, (float)pTrackingData ->y_angle);

		osPoolFree(poolTrackingData, pTrackingData); //공유메모리 프리상태
	}
}



//osEvent : osMessageGet같은 함수 실행시 RTOS는 단순 데이터만 주는게 아니라 여러 정보를 담은 응답신호를 리턴함
// status(성공, 타임아웃, 에러 등), Value(값, 주소), Def(어디서 받은것인지?)
// ptrackingData = evt.value.p 의미
// evt.value.p quequ에서 전달받은 osPool 주소값

// pTrackingData : 이 주소를 저장할 포인터 변수
