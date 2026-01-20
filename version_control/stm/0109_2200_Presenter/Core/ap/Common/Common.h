/*
 * Common.h
 *
 *  Created on: Jan 6, 2026
 *      Author: kccistc
 */

#ifndef AP_COMMON_COMMON_H_
#define AP_COMMON_COMMON_H_

#include "stm32f4xx_hal.h"
#include "tim.h"
#include "../Controller/Controller_StopWatch.h"

void Common_StartTIMInterrupt();
void Common_StopTIMInterrupt();

#endif /* AP_COMMON_COMMON_H_ */
