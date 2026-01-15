/*
 * buzzer.c
 *
 *  Created on: Dec 28, 2025
 *      Author: kccistc
 */

#include "buzzer.h"

hBuzzer buzzer;

void Buzzer_Init() {
	buzzer.GPIOx = GPIO_BUZZER;
	buzzer.pinNum = GPIO_PIN_BUZZER;
}

void Buzzer_Ring(int ms) {
    HAL_GPIO_WritePin(GPIO_BUZZER, GPIO_PIN_BUZZER, GPIO_PIN_SET);
    buzzer.endTime = HAL_GetTick() + ms;
}

void Buzzer_Update() {
    if (buzzer.endTime != 0 && HAL_GetTick() >= buzzer.endTime) {
        HAL_GPIO_WritePin(GPIO_BUZZER, GPIO_PIN_BUZZER, GPIO_PIN_RESET);
        buzzer.endTime = 0;
    }
}
