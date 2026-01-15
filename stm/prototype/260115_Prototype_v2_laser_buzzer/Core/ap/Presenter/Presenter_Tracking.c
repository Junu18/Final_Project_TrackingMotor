/*
 * Presenter_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Presenter_Tracking.h"
#include "../driver/Laser/Laser.h"										// 추가

Servo_t hServoPan;
Servo_t hServoTilt;
hlaser  hLaser;											// 추가

void Presenter_Tracking_Init() {
	LCD_Init(&hi2c1);
	Laser_Init(&hLaser, GPIOA, GPIO_PIN_12);			 //  추가
	Servo_Init(&hServoPan, &htim3, TIM_CHANNEL_1);
	Servo_SetAngle(&hServoPan, CENTER_PAN);
	Servo_Disable(&hServoPan);

	Servo_Init(&hServoTilt, &htim3, TIM_CHANNEL_2);
	Servo_SetAngle(&hServoTilt, CENTER_TILT);
	Servo_Disable(&hServoTilt);
}

void Presenter_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingDataMsgBox, osWaitForever);
	tracking_t *pTrackingData;
	if (evt.status != osEventMessage)
		return;

	pTrackingData = (tracking_t*) evt.value.p;
	Presenter_Tracking_UpdateState(pTrackingData);

	static int freeCount = 0;
	if (osPoolFree(poolTrackingData, pTrackingData) == osOK) {
		freeCount++;
		if (freeCount % 10 == 0) {
			char buf[20];
			sprintf(buf, "Free OK: %d\r\n", freeCount);
			HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 10);
		}
	}
}

void Presenter_Tracking_UpdateState(tracking_t *pTrackingData) {
	trackingState_t state = Model_GetTrackingState();

	Presenter_Tracking_ManageServoPower(state);				// 여기서 Laser도 제어 IDLE 일 때는 Laser off 하게 하려고
	if (state != TRACKING_IDLE) {
		Servo_SetAngle(&hServoPan, pTrackingData->angle_pan);
		Servo_SetAngle(&hServoTilt, pTrackingData->angle_tilt);
		char buf[20];
		sprintf(buf, "x: %d\n", pTrackingData->x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 10);
		sprintf(buf, "y: %d\n", pTrackingData->y_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) buf, strlen(buf), 10);

		SPI_SetTxData(0xAAAA5555);
		SPI_StartTransfer_DMA();
	}
}

void Presenter_Tracking_ManageServoPower(trackingState_t currState) {
	static trackingState_t prevState = TRACKING_IDLE;
	if (currState == prevState)
		return;

	if (currState == TRACKING_IDLE) {
		Servo_Disable(&hServoPan);
		Servo_Disable(&hServoTilt);
		Laser_off(&hLaser);						 //츠기
	} else if (prevState == TRACKING_IDLE) {
		Servo_Enable(&hServoPan);
		Servo_Enable(&hServoTilt);
		Laser_on(&hLaser);					 //추가
	}
	prevState = currState;
}
