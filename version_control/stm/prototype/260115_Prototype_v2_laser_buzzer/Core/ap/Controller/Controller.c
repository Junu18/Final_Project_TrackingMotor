/*
 * Controller.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Controller.h"

void Controller_Init() {
	Buzzer_Init();						// 추가
	Controller_Tracking_Init();
}

void Controller_Excute() {
	Buzzer_Update();					// 추가
	Controller_Tracking_Excute();
}
