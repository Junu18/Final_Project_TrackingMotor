/*
 * Controller_StopWatch.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Controller_StopWatch.h"

stopWatch_t stopWatchData;

void Controller_StopWatch_Init() {
	Common_StartTIMInterrupt();
	stopWatchData.hour = 0;
	stopWatchData.min = 0;
	stopWatchData.sec = 0;
	stopWatchData.msec = 0;

	stopWatch_t *pStopWatchData = osPoolAlloc(poolStopWatchData);
	memcpy(pStopWatchData, &stopWatchData, sizeof(stopWatch_t));
	osMessagePut(stopWatchDataMsgBox, (uint32_t) pStopWatchData, 0);
}

void Controller_StopWatch_Excute() {
	stopWatchState_t state = Model_GetStopWatchState();
	switch (state) {
	case STOPWATCH_STOP:
		Controller_StopWatch_Stop();
		break;
	case STOPWATCH_RUN:
		Controller_StopWatch_Run();
		break;
	case STOPWATCH_CLEAR:
		Controller_StopWatch_Clear();
		break;
	}
}

void Controller_StopWatch_Stop() {
	osEvent evt = osMessageGet(stopWatchEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_RUN_STOP) {
//			HAL_UART_Transmit(&huart2, (uint8_t *)"RUN_STOP\n", 9, 1000);
			Model_SetStopWatchState(STOPWATCH_RUN);
		} else if (evtState == EVENT_CLEAR) {
//			HAL_UART_Transmit(&huart2, (uint8_t *)"CLEAR\n", 6, 1000);
			Model_SetStopWatchState(STOPWATCH_CLEAR);
		}
	}
}

void Controller_StopWatch_Run() {
	osEvent evt = osMessageGet(stopWatchEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;

		if (evtState == EVENT_RUN_STOP) {
//			HAL_UART_Transmit(&huart2, (uint8_t *)"RUN_STOP\n", 9, 1000);
			Model_SetStopWatchState(STOPWATCH_STOP);
		}
	}

	static stopWatch_t prevStopWatchData;

	if (!memcmp(&stopWatchData, &prevStopWatchData, sizeof(stopWatch_t)))
		return;
	memcpy(&prevStopWatchData, &stopWatchData, sizeof(stopWatch_t));

	stopWatch_t *pStopWatchData = osPoolAlloc(poolStopWatchData);
//	pStopWatchData->hour = stopWatchData.hour;
//	pStopWatchData->min = stopWatchData.min;
//	pStopWatchData->sec = stopWatchData.sec;
//	pStopWatchData->msec = stopWatchData.msec;
	memcpy(pStopWatchData, &stopWatchData, sizeof(stopWatch_t));
	osMessagePut(stopWatchDataMsgBox, (uint32_t) pStopWatchData, 0);
}

void Controller_StopWatch_Clear() {
	stopWatchData.hour = 0;
	stopWatchData.min = 0;
	stopWatchData.sec = 0;
	stopWatchData.msec = 0;

	stopWatch_t *pStopWatchData = osPoolAlloc(poolStopWatchData);
	memcpy(pStopWatchData, &stopWatchData, sizeof(stopWatch_t));
	osMessagePut(stopWatchDataMsgBox, (uint32_t) pStopWatchData, 0);

	Model_SetStopWatchState(STOPWATCH_STOP);
}

void Controller_StopWatch_IncTimeCallBack() {
	if (Model_GetStopWatchState() != STOPWATCH_RUN)
		return;

	static int msec = 0;
	if (msec != 100 - 1) {
		msec++;
		return;
	}
	msec = 0;

	if (stopWatchData.msec != 10 - 1) {
		stopWatchData.msec++;
		return;
	}
	stopWatchData.msec = 0;

	if (stopWatchData.sec != 60 - 1) {
		stopWatchData.sec++;
		return;
	}
	stopWatchData.sec = 0;

	if (stopWatchData.min != 60 - 1) {
		stopWatchData.min++;
		return;
	}
	stopWatchData.min = 0;

	if (stopWatchData.hour != 24 - 1) {
		stopWatchData.hour++;
		return;
	}
	stopWatchData.hour = 0;
}
