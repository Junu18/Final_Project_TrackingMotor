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
#include "../../driver/SPI/SPI.h"
#include "../Model/Model_Tracking.h"
#include "spi.h"

#define BTN_TOGGLE_GPIO				GPIOA
#define BTN_TOGGLE_PIN				GPIO_PIN_11

void Listener_Tracking_Init();
void Listener_Tracking_Excute();
void Listener_Tracking_CheckButton();
void Listener_Tracking_TIM_ISR();
void Listener_OnComplete();

#endif /* AP_LISTENER_LISTENER_TRACKING_H_ */
