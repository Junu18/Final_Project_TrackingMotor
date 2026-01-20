/*
 * Model_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_TRACKING_H_
#define AP_MODEL_MODEL_TRACKING_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

#define CENTER_X	320
#define CENTER_Y	240
#define CENTER_PAN	90.0f
#define CENTER_TILT	90.0f

typedef enum {
	TRACKING_IDLE, TRACKING_SEARCH, TRACKING_FOLLOW, TRACKING_LOST, TRACKING_AIMED
} trackingState_t;

typedef enum {
	EVENT_DEBUG,	// for debug
	EVENT_START,
	EVENT_STOP,
	EVENT_CLEAR,
	EVENT_SERVO_TICK,
	EVENT_SPI_CMPLT,
	EVENT_TARGET_ON,
	EVENT_TARGET_AIMED,
	EVENT_TARGET_LOST,
	EVENT_DEBUG_PAN_PLUS,
	EVENT_DEBUG_PAN_MINUS,
	EVENT_DEBUG_TILT_PLUS,
	EVENT_DEBUG_TILT_MINUS
} trackingEvent_t;

typedef struct {
	float angle_pan;
	float angle_tilt;
	int x_pos;
	int y_pos;
	bool isDetected;
	bool isAimed;
	bool motorEnable;
} tracking_t;

extern trackingState_t trackingState;
extern osMessageQId trackingEventMsgBox;
extern osMessageQId trackingDataMsgBox;
extern osPoolId poolTrackingEvent;
extern osPoolId poolTrackingData;

void Model_TrackingInit();
void Model_SetTrackingState(trackingState_t state);
trackingState_t Model_GetTrackingState();

#endif /* AP_MODEL_MODEL_TRACKING_H_ */
