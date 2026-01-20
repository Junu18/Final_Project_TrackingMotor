/*
 * ControllerTracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "ControllerTracking.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ========================================
 * 설정 상수
 * ======================================== */
#define SCREEN_CENTER_X  320  // 화면 중심 X (640/2)
#define SCREEN_CENTER_Y  240  // 화면 중심 Y (480/2)

#define ANGLE_GAIN       0.1f // 각도 변환 게인 (조정 가능)

/* ========================================
 * 전역 Model 데이터
 * ======================================== */
Model_Tracking_t g_tracking_model;  // 전역 Model 인스턴스

/**
 * @brief Controller 초기화
 */
void Controller_Tracking_Init(void) {
    Model_Tracking_Init(&g_tracking_model);
    printf("[Controller] Tracking Controller Initialized\r\n");
}

/**
 * @brief Controller Tracking Execute (Reference Architecture)
 * @note Event 수신 → 데이터 처리 → 각도 계산 → Data 발행
 */
void Controller_Tracking_Excute(void) {
    extern osMessageQId trackingEventMsgBox;
    extern osMessageQId trackingDataMsgBox;
    extern osPoolId poolTrackingData;
    extern RxPacket_t g_rx_packet_tracking;  // ISR에서 저장한 수신 데이터
    extern uint8_t rx_buff[4];  // Raw bytes (디버그 출력용)
    extern volatile uint32_t g_isr_count;  // ISR 호출 횟수

    static uint32_t event_count = 0;  // Event 수신 횟수

    // Event 수신 (non-blocking, timeout=0)
    osEvent evt = osMessageGet(trackingEventMsgBox, 0);
    if (evt.status != osEventMessage) {
        return;  // Event 없으면 바로 리턴
    }

    event_count++;

    // ========================================
    // [중요] printf 출력 빈도 제한
    // 매번 출력하면 Queue가 가득 차서 멈춤!
    // 10000번에 한 번만 출력 (실시간 제어용)
    // ========================================
    bool should_print = (event_count % 10000 == 0);

    if (should_print) {
        printf("[Controller] Event #%lu (ISR count: %lu)\r\n",
               event_count, g_isr_count);
    }

    // Event 타입 확인
    uint16_t evtType = evt.value.v;
    if (evtType != EVENT_FPGA_DATA_RECEIVED) {
        return;
    }

    // ========================================
    // 1. 수신 데이터 처리 (Listener에서 이동한 로직)
    // ========================================
    g_tracking_model.rx_packet.raw = g_rx_packet_tracking.raw;

    // Raw bytes 출력 (100번에 한 번만)
    if (should_print) {
        printf("[SPI RX] Bytes:[0x%02X 0x%02X 0x%02X 0x%02X] Raw32=0x%08lX | ",
               rx_buff[0], rx_buff[1], rx_buff[2], rx_buff[3],
               g_tracking_model.rx_packet.raw);
    }

    // 헤더 검증
    if (!Model_Tracking_ValidateRx(&g_tracking_model)) {
        if (should_print) {
            printf("Header=0x%02X [INVALID!]\r\n", g_tracking_model.rx_packet.fields.header);
        }
        return;  // 무효한 데이터는 무시
    }

    // 유효한 데이터 → XY 좌표 업데이트
    uint16_t x = g_tracking_model.rx_packet.fields.x_pos;
    uint16_t y = g_tracking_model.rx_packet.fields.y_pos;
    Model_Tracking_UpdateXY(&g_tracking_model, x, y);

    if (should_print) {
        printf("Header=0x%02X | X=%4u | Y=%4u [OK]\r\n",
               g_tracking_model.rx_packet.fields.header, x, y);
    }

    // ========================================
    // 2. 각도 계산
    // ========================================
    Controller_Tracking_CalculateAngles(&g_tracking_model);

    // ========================================
    // 3. Data 발행 (Presenter에게)
    // ========================================
    Model_Tracking_t *pData = osPoolAlloc(poolTrackingData);
    if (pData != NULL) {
        memcpy(pData, &g_tracking_model, sizeof(Model_Tracking_t));
        osMessagePut(trackingDataMsgBox, (uint32_t)pData, 0);
    }
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
}
