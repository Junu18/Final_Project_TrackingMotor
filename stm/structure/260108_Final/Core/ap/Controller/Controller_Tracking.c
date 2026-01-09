/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

tracking_t trackingData;
static uint16_t currEvent;

void Controller_Tracking_Init() {
	Controller_Tracking_ResetData();
}

void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, osWaitForever);
	if (evt.status != osEventMessage)
		return;

	currEvent = evt.value.v;

	Controller_Tracking_UpdateState();
	Controller_Tracking_HandleSignal();
}

void Controller_Tracking_ResetData() {
	trackingData.x_pos = 0;
	trackingData.y_pos = 0;
	trackingData.x_angle = 90.0f;
	trackingData.y_angle = 90.0f;

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
	osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
}

void Controller_Tracking_HandleSignal() {
	trackingState_t state = Model_GetTrackingState();
	if (currEvent == EVENT_SERVO_TICK && state != TRACKING_IDLE) {
//        Controller_Tracking_RequestSPI();
	}

	if (currEvent == EVENT_SPI_CMPLT) {
//        Controller_Tracking_Calculate();
	}
}

void Controller_Tracking_UpdateState() {
	trackingState_t state = Model_GetTrackingState();

	switch (state) {
	case TRACKING_IDLE:
		Controller_Tracking_Idle();
		break;
	case TRACKING_SEARCH:
		Controller_Tracking_Search();
		break;
	case TRACKING_FOLLOW:
		Controller_Tracking_Follow();
		break;
	case TRACKING_LOST:
		Controller_Tracking_Lost();
		break;
	case TRACKING_AIMED:
		Controller_Tracking_Aimed();
		break;
	}
	trackingState_t currState = Model_GetTrackingState();
	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[50];
	if (state != currState) {
		sprintf(str, "\r\n[EVENT: %d] STATE CHANGE: %s -> %s\r\n", currEvent,
				stateNames[state], stateNames[currState]);
		HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 1000);
	}
}

void Controller_Tracking_Idle() {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search() {
	if (currEvent == EVENT_STOP) {
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow() {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);
	}
}

void Controller_Tracking_Lost() {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed() {
	if (currEvent == EVENT_CLEAR) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}
