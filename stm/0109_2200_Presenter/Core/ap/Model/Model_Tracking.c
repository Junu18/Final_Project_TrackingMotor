/*
 * Model_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Model_Tracking.h"

trackingState_t trackingState = TRACKING_90; // state shared memory
//전역상태변수

//메세지큐
//Task와 Task사이에 데이터를 주고받는 MailBox
osMessageQId trackingEventMsgBox;
//Que신호, 메일박스크기, Tracking Evnet(각도제어)
osMessageQDef(trackingEventQue, 4, trackingEvent_t);


//TrackingEvent_t는 편지규격임
osMessageQId trackingDataMsgBox; //Queue ID 설정

osMessageQDef(trackingDataQue, 4, tracking_t); //Queue 규격설정

// Dynamic memory allocation provided by RTOS
osPoolDef(poolTrackingEvent, 4, trackingEvent_t); //메모리 Pool정의
osPoolId poolTrackingEvent; //생성된 이벤트용 메모리 풀의 ID를 저장함
osPoolDef(poolTrackingData, 4, tracking_t); // 메모리 Pool규격설정
osPoolId poolTrackingData; //데이터 pool ID 선언

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

