/*
 * Presenter_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_PRESENTER_PRESENTER_TRACKING_H_
#define AP_PRESENTER_PRESENTER_TRACKING_H_

#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "tim.h"
#include "../Model/Model_Tracking.h"
#include "../../driver/lcd/lcd.h"
#include "../../driver/ServoMotor/ServoMotor.h"
#include "../../driver/SPI/SPI.h"
#include "../../driver/Laser/Laser.h"
#include "../../driver/Buzzer/buzzer.h"

#define LASER_GPIO	GPIOB
#define LASER_PIN	GPIO_PIN_10

void Presenter_Tracking_Init();
void Presenter_Tracking_Excute();
void Presenter_Tracking_UpdateState(tracking_t *pTrackingData);
void Presenter_Tracking_ManageServoPower(trackingState_t currState);
void Presenter_Tracking_SendData(tracking_t *pTrackingData,
		trackingState_t currState);
#endif /* AP_PRESENTER_PRESENTER_TRACKING_H_ */
