/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"
#include <stdio.h>

Servo_t hServo;
static uint32_t g_servo_update_count = 0;  // Servo 업데이트 횟수

void Presenter_Tracking_Init() {
	// LCD 제거 - UART 디버깅으로 대체
	// LCD_Init(&hi2c1);
	Servo_Init(&hServo, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServo, 90.0f);
	Servo_Start(&hServo);
	printf("[PRES_INIT] Presenter initialized - Servo angle: 90.0\r\n");
}

void Presenter_Tracking_Excute() {
	tracking_t *pTrackingData;
	osEvent evt;
	evt = osMessageGet(trackingDataMsgBox, 0);

	if (evt.status == osEventMessage) {
		pTrackingData = evt.value.p;
		g_servo_update_count++;

		// ===== DEBUG: Servo 업데이트 =====
		if (g_servo_update_count % 50 == 0) {
			printf("[PRES_SERVO] Update #%ld | Angle: %.1f degrees\r\n", 
				   g_servo_update_count, pTrackingData->angle);
		}

		// Servo 각도 설정
		Servo_SetAngle(&hServo, pTrackingData->angle);
		osPoolFree(poolTrackingData, pTrackingData);
	} else if (evt.status != osOK && g_servo_update_count % 100 == 0) {
		printf("[PRES_WARNING] No data in queue (count: %ld)\r\n", g_servo_update_count);
	}
}
