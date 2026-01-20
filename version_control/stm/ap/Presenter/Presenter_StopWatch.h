/*
 * Presenter_StopWatch.h
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#ifndef AP_PRESENTER_PRESENTER_STOPWATCH_H_
#define AP_PRESENTER_PRESENTER_STOPWATCH_H_

#include "cmsis_os.h"
#include "../Model/Model_StopWatch.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "../../driver/lcd/lcd.h"
#include "i2c.h"

void Presenter_StopWatch_Init();
void Presenter_StopWatch_Excute();

#endif /* AP_PRESENTER_PRESENTER_STOPWATCH_H_ */
