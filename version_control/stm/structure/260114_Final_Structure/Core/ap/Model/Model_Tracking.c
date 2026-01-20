/*
 * Model_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Model_Tracking.h"

trackingState_t trackingState = TRACKING_IDLE; // state shared memory

osMessageQId trackingEventMsgBox;
osMessageQDef(trackingEventQue, 16, trackingEvent_t);

osMessageQId trackingDataMsgBox;
osMessageQDef(trackingDataQue, 16, uint32_t);

// Dynamic memory allocation provided by RTOS
osPoolDef(poolTrackingEvent, 16, trackingEvent_t);
osPoolId poolTrackingEvent;
osPoolDef(poolTrackingData, 32, tracking_t);
osPoolId poolTrackingData;

void Model_TrackingInit() {
	poolTrackingEvent = osPoolCreate(osPool(poolTrackingEvent));
	poolTrackingData = osPoolCreate(osPool(poolTrackingData));
	trackingEventMsgBox = osMessageCreate(osMessageQ(trackingEventQue), NULL);
	trackingDataMsgBox = osMessageCreate(osMessageQ(trackingDataQue), NULL);
}

void Model_SetTrackingState(trackingState_t state) {
	trackingState = state;
}

trackingState_t Model_GetTrackingState() {
	return trackingState;
}
