/*
 * Controller.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Controller.h"

void Controller_Init() {
	Controller_Tracking_Init();
}

void Controller_Excute() {
	Controller_Tracking_Excute();
}
