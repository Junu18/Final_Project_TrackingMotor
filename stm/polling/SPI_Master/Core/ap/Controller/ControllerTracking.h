/*
 * ControllerTracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_CONTROLLER_CONTROLLERTRACKING_H_
#define AP_CONTROLLER_CONTROLLERTRACKING_H_

#include "../Model/ModelTracking.h"

/* ========================================
 * 각도 데이터 구조체 (Queue 전달용)
 * ======================================== */
typedef struct {
    int16_t pan_angle;   // Pan 각도 (좌우)
    int16_t tilt_angle;  // Tilt 각도 (상하)
} AngleData_t;

/* ========================================
 * 함수 프로토타입
 * ======================================== */
void Controller_Tracking_Init(void);
void Controller_Tracking_Excute(void);
void Controller_Tracking_CalculateAngles(Model_Tracking_t* model);

#endif /* AP_CONTROLLER_CONTROLLERTRACKING_H_ */
