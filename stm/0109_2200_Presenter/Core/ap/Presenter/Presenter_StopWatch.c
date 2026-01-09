/*
 * Presenter_StopWatch.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Presenter_StopWatch.h"

void Presenter_StopWatch_Init() {
	LCD_Init(&hi2c1);
}

void Presenter_StopWatch_Excute() {
	stopWatch_t *pStopWatchData;
	osEvent evt;
	evt = osMessageGet(stopWatchDataMsgBox, 0);

	if (evt.status == osEventMessage) {
		pStopWatchData = evt.value.p;
		char str[50];
		sprintf(str, "%02d:%02d:%02d:%01d\n", pStopWatchData->hour,
				pStopWatchData->min, pStopWatchData->sec, pStopWatchData->msec);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, sizeof(str), 1000);
		sprintf(str, "%02d:%02d:%02d:%01d", pStopWatchData->hour,
				pStopWatchData->min, pStopWatchData->sec, pStopWatchData->msec);
		LCD_WriteStringXY(1, 0, str);
		osPoolFree(poolStopWatchData, pStopWatchData);
	}
}
