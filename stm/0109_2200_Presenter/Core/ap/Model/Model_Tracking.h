/*
 * Model_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_TRACKING_H_
#define AP_MODEL_MODEL_TRACKING_H_

#include <stdint.h>
#include "cmsis_os.h"

typedef enum {
	TRACKING_0, TRACKING_90, TRACKING_180
} trackingState_t;

typedef enum {
	EVENT_0, EVENT_90, EVENT_180
} trackingEvent_t;

typedef struct {
	float x_angle;
	float y_angle;
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


//Event : 시스템의 상태를 바꾸는 명령
//Data(데이터) : 시스템이 처리해야할 수치임 (정확한 좌표값)(
//poolTrackingEvent : 공간 / 제어명령 전용 보관
//poolTrackingData : 공간(pool) 좌표값 전용 바구니 4개를 보관
// TrackingEventMsgBox : 통로(Queue) 제어명령 바구니가 지나다닐 전용 통로
// Tracking DataMsgBox : 통로(Queue), 좌표데이터 지나다닐 전용 통로
