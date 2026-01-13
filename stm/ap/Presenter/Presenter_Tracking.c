/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"

Servo_t hServo;
static uint32_t g_servo_count = 0;

void Presenter_Tracking_Init() {
	Servo_Init(&hServo, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServo, 90.0f);
	Servo_Enable(&hServo);
	printf("[PRES] Init OK\r\n");
}

void Presenter_Tracking_Excute() {
	tracking_t *pTrackingData;
	osEvent evt;
	evt = osMessageGet(trackingDataMsgBox, 0);

	if (evt.status == osEventMessage) {
		pTrackingData = (tracking_t *)evt.value.p;
		g_servo_count++;

		// 매 1000회마다 디버깅 출력
		if (g_servo_count % 1000 == 0) {
			printf("[PRES] Servo updates: %ld | Pan: %d, Tilt: %d\r\n", 
				   g_servo_count, (int)pTrackingData->angle_pan, (int)pTrackingData->angle_tilt);
		}

		Servo_SetAngle(&hServo, pTrackingData->angle_pan);
		osPoolFree(poolTrackingData, pTrackingData);
	}
}
