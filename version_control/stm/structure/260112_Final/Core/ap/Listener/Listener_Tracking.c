/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Listener_Tracking.h"
#include "stm32f4xx_hal.h"
#include "usart.h"

hBtn hbtnStart;
hBtn hbtnStop;
hBtn hbtnClear;
hBtn hbtnTargetOn;
hBtn hbtnTargetLost;
hBtn hbtnTargetAimed;
hBtn hbtnDebug;
hBtn hbtnDebugPanPlus;
hBtn hbtnDebugPanMinus;
hBtn hbtnDebugTiltPlus;
hBtn hbtnDebugTiltMinus;

void Listener_Tracking_Init() {
	Button_Init(&hbtnStart, BTN_START_GPIO, BTN_START_PIN);
	Button_Init(&hbtnStop, BTN_STOP_GPIO, BTN_STOP_PIN);
	Button_Init(&hbtnClear, BTN_CLEAR_GPIO, BTN_CLEAR_PIN);
	Button_Init(&hbtnTargetOn, BTN_TARGET_ON_GPIO, BTN_TARGET_ON_PIN);
	Button_Init(&hbtnTargetLost, BTN_TARGET_LOST_GPIO, BTN_TARGET_LOST_PIN);
	Button_Init(&hbtnTargetAimed, BTN_TARGET_AIMED_GPIO, BTN_TARGET_AIMED_PIN);
	Button_Init(&hbtnDebug, BTN_DEBUG_GPIO, BTN_DEBUG_PIN);
	Button_Init(&hbtnDebugPanPlus, BTN_DEBUG_PAN_PLUS_GPIO,
			BTN_DEBUG_PAN_PLUS_PIN);
	Button_Init(&hbtnDebugPanMinus, BTN_DEBUG_PAN_MINUS_GPIO,
			BTN_DEBUG_PAN_MINUS_PIN);
	Button_Init(&hbtnDebugTiltPlus, BTN_DEBUG_TILT_PLUS_GPIO,
			BTN_DEBUG_TILT_PLUS_PIN);
	Button_Init(&hbtnDebugTiltMinus, BTN_DEBUG_TILT_MINUS_GPIO,
			BTN_DEBUG_TILT_MINUS_PIN);
}

void Listener_Tracking_Excute() {
	Listener_Tracking_CheckButton();
}

void Listener_Tracking_CheckButton() {
	if (Button_GetState(&hbtnStart) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_START, 0);
	} else if (Button_GetState(&hbtnStop) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_STOP, 0);
	} else if (Button_GetState(&hbtnClear) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_CLEAR, 0);
	} else if (Button_GetState(&hbtnTargetOn) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
	} else if (Button_GetState(&hbtnTargetLost) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
	} else if (Button_GetState(&hbtnTargetAimed) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
	} else if (Button_GetState(&hbtnDebug) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG, 0);
	} else if (Button_GetState(&hbtnDebugPanPlus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_PAN_PLUS, 0);
	} else if (Button_GetState(&hbtnDebugPanMinus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_PAN_MINUS, 0);
	} else if (Button_GetState(&hbtnDebugTiltPlus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_TILT_PLUS, 0);
	} else if (Button_GetState(&hbtnDebugTiltMinus) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_DEBUG_TILT_MINUS, 0);
	}
}

void Listener_Tracking_TIM_ISR() {
	osMessagePut(trackingEventMsgBox, EVENT_SERVO_TICK, 0);
}
