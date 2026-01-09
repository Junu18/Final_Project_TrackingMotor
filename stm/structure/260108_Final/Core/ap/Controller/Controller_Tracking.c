/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"

tracking_t trackingData;

void Controller_Tracking_Init() {
	trackingData.x_angle = 90.0f;

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
	osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
}

void Controller_Tracking_Excute() {
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
}

void Controller_Tracking_Idle() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_START) {
			Model_SetTrackingState(TRACKING_SEARCH);
		}
	}
}

void Controller_Tracking_Search() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_STOP) {
			Model_SetTrackingState(TRACKING_IDLE);
		} else if (evtState == EVENT_TARGET_ON) {
			Model_SetTrackingState(TRACKING_FOLLOW);
		}
	}
}

void Controller_Tracking_Follow() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_TARGET_LOST) {
			Model_SetTrackingState(TRACKING_IDLE);
		} else if (evtState == EVENT_TARGET_ON) {
			Model_SetTrackingState(TRACKING_FOLLOW);
		}
	}
}

void Controller_Tracking_Lost() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_TARGET_ON) {
			Model_SetTrackingState(TRACKING_FOLLOW);
		}
	}

}


void Controller_Tracking_Aimed() {

}
