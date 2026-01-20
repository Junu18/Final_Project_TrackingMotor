/*
 * Controller_StopWatch.h
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#ifndef AP_CONTROLLER_CONTROLLER_STOPWATCH_H_
#define AP_CONTROLLER_CONTROLLER_STOPWATCH_H_

#include <stdint.h>
#include "cmsis_os.h"
#include <string.h>
#include "../Model/Model_StopWatch.h"
#include "usart.h"
#include "../Common/Common.h"

void Controller_StopWatch_Init();
void Controller_StopWatch_Excute();
void Controller_StopWatch_Stop();
void Controller_StopWatch_Run();
void Controller_StopWatch_Clear();
void Controller_StopWatch_IncTimeCallBack();

#endif /* AP_CONTROLLER_CONTROLLER_STOPWATCH_H_ */
