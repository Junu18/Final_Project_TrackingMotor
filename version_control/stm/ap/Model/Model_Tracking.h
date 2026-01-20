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
 * @note 각종 상수 정의 / Controller/Presenter에서 사용
 */
#define CENTER_X	320
#define CENTER_Y	240
#define CENTER_PAN	90.0f
#define CENTER_TILT	90.0f

/**
 * @brief tracking state 타입 정의
 * @note trackingState_t 구조체는 Controller/Presenter에서 사용
 */
typedef enum {
	TRACKING_IDLE, TRACKING_SEARCH, TRACKING_FOLLOW, TRACKING_LOST, TRACKING_AIMED
} trackingState_t;

/**
 * @brief tracking event 타입 정의 (Listener → Controller)
 * @note trackingEvent_t 구조체는 Controller/Listener에서 사용
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
 * @brief FPGA로부터 수신하는 32비트 패킷 구조
 * @note RxPacket_t 구조체는 Listener/Controller에서 사용
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
 * @brief STM32에서 FPGA로 송신하는 32비트 패킷 구조
 * @note TxPacket_t 구조체는 Controller/Listener에서 사용
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
 * @note tracking_t 구조체는 Controller/Presenter에서 사용
 */
typedef struct {
	/// @brief 현재 타겟 좌표
	uint16_t  x_pos;
	uint16_t  y_pos;

	/// @brief 서보모터 각도
	float angle_pan;
	float angle_tilt;

	/// @brief 상태 플래그
	bool is_Detected;
	bool is_Aimed;

	/// @brief 패킷 정보
    RxPacket_t rx_packet;
    TxPacket_t tx_packet;

	/// @brief 통계 정보 (디버깅)
	uint32_t rx_count;
    uint32_t rx_error_count;
} tracking_t;

/**
 * @brief 공유 변수 선언
 * @note Model_Tracking.c에서 정의
 */
extern trackingState_t trackingState;
extern osMessageQId trackingEventMsgBox;
extern osMessageQId trackingDataMsgBox;
extern osPoolId poolTrackingEvent;
extern osPoolId poolTrackingData;

/**
 * @brief Model 함수 선언
 * @note Model_Tracking.c에서 정의
 */
