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



// 이건 왜 필요하지?
#define GAIN_X		0.1f
#define GAIN_Y		0.1f
#define MIN_X		0
#define MIN_Y		0
#define MAX_X		640
#define MAX_Y		320


/** 
 * @brief 각도 데이터 구조체 
 * @note  Queue 전달용 / AngleData_t 구조체는 Presenter에서 사용 
 */
typedef struct {
    int16_t angle_pan;   // Pan 각도 (좌우)
    int16_t angle_tilt;  // Tilt 각도 (상하)
} AngleData_t;

/**
 * @brief 함수 
 * @note  프로토타입 선언
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
void Controller_Tracking_ComputeServoAngle();
void Controller_Tracking_PushData();

#endif /* AP_CONTROLLER_CONTROLLER_TRACKING_H_ */
