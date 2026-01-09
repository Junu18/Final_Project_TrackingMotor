/*
 * ControllerTracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "ControllerTracking.h"
#include <stdio.h>

/* ========================================
 * 설정 상수
 * ======================================== */
#define SCREEN_CENTER_X  320  // 화면 중심 X (640/2)
#define SCREEN_CENTER_Y  240  // 화면 중심 Y (480/2)

#define ANGLE_GAIN       0.1f // 각도 변환 게인 (조정 가능)

/**
 * @brief Controller 초기화
 */
void Controller_Tracking_Init(void) {
    printf("[Controller] Tracking Controller Initialized\r\n");
}

/**
 * @brief Controller Tracking Execute (TaskNotify 대기 및 각도 계산)
 * @note freertos.c의 로직을 ap/ 레이어로 이동
 */
void Controller_Tracking_Excute(void) {
    extern osThreadId Presenter_TaskHandle;
    extern Model_Tracking_t g_tracking_model;

    // Listener로부터 알림 대기 (Blocking)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // 각도 계산
    Controller_Tracking_CalculateAngles(&g_tracking_model);

    // Presenter에게 알림
    xTaskNotifyGive(Presenter_TaskHandle);
}

/**
 * @brief XY 좌표를 서보모터 각도로 변환
 * @param model: Tracking Model 포인터
 *
 * @note 계산 로직:
 *  - 화면 중심으로부터 떨어진 거리를 각도로 변환
 *  - Pan (좌우): X 좌표 차이
 *  - Tilt (상하): Y 좌표 차이
 *  - 추후 PID 제어나 칼만 필터 적용 가능
 */
void Controller_Tracking_CalculateAngles(Model_Tracking_t* model) {
    // 현재 타겟 좌표
    int16_t target_x = model->target_x;
    int16_t target_y = model->target_y;

    // 화면 중심으로부터의 오차 계산
    int16_t error_x = target_x - SCREEN_CENTER_X;
    int16_t error_y = target_y - SCREEN_CENTER_Y;

    // ========================================
    // [TODO] 여기에 실제 각도 계산 로직 구현
    // ========================================
    // 현재는 간단한 비례 제어 (P 제어)
    // 추후 PID 제어, 칼만 필터, 또는 룩업 테이블 사용 가능

    int16_t pan_angle = (int16_t)(error_x * ANGLE_GAIN);
    int16_t tilt_angle = (int16_t)(error_y * ANGLE_GAIN);

    // 각도 제한 (-90 ~ +90도)
    if (pan_angle > 90) pan_angle = 90;
    if (pan_angle < -90) pan_angle = -90;
    if (tilt_angle > 90) tilt_angle = 90;
    if (tilt_angle < -90) tilt_angle = -90;

    // Model에 계산된 각도 저장
    Model_Tracking_UpdateAngles(model, pan_angle, tilt_angle);

    // 디버깅 출력 (선택적)
    // printf("[Controller] Pan:%d Tilt:%d\r\n", pan_angle, tilt_angle);
}


