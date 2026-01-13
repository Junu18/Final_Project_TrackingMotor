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
#include <stdlib.h>
#include "../driver/SPI/SPI.h"

#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10

tracking_t trackingData;

void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}

void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;

	Controller_Tracking_UpdateState(currEvent);
	Controller_Tracking_HandleSignal(currEvent);
}

void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();
	static trackingState_t prevState = TRACKING_IDLE;

	if (state != prevState) {
		Controller_Tracking_PushData();
		prevState = state;
	}

	if (currEvent == EVENT_SERVO_TICK) {
		if (state != TRACKING_IDLE) {
			Controller_Tracking_PushData();
		}
	}

	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		SPI_GetRxBuffer();
	}

	if (currEvent == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
//		HAL_UART_Transmit(&huart2, (uint8_t*) "compute\n", 8, 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_PLUS) {
		if (trackingData.x_pos <= MAX_X - DEBUG_GAIN_X_POS) {
			trackingData.x_pos += DEBUG_GAIN_X_POS;
		}
//		char debugStr[30];
//		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
//		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_MINUS) {
		if (trackingData.x_pos >= MIN_X + DEBUG_GAIN_X_POS) {
			trackingData.x_pos -= DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);

	}

}

void Controller_Tracking_UpdateState(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();

	switch (state) {
	case TRACKING_IDLE:
		Controller_Tracking_Idle(currEvent);
		break;
	case TRACKING_SEARCH:
		Controller_Tracking_Search(currEvent);
		break;
	case TRACKING_FOLLOW:
		Controller_Tracking_Follow(currEvent);
		break;
	case TRACKING_LOST:
		Controller_Tracking_Lost(currEvent);
		break;
	case TRACKING_AIMED:
		Controller_Tracking_Aimed(currEvent);
		break;
	}
	trackingState_t currState = Model_GetTrackingState();
	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[50];
	if (state != currState) {
		sprintf(str, "\r\n[EVENT: %d] STATE CHANGE: %s -> %s\r\n", currEvent,
				stateNames[state], stateNames[currState]);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 10);
	}
}

void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);
	}
}

void Controller_Tracking_Lost(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed(uint16_t currEvent) {
	if (currEvent == EVENT_CLEAR) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_ResetData() {
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
	trackingData.isDetected = false;
	trackingData.isAimed = false;
}

void Controller_Tracking_ComputeServoAngle() {
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// Apply Deadzone  (Protect from vibration)
	if (abs(diff_x) < 5)
		diff_x = 0;
	if (abs(diff_y) < 5)
		diff_y = 0;

	trackingData.angle_pan += (float) diff_x * GAIN_X;
	trackingData.angle_tilt += (float) diff_y * GAIN_Y;
}

void Controller_Tracking_PushData() {
	static tracking_t prevData;

//	if (!memcmp(&trackingData, &prevData, sizeof(tracking_t)))
//		return;
	memcpy(&prevData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
	}
}

/*
void Controller_Tracking_Unpack() {
   // SPI ISR에서 저장한 FPGA 패킷 참조
   extern RxPacket_t g_rx_packet_tracking;

   // 패킷에서 헤더 검증 (0x55)
   if (g_rx_packet_tracking.fields.header != 0x55) {
      trackingData.rx_error_count++;
      return;
   }

   // 좌표 추출
   trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
   trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
   trackingData.rx_packet.raw = g_rx_packet_tracking.raw;
   trackingData.rx_count++;
   trackingData.is_Detected = true;
}
*/
