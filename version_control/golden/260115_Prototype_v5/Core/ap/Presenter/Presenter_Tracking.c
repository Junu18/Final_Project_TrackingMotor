/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"

Servo_t hServoPan;
Servo_t hServoTilt;
Laser_t hLaser;
Buzzer_t hBuzzer;

void Presenter_Tracking_Init() {
	Laser_Init(&hLaser, LASER_GPIO, LASER_PIN);
	Buzzer_Init(&hBuzzer, &htim4, TIM_CHANNEL_1);
	Servo_Init(&hServoPan, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServoPan, CENTER_PAN);
	Servo_Disable(&hServoPan);
//	Servo_Enable(&hServoPan);

	Servo_Init(&hServoTilt, &htim3, TIM_CHANNEL_2);
	Servo_SetAngle(&hServoTilt, CENTER_TILT);
	Servo_Disable(&hServoTilt);
//	Servo_Enable(&hServoTilt);
}

void Presenter_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingDataMsgBox, osWaitForever);
	tracking_t *pTrackingData;
	if (evt.status != osEventMessage)
		return;

	pTrackingData = (tracking_t*) evt.value.p;
	Presenter_Tracking_UpdateState(pTrackingData);
	osPoolFree(poolTrackingData, pTrackingData);
}

void Presenter_Tracking_UpdateState(tracking_t *pTrackingData) {
	trackingState_t state = Model_GetTrackingState();

	Presenter_Tracking_ManageServoPower(state);
	if (state != TRACKING_IDLE) {
		Servo_SetAngle(&hServoPan, pTrackingData->angle_pan);
		Servo_SetAngle(&hServoTilt, pTrackingData->angle_tilt);
		Presenter_Tracking_SendData(pTrackingData, state);
		char buf[50];
		sprintf(buf, "x: %d\n", pTrackingData->x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 1000);
		sprintf(buf, "y: %d\n", pTrackingData->y_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 1000);
		if (state == TRACKING_AIMED) {
			Buzzer_PlayPattern(&hBuzzer, 500, NOTE_A5);
			Laser_On(&hLaser);
		} else {
			Buzzer_StopSound(&hBuzzer);
			Laser_Off(&hLaser);
		}

	}
}

void Presenter_Tracking_SendData(tracking_t *pTrackingData,
		trackingState_t currState) {
	TxPacket_t tx = { 0, };
	tx.fields.motor_pan_data = (uint32_t) pTrackingData->angle_pan;
	tx.fields.motor_pan_data = (uint32_t) (PAN_MAX - pTrackingData->angle_pan);
	tx.fields.motor_tilt_data =
			(uint32_t) (pTrackingData->angle_tilt - HALF_TILT);
//	tx.fields.motor_tilt_data = (uint32_t) pTrackingData->angle_tilt;
	tx.fields.stm_state = currState;
	SPI_SetTxData(tx.raw);
	SPI_StartTransfer_DMA();
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
