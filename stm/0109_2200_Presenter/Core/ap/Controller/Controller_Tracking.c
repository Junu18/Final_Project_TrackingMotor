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
	case TRACKING_0:
		trackingData.x_angle = 0.0f;
		break;
	case TRACKING_90:
		trackingData.x_angle = 90.0f;
		break;
	case TRACKING_180:
		trackingData.x_angle = 180.0f;
		break;
	}
	Controller_Tracking_PutAngle();
}

void Controller_Tracking_PutAngle() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_0) {
			Model_SetTrackingState(TRACKING_0);
		} else if (evtState == EVENT_90) {
			Model_SetTrackingState(TRACKING_90);
		} else if (evtState == EVENT_180) {
			Model_SetTrackingState(TRACKING_180);
		}
	}

	static tracking_t prevTrackingData;

	if (!memcmp(&trackingData, &prevTrackingData, sizeof(tracking_t)))
		return;
	memcpy(&prevTrackingData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
	osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
}
