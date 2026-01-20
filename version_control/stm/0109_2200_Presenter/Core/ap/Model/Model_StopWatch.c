/*
 * Model_StopWatch.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Model_StopWatch.h"

stopWatchState_t stopWatchState = STOPWATCH_STOP; // state shared memory

osMessageQId stopWatchEventMsgBox;
osMessageQDef(stopWatchEventQue, 4, stopWatchEvent_t);

osMessageQId stopWatchDataMsgBox;
osMessageQDef(stopWatchDataQue, 4, stopWatch_t);

// Dynamic memory allocation provided by RTOS
osPoolDef(poolStopWatchEvent, 4, stopWatchEvent_t);
osPoolId poolStopWatchEvent;
osPoolDef(poolStopWatchData, 32, stopWatch_t); //
osPoolId poolStopWatchData;

void Model_StopWatchInit() {
	poolStopWatchEvent = osPoolCreate(osPool(poolStopWatchEvent));
	poolStopWatchData = osPoolCreate(osPool(poolStopWatchData));
	stopWatchEventMsgBox = osMessageCreate(osMessageQ(stopWatchEventQue), NULL);
	stopWatchDataMsgBox = osMessageCreate(osMessageQ(stopWatchDataQue), NULL);
}

void Model_SetStopWatchState(stopWatchState_t state) {
	stopWatchState = state;
}

stopWatchState_t Model_GetStopWatchState() {
	return stopWatchState;
}
