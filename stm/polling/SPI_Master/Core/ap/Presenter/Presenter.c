/*
 * Presenter.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

#include "Presenter.h"
#include "Presenter_SPITest.h"

// 전역 모델 참조
extern Model_SPITest_t g_spiModel;

void Presenter_Init() {
    // 1. 제공해주신 lcd.c의 함수명과 매개변수(&hi2c1)에 맞춰 초기화
    // hi2c1은 i2c.h에 선언되어 있어야 합니다.
    LCD_Init(&hi2c1);

    // 2. 다른 모드(Stopwatch, Distance 등)의 Init 함수는 파일이 없으므로 삭제
    // Presenter_StopWatch_Init(); <- 삭제
    // Presenter_Distance_Init();  <- 삭제

    LCD_WriteStringXY(0, 0, "SPI TEST START  ");
}

void Presenter_Excute() {
    // 1. 데이터 동기화 (Listener의 데이터를 Model로 복사 및 검증)
    Presenter_SPITest_SyncData(&g_spiModel);

    // 2. 단일 모드이므로 Presenter_DispMode() 같은 복잡한 분기문 없이 유지하거나 삭제
}
