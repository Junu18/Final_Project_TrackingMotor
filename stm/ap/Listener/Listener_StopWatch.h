/*
 * Listener_StopWatch.h
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#ifndef AP_LISTENER_LISTENER_STOPWATCH_H_
#define AP_LISTENER_LISTENER_STOPWATCH_H_

#include <stdint.h>
#include "cmsis_os.h"
#include "../../driver/button/button.h"
#include "../Model/Model_StopWatch.h"

#define BTN_RUN_STOP_GPIO	GPIOC
#define BTN_RUN_STOP_PIN	GPIO_PIN_11
#define BTN_CLEAR_GPIO		GPIOC
#define BTN_CLEAR_PIN		GPIO_PIN_12

void Listener_StopWatch_Init();
void Listener_StopWatch_Excute();
void Listener_StopWatch_CheckButton();

#endif /* AP_LISTENER_LISTENER_STOPWATCH_H_ */
