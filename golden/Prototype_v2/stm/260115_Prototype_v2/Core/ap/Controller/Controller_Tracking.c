/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"

tracking_t trackingData;

/**
 * @brief Initialize tracking controller and reset data
 */
void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}

/**
 * @brief Main execution loop (Scheduler)
 */
void Controller_Tracking_Excute() {
	// 1. Fetch event from message queue
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;
	trackingState_t prevState = Model_GetTrackingState();

	// 2. Data Parsing: Update trackingData only when not in IDLE
	if (currEvent == EVENT_FPGA_DATA_RECEIVED && prevState != TRACKING_IDLE) {
		Controller_Tracking_Unpack();
	}

	// 3. State Management: Directly transition based on trackingData flags
	Controller_Tracking_UpdateState(prevState, currEvent);

	// 4. Synchronization: Notify Presenter on state change or periodic tick
	trackingState_t currState = Model_GetTrackingState();
	Controller_Tracking_LogStateChange(currEvent, prevState, currState);

	bool isStateChanged = (prevState != currState);
	bool isPeriodicTick = (currEvent == EVENT_SERVO_TICK
			&& currState != TRACKING_IDLE);
	if (isStateChanged || isPeriodicTick) {
		Controller_Tracking_PushData();
	}
}

/**
 * @brief Routes the event to the appropriate state handler
 */
void Controller_Tracking_UpdateState(trackingState_t prevState,
		uint16_t currEvent) {
	switch (prevState) {
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
}

/* --- State Handlers --- */
void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		Controller_Tracking_ResetData();
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Controller_Tracking_ResetData();
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (trackingData.isDetected) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Controller_Tracking_ResetData();
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (!trackingData.isDetected) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (trackingData.isAimed) {
		Model_SetTrackingState(TRACKING_AIMED);
	} else if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		Controller_Tracking_ComputeServoAngle();
	}
}

void Controller_Tracking_Lost(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Controller_Tracking_ResetData();
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (trackingData.isDetected) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Controller_Tracking_ResetData();
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (!trackingData.isAimed) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	} else if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
		Controller_Tracking_ComputeServoAngle();
	}
}

/* --- Helper Functions --- */
void Controller_Tracking_LogStateChange(uint16_t currEvent,
		trackingState_t prevState, trackingState_t currState) {
	if (prevState == currState)
		return; // No change, no log

	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[100];
	sprintf(str, "\n[EVENT: %d] STATE CHANGE: %s -> %s\n", currEvent,
			stateNames[prevState], stateNames[currState]);
	HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 10);
}

void Controller_Tracking_Unpack() {
	RxPacket_t rx;
	rx.raw = SPI_GetRxData();

//	char debugStr[30];
//	sprintf(debugStr, "raw : %08lx\r\n", rx.raw);
//	HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);

	int x_pos = rx.fields.x_pos;
	int y_pos = rx.fields.y_pos;

	if (x_pos >= MAX_X || x_pos == MIN_X || y_pos >= MAX_Y || y_pos == MIN_Y)
		return;

	trackingData.x_pos = x_pos;
	trackingData.y_pos = y_pos;
	trackingData.isDetected = rx.fields.red_detect;
	trackingData.isAimed = rx.fields.target_on_box_fpga;
}

void Controller_Tracking_ResetData() {
	memset(&trackingData, 0, sizeof(tracking_t));
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
}

void Controller_Tracking_PushData() {
	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
	}
}

void Controller_Tracking_ComputeServoAngle() {
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// 1.  Apply Deadzone  (Protect from vibration)
	if (abs(diff_x) < 20)
		diff_x = 0;
	if (abs(diff_y) < 20)
		diff_y = 0;

	trackingData.angle_pan += (float) diff_x * GAIN_X;
	trackingData.angle_tilt += (float) diff_y * GAIN_Y;

	// 3. Clamping (Crucial for hardware protection)
	if (trackingData.angle_pan > PAN_MAX)
		trackingData.angle_pan = PAN_MAX;
	if (trackingData.angle_pan < PAN_MIN)
		trackingData.angle_pan = PAN_MIN;
	if (trackingData.angle_tilt > TILT_MAX)
		trackingData.angle_tilt = TILT_MAX;
	if (trackingData.angle_tilt < TILT_MIN)
		trackingData.angle_tilt = TILT_MIN;
}

void Controller_Tracking_Debug(trackingState_t state, trackingEvent_t event) {
	if (event == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
	}
	if (event == EVENT_DEBUG_PAN_PLUS) {
		if (trackingData.x_pos <= MAX_X - DEBUG_GAIN_X_POS) {
			trackingData.x_pos += DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}
	if (event == EVENT_DEBUG_PAN_MINUS) {
		if (trackingData.x_pos >= MIN_X + DEBUG_GAIN_X_POS) {
			trackingData.x_pos -= DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current PAN: %d\r\n", (int) trackingData.angle_pan);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
		sprintf(debugStr, "Current TILT: %d\r\n",
				(int) trackingData.angle_tilt);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}
}
