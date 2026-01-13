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
