/*
 * Controller_Traking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_CONTROLLER_CONTROLLER_TRACKING_H_
#define AP_CONTROLLER_CONTROLLER_TRACKING_H_

#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "../Common/Common.h"
#include "../Model/Model_Tracking.h"
#include "../driver/SPI/SPI.h"
#include <stdlib.h>
/*uart*/
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdio.h>

#define GAIN_X				0.005f
#define GAIN_Y				0.005f
#define MIN_X				0
#define MIN_Y				0
#define MAX_X				640 - 1
#define MAX_Y				320 - 1
#define PAN_MIN				0.0f
#define PAN_MAX				180.0f
#define TILT_MIN			0.0f
#define TILT_MAX			90.0f
#define AIM_TOLERANCE 		30
#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10

typedef union {
	uint32_t raw;
	struct {
		uint32_t header :8;  // Bits 24~31 : 검증용 헤더 (0x55)
		uint32_t reserved :2;  // Bits 0~1 : 남는 공간 (Padding)
		uint32_t laser_fire_flag :1;  // Bit 2
		uint32_t target_on_box_fpga :1;  // Bit 3
		uint32_t red_detect :1;  // Bit 4
		uint32_t y_pos :9;  // Bits 5~13 : Y좌표 (9bit)
		uint32_t x_pos :10; // Bits 14~23 : X좌표 (10bit)
	} fields;
} RxPacket_t;

typedef union {
	uint32_t raw;
	struct {
		// [LSB: 0번 비트부터 쌓아 올림]
		uint32_t reserved :13;
		uint32_t laser_fire_complete :1;
		uint32_t stm_state :3;
		// 4. 상단 (Bits 16~23): Tilt 각도 (8비트)
		// (FPGA 이미지상 23-16번 비트)
		uint32_t tilt_angle :7;
		// 5. 최상단 (Bits 24~31): Pan 각도 (8비트)
		// (FPGA 이미지상 31-24번 비트)
		uint32_t pan_angle :8;
	} fields;
} TxPacket_t;

/* --- External Interface --- */
void Controller_Tracking_Init();
void Controller_Tracking_Excute();

/* --- Internal Logic Functions --- */
void Controller_Tracking_Unpack();
void Controller_Tracking_ResetData();
void Controller_Tracking_PushData();
void Controller_Tracking_ComputeServoAngle();
void Controller_Tracking_UpdateState(trackingState_t prevState, uint16_t currEvent);
void Controller_Tracking_LogStateChange(uint16_t currEvent,
		trackingState_t prevState, trackingState_t currState);
void Controller_Tracking_Debug(trackingState_t state, trackingEvent_t event);

/* --- State Handler Function --- */
void Controller_Tracking_Idle(uint16_t currEvent);
void Controller_Tracking_Search(uint16_t currEvent);
void Controller_Tracking_Follow(uint16_t currEvent);
void Controller_Tracking_Lost(uint16_t currEvent);
void Controller_Tracking_Aimed(uint16_t currEvent);

///* ========================================
// * STM32에서 FPGA로 송신하는 32비트 패킷 구조
// * (추후 모터 각도 전송용)
// * ======================================== */
//typedef union {
//    uint32_t raw;
//    struct {
//        uint32_t reserved     : 13;  // 0xAA (STM 송신 헤더)
//        uint32_t pan_angle  : 8; // Pan 각도 (0~179)
//        uint32_t tilt_angle : 7; // Tilt 각도 (0~89)
//        uint32_t stm_state : 3;
//        uint32_t laser_fire_complete : 1;
//    } fields;
//} TxPacket_t;

#endif /* AP_CONTROLLER_CONTROLLER_TRACKING_H_ */
