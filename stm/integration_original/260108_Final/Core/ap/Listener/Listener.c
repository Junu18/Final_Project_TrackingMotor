/*
 * Listener.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Listener.h"

void Listener_Init() {
	Listener_Tracking_Init();

	// SPI 인터럽트 수신 시작
	Listener_Tracking_StartReceive();
}

void Listener_Excute() {
	Listener_Tracking_Excute();
}
