/*
 * PresenterTracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "PresenterTracking.h"
#include "../../driver/lcd/lcd.h"
#include "i2c.h"
#include <stdio.h>

/**
 * @brief Presenter 초기화
 */
void Presenter_Tracking_Init(void) {
    // LCD 초기화
    LCD_Init(&hi2c1);
    LCD_WriteStringXY(0, 0, "Tracking Ready  ");
    LCD_WriteStringXY(1, 0, "Waiting Data... ");

    printf("[Presenter] Tracking Presenter Initialized\r\n");
}

/**
 * @brief Presenter Tracking Execute (Reference Architecture)
 * @note Data 수신 → 서보 업데이트 → LCD 업데이트 → Pool 해제
 */
void Presenter_Tracking_Excute(void) {
    extern osMessageQId trackingDataMsgBox;
    extern osPoolId poolTrackingData;

    // Data 수신 (non-blocking, timeout=0)
    osEvent evt = osMessageGet(trackingDataMsgBox, 0);
    if (evt.status != osEventMessage) {
        return;  // Data 없으면 바로 리턴
    }

    // Pool에서 할당된 Data 포인터 받기
    Model_Tracking_t *pData = (Model_Tracking_t *)evt.value.p;
    if (pData == NULL) {
        return;
    }

    // ========================================
    // 1. 서보모터 각도 업데이트
    // ========================================
    Presenter_Tracking_UpdateServo(pData);

    // ========================================
    // 2. LCD 화면 업데이트
    // ========================================
    Presenter_Tracking_UpdateLCD(pData);

    // ========================================
    // 3. UART 디버깅 출력
    // ========================================
    printf("[Tracking] X:%u Y:%u | Pan:%d Tilt:%d | RX:%lu ERR:%lu\r\n",
           pData->target_x,
           pData->target_y,
           pData->pan_angle,
           pData->tilt_angle,
           pData->rx_count,
           pData->rx_error_count);

    // ========================================
    // 4. Memory Pool 해제 (Reference Architecture)
    // ========================================
    osPoolFree(poolTrackingData, pData);
}

/**
 * @brief 서보모터 각도 업데이트
 * @param model: Tracking Model 포인터
 *
 * @note [TODO] 서보모터 드라이버 연동
 *  - 현재는 UART 출력만 수행
 *  - 추후 Subo_Motor.c의 함수 호출로 교체
 */
void Presenter_Tracking_UpdateServo(Model_Tracking_t* model) {
    // ========================================
    // [TODO] 서보모터 드라이버 함수 호출
    // ========================================
    // 예시:
    // Servo_SetAngle(SERVO_PAN, model->pan_angle);
    // Servo_SetAngle(SERVO_TILT, model->tilt_angle);

    // 현재는 디버깅 출력만
    printf("[Presenter] Servo -> Pan:%d Tilt:%d\r\n",
           model->pan_angle, model->tilt_angle);
}

/**
 * @brief LCD에 현재 상태 출력
 * @param model: Tracking Model 포인터
 */
void Presenter_Tracking_UpdateLCD(Model_Tracking_t* model) {
    char buff[17];

    // 1줄: 좌표 정보
    sprintf(buff, "X:%04u Y:%04u   ", model->target_x, model->target_y);
    LCD_WriteStringXY(0, 0, buff);

    // 2줄: 각도 정보
    sprintf(buff, "P:%+04d T:%+04d  ", model->pan_angle, model->tilt_angle);
    LCD_WriteStringXY(1, 0, buff);
}
