/*
 * ModelTracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODELTRACKING_H_
#define AP_MODEL_MODELTRACKING_H_

#include "../Common/Common.h"
#include "cmsis_os.h"

/* ========================================
 * Event 타입 정의 (Listener → Controller)
 * ======================================== */
typedef enum {
    EVENT_FPGA_DATA_RECEIVED = 1,  // FPGA로부터 데이터 수신 완료
} trackingEvent_t;

/* ========================================
 * FPGA로부터 수신하는 32비트 패킷 구조
 * ======================================== */
typedef union {
    uint32_t raw;
    struct {
        uint32_t header : 8;  // 0x55 (검증용 헤더)
        uint32_t x_pos  : 12; // X 좌표 (0~4095)
        uint32_t y_pos  : 12; // Y 좌표 (0~4095)
    } fields;
} RxPacket_t;

/* ========================================
 * STM32에서 FPGA로 송신하는 32비트 패킷 구조
 * (추후 모터 각도 전송용)
 * ======================================== */
typedef union {
    uint32_t raw;
    struct {
        uint32_t header     : 8;  // 0xAA (STM 송신 헤더)
        int32_t  pan_angle  : 12; // Pan 각도 (-2048~2047)
        int32_t  tilt_angle : 12; // Tilt 각도 (-2048~2047)
    } fields;
} TxPacket_t;

/* ========================================
 * Tracking Model 구조체
 * ======================================== */
typedef struct {
    // 현재 타겟 좌표 (FPGA로부터 수신)
    uint16_t target_x;
    uint16_t target_y;

    // 계산된 서보모터 각도 (Controller에서 계산)
    int16_t pan_angle;
    int16_t tilt_angle;

    // 수신 패킷 (원본 데이터)
    RxPacket_t rx_packet;

    // 송신 패킷 (FPGA로 보낼 각도 데이터)
    TxPacket_t tx_packet;

    // 통계
    uint32_t rx_count;        // 수신 성공 횟수
    uint32_t rx_error_count;  // 수신 실패 횟수 (헤더 오류)

    // 상태 플래그
    uint8_t target_detected;  // 타겟 감지 여부 (1=감지, 0=미감지)
    uint8_t laser_active;     // 레이저 활성화 여부 (1=ON, 0=OFF)

} Model_Tracking_t;

/* ========================================
 * FreeRTOS Queue & Pool (Reference Architecture)
 * ======================================== */
// Event Queue: Listener → Controller (Event만 전달)
extern osMessageQId trackingEventMsgBox;
// Data Queue: Controller → Presenter (계산된 데이터 전달)
extern osMessageQId trackingDataMsgBox;
// Memory Pool: 동적 메모리 할당
extern osPoolId poolTrackingData;

/* ========================================
 * 함수 프로토타입
 * ======================================== */
void Model_Tracking_QueueInit(void);  // Queue/Pool 초기화 (freertos.c에서 호출)
void Model_Tracking_Init(Model_Tracking_t* model);
void Model_Tracking_UpdateXY(Model_Tracking_t* model, uint16_t x, uint16_t y);
void Model_Tracking_UpdateAngles(Model_Tracking_t* model, int16_t pan, int16_t tilt);
uint8_t Model_Tracking_ValidateRx(Model_Tracking_t* model);

#endif /* AP_MODEL_MODELTRACKING_H_ */
