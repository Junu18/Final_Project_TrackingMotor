/*
 * Listener_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_LISTENER_LISTENER_TRACKING_H_
#define AP_LISTENER_LISTENER_TRACKING_H_

#include <stdint.h>
#include "cmsis_os.h"
#include "../../driver/button/button.h"
#include "../Model/Model_Tracking.h"

#define BTN_0_GPIO		GPIOC
#define BTN_0_PIN		GPIO_PIN_10
#define BTN_90_GPIO		GPIOC
#define BTN_90_PIN		GPIO_PIN_11
#define BTN_180_GPIO	GPIOC
#define BTN_180_PIN		GPIO_PIN_12

void Listener_Tracking_Init();
void Listener_Tracking_Excute();
void Listener_Tracking_CheckButton();

#endif /* AP_LISTENER_LISTENER_TRACKING_H_ */
