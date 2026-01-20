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
#include "../../driver/lcd/lcd.h"
#include "i2c.h"

#define GAIN_X		0.1f
#define GAIN_Y		0.1f
#define MIN_X		0
#define MIN_Y		0
#define MAX_X		640
#define MAX_Y		320

void Controller_Tracking_Init();
void Controller_Tracking_Excute();
void Controller_Tracking_HandleSignal(uint16_t currEvent);
void Controller_Tracking_UpdateState(uint16_t currEvent);
void Controller_Tracking_Idle(uint16_t currEvent);
void Controller_Tracking_Search(uint16_t currEvent);
void Controller_Tracking_Follow(uint16_t currEvent);
void Controller_Tracking_Lost(uint16_t currEvent);
void Controller_Tracking_Aimed(uint16_t currEvent);
void Controller_Tracking_ResetData();
void Controller_Tracking_ComputeServoAngle();
void Controller_Tracking_PushData();

typedef union {
    uint32_t raw;
    struct {
        uint32_t reserved : 10;  // 0x55 (검증용 헤더)
        uint32_t x_pos  : 10; // X 좌표 (0~4095)
        uint32_t y_pos  : 9; // Y 좌표 (0~4095)
        uint32_t red_detect : 1;
        uint32_t target_on_box_fpga: 1;
        uint32_t laser_fire_flag : 1;
    } fields;
} RxPacket_t;

/* ========================================
 * STM32에서 FPGA로 송신하는 32비트 패킷 구조
 * (추후 모터 각도 전송용)
 * ======================================== */
typedef union {
    uint32_t raw;
    struct {
        uint32_t reserved     : 13;  // 0xAA (STM 송신 헤더)
        uint32_t pan_angle  : 8; // Pan 각도 (0~179)
        uint32_t tilt_angle : 7; // Tilt 각도 (0~89)
        uint32_t stm_state : 3;
        uint32_t laser_fire_complete : 1;
    } fields;
} TxPacket_t;


#endif /* AP_CONTROLLER_CONTROLLER_TRACKING_H_ */
