/*
 * buzzer.c
 *
 *  Created on: Dec 28, 2025
 *      Author: kccistc
 */

#include "buzzer.h"

void Buzzer_Init(Buzzer_t *pBuzzer, TIM_HandleTypeDef *htim, uint32_t channel) {
    pBuzzer->htim = htim;
    pBuzzer->channel = channel;
    pBuzzer->prevTime = 0;
    pBuzzer->isSounding = false;
}

void Buzzer_MakeSound(Buzzer_t *pBuzzer, int herz) {
    uint32_t arrValue = (uint32_t)(1000000 / herz);
    __HAL_TIM_SET_AUTORELOAD(pBuzzer->htim, arrValue - 1);
    __HAL_TIM_SET_COMPARE(pBuzzer->htim, pBuzzer->channel, (arrValue / 2 - 1));
}

void Buzzer_StartSound(Buzzer_t *pBuzzer) {
    HAL_TIM_PWM_Start(pBuzzer->htim, pBuzzer->channel);
}

void Buzzer_StopSound(Buzzer_t *pBuzzer) {
    HAL_TIM_PWM_Stop(pBuzzer->htim, pBuzzer->channel);
}

void Buzzer_PlayPattern(Buzzer_t *pBuzzer, uint32_t ms, int herz) {
    uint32_t currTime = HAL_GetTick();
    if (currTime - pBuzzer->prevTime >= ms) {
        pBuzzer->isSounding = !pBuzzer->isSounding;
        if (pBuzzer->isSounding) {
            Buzzer_MakeSound(pBuzzer, herz);
            Buzzer_StartSound(pBuzzer);
        } else {
            Buzzer_StopSound(pBuzzer);
        }
        pBuzzer->prevTime = currTime;
    }
}
