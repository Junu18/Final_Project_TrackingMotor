/*
 * ModelTracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "ModelTracking.h"
#include <string.h>

/**
 * @brief Model 초기화
 */
void Model_Tracking_Init(Model_Tracking_t* model) {
    memset(model, 0, sizeof(Model_Tracking_t));

    // 초기 상태: 화면 중앙 (640x480 기준)
    model->target_x = 320;
    model->target_y = 240;

    // 초기 각도: 정면 (0도)
    model->pan_angle = 0;
    model->tilt_angle = 0;

    // 타겟 미감지 상태
    model->target_detected = 0;
    model->laser_active = 0;
}

/**
 * @brief XY 좌표 업데이트 (Listener에서 호출)
 */
void Model_Tracking_UpdateXY(Model_Tracking_t* model, uint16_t x, uint16_t y) {
    model->target_x = x;
    model->target_y = y;
    model->target_detected = 1;  // 타겟 감지됨
    model->rx_count++;
}

/**
 * @brief 서보모터 각도 업데이트 (Controller에서 호출)
 */
void Model_Tracking_UpdateAngles(Model_Tracking_t* model, int16_t pan, int16_t tilt) {
    model->pan_angle = pan;
    model->tilt_angle = tilt;

    // 송신 패킷 구성 (추후 FPGA로 전송용)
    model->tx_packet.fields.header = 0xAA;
    model->tx_packet.fields.pan_angle = pan;
    model->tx_packet.fields.tilt_angle = tilt;
}

/**
 * @brief 수신 데이터 검증 (헤더 체크)
 */
uint8_t Model_Tracking_ValidateRx(Model_Tracking_t* model) {
    if (model->rx_packet.fields.header == 0x55) {
        return 1;  // 유효함
    } else {
        model->rx_error_count++;
        return 0;  // 무효함
    }
}


