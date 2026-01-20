/*
 * Model_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Model_Tracking.h"

trackingState_t trackingState = TRACKING_IDLE;  // 초기 상태: IDLE

osMessageQId trackingEventMsgBox;
osMessageQDef(trackingEventQue, 16, uint32_t);  // 이벤트 큐: 크기 16

osMessageQId trackingDataMsgBox;
osMessageQDef(trackingDataQue, 8, uint32_t);    // 데이터 큐: 크기 8
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
