/*
 * Presenter.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Presenter.h"

// 전역 모델 참조
extern Model_Tracking_t g_tracking_model;

void Presenter_Init() {
    // Model 초기화
    Model_Tracking_Init(&g_tracking_model);

    // Tracking Presenter 초기화 (LCD 포함)
    Presenter_Tracking_Init();

    osDelay(500);

    printf("\r\n");
    printf("================================================\r\n");
    printf("  Drone Tracking System - Interrupt Mode\r\n");
    printf("================================================\r\n");
}

void Presenter_Excute() {
    // Tracking Execute 호출 (TaskNotify 대기 및 출력)
    Presenter_Tracking_Excute();
}
