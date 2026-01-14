/*
 * Presenter_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_PRESENTER_PRESENTER_TRACKING_H_
#define AP_PRESENTER_PRESENTER_TRACKING_H_

#include "cmsis_os.h"
#include "../Model/Model_Tracking.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "../../driver/lcd/lcd.h"
#include "../../driver/ServoMotor/ServoMotor.h"
#include "../../driver/SPI/SPI.h"
#include "i2c.h"
#include "tim.h"

void Presenter_Tracking_Init();
void Presenter_Tracking_Excute();
void Presenter_Tracking_UpdateState(tracking_t *pTrackingData);
void Presenter_Tracking_ManageServoPower(trackingState_t currState);
void Presenter_Tracking_DispLCD(tracking_t *pTrackingData);

#endif /* AP_PRESENTER_PRESENTER_TRACKING_H_ */
