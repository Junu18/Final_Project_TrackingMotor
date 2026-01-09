/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"

Servo_t hServo;

void Presenter_Tracking_Init() {
	LCD_Init(&hi2c1);
	Servo_Init(&hServo, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServo, 90.0f);
	Servo_Start(&hServo);
}

void Presenter_Tracking_Excute() {
	tracking_t *pTrackingData;
	osEvent evt;
	evt = osMessageGet(trackingDataMsgBox, 0);

	if (evt.status == osEventMessage) {
		pTrackingData = evt.value.p;
		char str[50];
		sprintf(str, "%03d", (int)pTrackingData->x_angle);
		LCD_WriteStringXY(1, 0, str);
		Servo_SetAngle(&hServo, pTrackingData->x_angle);
		osPoolFree(poolTrackingData, pTrackingData);
	}
}
