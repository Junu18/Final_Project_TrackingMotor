/*
 * Model_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_TRACKING_H_
#define AP_MODEL_MODEL_TRACKING_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "../Common/Common.h"

/**
 * @brief define 상수
 */
#define CENTER_X	320
#define CENTER_Y	240
#define CENTER_PAN	90.0f
#define CENTER_TILT	90.0f

/**
 * @brief tracking state 타입 정의
 */
typedef enum {
	TRACKING_IDLE, TRACKING_SEARCH, TRACKING_FOLLOW, TRACKING_LOST, TRACKING_AIMED
} trackingState_t;

/**
 * @brief tracking event 타입 정의
 */
typedef enum {
	EVENT_DEBUG,
	EVENT_FPGA_DATA_RECEIVED,
	EVENT_START,
	EVENT_STOP,
	EVENT_CLEAR,
	EVENT_SERVO_TICK,
	EVENT_TARGET_ON,
	EVENT_TARGET_AIMED,
	EVENT_TARGET_LOST,
	EVENT_DEBUG_PAN_PLUS,
	EVENT_DEBUG_PAN_MINUS
} trackingEvent_t;

/**
 * @brief FPGA로부터 수신하는 32비트 패킷
 */
typedef union {
    uint32_t raw;
    struct {
        uint32_t header : 8;
        uint32_t x_pos  : 12;
        uint32_t y_pos  : 12;
    } fields;
} RxPacket_t;

/**
 * @brief STM32에서 FPGA로 송신하는 32비트 패킷
 */
typedef union {
    uint32_t raw;
    struct {
        uint32_t header     : 8;
        int32_t  angle_pan  : 12;
        int32_t  angle_tilt : 12;
    } fields;
} TxPacket_t;

/**
 * @brief tracking data 구조체
 */
typedef struct {
	uint16_t  x_pos;
	uint16_t  y_pos;
	float angle_pan;
	float angle_tilt;
	bool is_Detected;
	bool is_Aimed;
    RxPacket_t rx_packet;
    TxPacket_t tx_packet;
	uint32_t rx_count;
    uint32_t rx_error_count;
} tracking_t;

extern trackingState_t trackingState;
extern osMessageQId trackingEventMsgBox;
extern osMessageQId trackingDataMsgBox;
extern osPoolId poolTrackingEvent;
extern osPoolId poolTrackingData;

void Model_TrackingInit();
void Model_SetTrackingState(trackingState_t state);
trackingState_t Model_GetTrackingState();

#endif /* AP_MODEL_MODEL_TRACKING_H_ */
