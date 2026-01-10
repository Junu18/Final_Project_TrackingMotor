/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"

Servo_t hServoPan;
Servo_t hServoTilt;

tracking_t *pTrackingData;

void Presenter_Tracking_Init() {
	LCD_Init(&hi2c1);
	Servo_Init(&hServoPan, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServoPan, CENTER_PAN);
	Servo_Disable(&hServoPan);

	Servo_Init(&hServoTilt, &htim3, TIM_CHANNEL_2);
	Servo_SetAngle(&hServoTilt, CENTER_TILT);
	Servo_Disable(&hServoTilt);
}

void Presenter_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingDataMsgBox, osWaitForever);

	if (evt.status != osEventMessage)
		return;

	pTrackingData = evt.value.p;
	Presenter_Tracking_UpdateState();
	Presenter_Tracking_DispLCD();
	osPoolFree(poolTrackingData, pTrackingData);
}

void Presenter_Tracking_UpdateState() {
	trackingState_t state = Model_GetTrackingState();

	Presenter_Tracking_ManageServoPower(state);
	Servo_SetAngle(&hServoPan, pTrackingData->angle_pan);
	Servo_SetAngle(&hServoTilt, pTrackingData->angle_tilt);
//	if (state != TRACKING_IDLE) {
//	}
}

void Presenter_Tracking_ManageServoPower(trackingState_t currState) {
	static trackingState_t prevState = TRACKING_IDLE;
	if (currState == prevState)
		return;

	if (currState == TRACKING_IDLE) {
		Servo_Disable(&hServoPan);
		Servo_Disable(&hServoTilt);
	} else if (prevState == TRACKING_IDLE) {
		Servo_Enable(&hServoPan);
		Servo_Enable(&hServoTilt);
	}
	prevState = currState;
}

void Presenter_Tracking_DispLCD() {
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
