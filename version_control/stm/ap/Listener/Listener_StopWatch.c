/*
 * Listener_StopWatch.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Listener_StopWatch.h"

hBtn hbtnRunStop;
hBtn hbtnClear;

void Listener_StopWatch_Init() {
	Button_Init(&hbtnRunStop, BTN_RUN_STOP_GPIO, BTN_RUN_STOP_PIN);
	Button_Init(&hbtnClear, BTN_CLEAR_GPIO, BTN_CLEAR_PIN);
}

void Listener_StopWatch_Excute() {
	Listener_StopWatch_CheckButton();
}

void Listener_StopWatch_CheckButton() {
	if (Button_GetState(&hbtnRunStop) == ACT_PUSHED) {
		osMessagePut(stopWatchEventMsgBox, EVENT_RUN_STOP, 0);
	} else if (Button_GetState(&hbtnClear) == ACT_PUSHED) {
		osMessagePut(stopWatchEventMsgBox, EVENT_CLEAR, 0);
	}
}
