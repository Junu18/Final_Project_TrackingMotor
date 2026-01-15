/*
 * buzzer.h
 *
 *  Created on: Dec 28, 2025
 *      Author: kccistc
 */

#ifndef DRIVER_BUZZER_BUZZER_H_
#define DRIVER_BUZZER_BUZZER_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define GPIO_BUZZER		GPIOA
#define GPIO_PIN_BUZZER	GPIO_PIN_11

typedef struct {
	GPIO_TypeDef*	GPIOx;
	uint16_t		pinNum;
	uint32_t		endTime;
} hBuzzer;

extern hBuzzer buzzer;

void Buzzer_Init();
void Buzzer_Ring(int mSec);
void Buzzer_Update();

#endif /* DRIVER_BUZZER_BUZZER_H_ */
