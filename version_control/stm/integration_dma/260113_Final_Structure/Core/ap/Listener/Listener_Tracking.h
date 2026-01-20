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

//#define BTN_START_GPIO				GPIOC
//#define BTN_START_PIN				GPIO_PIN_10
//#define BTN_STOP_GPIO				GPIOC
//#define BTN_STOP_PIN				GPIO_PIN_11
//#define BTN_CLEAR_GPIO				GPIOC
//#define BTN_CLEAR_PIN				GPIO_PIN_12

#define BTN_START_GPIO				GPIOB
#define BTN_START_PIN				GPIO_PIN_13
#define BTN_STOP_GPIO				GPIOB
#define BTN_STOP_PIN				GPIO_PIN_14
#define BTN_CLEAR_GPIO				GPIOB
#define BTN_CLEAR_PIN				GPIO_PIN_15

// 여기서 부터 일단 안씀
#define BTN_TARGET_ON_GPIO			GPIOC
#define BTN_TARGET_ON_PIN			GPIO_PIN_9
#define BTN_TARGET_LOST_GPIO		GPIOC
#define BTN_TARGET_LOST_PIN			GPIO_PIN_6
#define BTN_TARGET_AIMED_GPIO		GPIOC
#define BTN_TARGET_AIMED_PIN		GPIO_PIN_5
#define BTN_DEBUG_GPIO				GPIOA
#define BTN_DEBUG_PIN				GPIO_PIN_12
#define BTN_DEBUG_PAN_PLUS_GPIO		GPIOA
#define BTN_DEBUG_PAN_PLUS_PIN		GPIO_PIN_11
#define BTN_DEBUG_PAN_MINUS_GPIO	GPIOB
#define BTN_DEBUG_PAN_MINUS_PIN		GPIO_PIN_12

void Listener_Tracking_Init();
void Listener_Tracking_Excute();
void Listener_Tracking_CheckButton();
void Listener_Tracking_TIM_ISR();
void Listener_OnComplete();

#endif /* AP_LISTENER_LISTENER_TRACKING_H_ */
