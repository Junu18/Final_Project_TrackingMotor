/*
 * buzzer.h
 *
 *  Created on: Dec 28, 2025
 *      Author: kccistc
 */

#ifndef DRIVER_BUZZER_BUZZER_H_
#define DRIVER_BUZZER_BUZZER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint32_t prevTime;
    bool isSounding;
} Buzzer_t;

void Buzzer_Init(Buzzer_t *pBuzzer, TIM_HandleTypeDef *htim, uint32_t channel);
void Buzzer_MakeSound(Buzzer_t *pBuzzer, int herz);
void Buzzer_StartSound(Buzzer_t *pBuzzer);
void Buzzer_StopSound(Buzzer_t *pBuzzer);
void Buzzer_PlayPattern(Buzzer_t *pBuzzer, uint32_t ms, int herz);

#endif /* DRIVER_BUZZER_BUZZER_H_ */
