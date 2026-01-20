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

/* --- External Interface --- */
void Controller_Tracking_Init();
void Controller_Tracking_Excute();

/* --- Internal Logic Functions --- */
void Controller_Tracking_Unpack();
void Controller_Tracking_ResetData();
void Controller_Tracking_SetMiddle();
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
