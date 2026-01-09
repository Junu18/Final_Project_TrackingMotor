/*
 * Model_StopWatch.h
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_STOPWATCH_H_
#define AP_MODEL_MODEL_STOPWATCH_H_

#include <stdint.h>
#include "cmsis_os.h"

typedef enum {
	STOPWATCH_STOP, STOPWATCH_RUN, STOPWATCH_CLEAR
} stopWatchState_t;

typedef enum {
	EVENT_RUN_STOP, EVENT_CLEAR
} stopWatchEvent_t;

typedef struct {
	int hour;
	int min;
	int sec;
	int msec;
} stopWatch_t;

extern stopWatchState_t stopWatchState;
extern osMessageQId stopWatchEventMsgBox;
extern osMessageQId stopWatchDataMsgBox;
extern osPoolId poolStopWatchEvent;
extern osPoolId poolStopWatchData;

void Model_StopWatchInit();
void Model_SetStopWatchState(stopWatchState_t state);
stopWatchState_t Model_GetStopWatchState();

#endif /* AP_MODEL_MODEL_STOPWATCH_H_ */
