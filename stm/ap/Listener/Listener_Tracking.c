/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Listener_Tracking.h"

hBtn hbtn0;
hBtn hbtn90;
hBtn hbtn180;

void Listener_Tracking_Init() {
	Button_Init(&hbtn0, BTN_0_GPIO, BTN_0_PIN);
	Button_Init(&hbtn90, BTN_90_GPIO, BTN_90_PIN);
	Button_Init(&hbtn180, BTN_180_GPIO, BTN_180_PIN);
}

void Listener_Tracking_Excute() {
	Listener_Tracking_CheckButton();
}

void Listener_Tracking_CheckButton() {
	if (Button_GetState(&hbtn0) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_0, 0);
	} else if (Button_GetState(&hbtn90) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_90, 0);
	} else if (Button_GetState(&hbtn180) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_180, 0);
	}
}
