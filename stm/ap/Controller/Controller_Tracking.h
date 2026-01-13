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
