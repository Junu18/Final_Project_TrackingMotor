/*
 * PresenterTracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_PRESENTER_PRESENTERTRACKING_H_
#define AP_PRESENTER_PRESENTERTRACKING_H_

#include "../Model/ModelTracking.h"
#include "../Controller/ControllerTracking.h"

/* ========================================
 * 함수 프로토타입
 * ======================================== */
void Presenter_Tracking_Init(void);
void Presenter_Tracking_Excute(void);
void Presenter_Tracking_UpdateServo(Model_Tracking_t* model);
void Presenter_Tracking_UpdateLCD(Model_Tracking_t* model);

#endif /* AP_PRESENTER_PRESENTERTRACKING_H_ */
