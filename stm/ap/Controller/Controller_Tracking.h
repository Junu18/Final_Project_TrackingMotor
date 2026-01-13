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

/**
 * @brief 각도 변환 게인 및 화면 설정
 */
#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10
#define SCREEN_CENTER_X		320  // 화면 중심 X (640/2)
#define SCREEN_CENTER_Y		240  // 화면 중심 Y (480/2)
#define ANGLE_GAIN			0.1f // 각도 변환 게인
#define GAIN_X				0.1f
#define GAIN_Y				0.1f

/**
 * @brief 함수 프로토타입 선언
 */
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
void Controller_Tracking_Unpack();
void Controller_Tracking_ComputeServoAngle();
void Controller_Tracking_PushData();

#endif /* AP_CONTROLLER_CONTROLLER_TRACKING_H_ */
