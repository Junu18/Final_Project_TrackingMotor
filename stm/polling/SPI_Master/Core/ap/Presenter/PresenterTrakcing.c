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
 * @brief Presenter Tracking Execute (TaskNotify 대기 및 출력)
 * @note freertos.c의 로직을 ap/ 레이어로 이동
 */
void Presenter_Tracking_Excute(void) {
    extern Model_Tracking_t g_tracking_model;

    // Controller로부터 알림 대기 (Blocking)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // 서보모터 각도 업데이트
    Presenter_Tracking_UpdateServo(&g_tracking_model);

    // LCD 화면 업데이트
    Presenter_Tracking_UpdateLCD(&g_tracking_model);

    // UART 디버깅 출력
    printf("[Tracking] X:%u Y:%u | Pan:%d Tilt:%d | RX:%lu ERR:%lu\r\n",
           g_tracking_model.target_x,
           g_tracking_model.target_y,
           g_tracking_model.pan_angle,
           g_tracking_model.tilt_angle,
           g_tracking_model.rx_count,
           g_tracking_model.rx_error_count);
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


