/*
 * button.h
 *
 *  Created on: Dec 16, 2025
 *      Author: rhoblack
 */

#ifndef DRIVER_BUTTON_BUTTON_H_
#define DRIVER_BUTTON_BUTTON_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

enum {
	PUSHED, RELEASED
};

typedef enum {
	NO_ACT, ACT_PUSHED, ACT_RELEASED
} button_state_t;

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint32_t pinNum;
	uint32_t prevState;
} hBtn;

void Button_Init(hBtn *btn, GPIO_TypeDef *GPIOx, uint32_t pinNum);
button_state_t Button_GetState(hBtn *btn);

#endif /* DRIVER_BUTTON_BUTTON_H_ */
