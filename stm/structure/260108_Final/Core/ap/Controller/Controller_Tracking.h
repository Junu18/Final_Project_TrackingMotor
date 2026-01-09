/*
 * Controller_Traking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_CONTROLLER_CONTROLLER_TRACKING_H_
#define AP_CONTROLLER_CONTROLLER_TRACKING_H_

#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "../Model/Model_Tracking.h"
#include "../../driver/lcd/lcd.h"
#include "i2c.h"

void Controller_Tracking_Init();
void Controller_Tracking_Excute();
void Controller_Tracking_HandleSignal();
void Controller_Tracking_UpdateState();
void Controller_Tracking_Idle();
void Controller_Tracking_Search();
void Controller_Tracking_Follow();
void Controller_Tracking_Lost();
void Controller_Tracking_Aimed();

#endif /* AP_CONTROLLER_CONTROLLER_TRACKING_H_ */
