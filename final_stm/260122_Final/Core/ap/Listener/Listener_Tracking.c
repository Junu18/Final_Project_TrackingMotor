/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Listener_Tracking.h"
#include "usart.h"
#include "stm32f4xx_hal.h"

hBtn hbtnToggle;

void Listener_Tracking_Init() {
	Button_Init(&hbtnToggle, BTN_TOGGLE_GPIO, BTN_TOGGLE_PIN);
	SPI_Init(&hspi1);
	SPI_RegisterCallback(Listener_OnComplete);
}

void Listener_Tracking_Excute() {
	Listener_Tracking_CheckButton();
}

void Listener_Tracking_CheckButton() {
	if (Button_GetState(&hbtnToggle) == ACT_PUSHED) {
		osMessagePut(trackingEventMsgBox, EVENT_TOGGLE, 0);
	}
}

void Listener_Tracking_TIM_ISR() {
	osMessagePut(trackingEventMsgBox, EVENT_SERVO_TICK, 0);
}

void Listener_OnComplete() {
	osMessagePut(trackingEventMsgBox, EVENT_FPGA_DATA_RECEIVED, 0);
}
