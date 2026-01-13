/*
 * Listener.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Listener.h"
#include <stdio.h>

void Listener_Init() {
	printf("[DEBUG] Listener_Init() START\r\n");

	Listener_Tracking_Init();
	printf("[DEBUG] After Listener_Tracking_Init()\r\n");

	// SPI 인터럽트 수신 시작
	Listener_Tracking_StartReceive();
	printf("[DEBUG] After Listener_Tracking_StartReceive()\r\n");
}

void Listener_Excute() {
	Listener_Tracking_Excute();
}
