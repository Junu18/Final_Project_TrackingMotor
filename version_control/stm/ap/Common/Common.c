/*
 * Common.c
 *
 *  Created on: Jan 6, 2026
 *      Author: kccistc
 */

#include "Common.h"

void Common_StartTIMInterrupt() {
	HAL_TIM_Base_Start_IT(&htim1);
}

void Common_StopTIMInterrupt() {
	HAL_TIM_Base_Stop_IT(&htim1);
}
