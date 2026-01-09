/*
 * Presenter_SPITest.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_PRESENTER_PRESENTER_SPITEST_H_
#define AP_PRESENTER_PRESENTER_SPITEST_H_

#include "../Common/Common.h"
#include "../Model/Model_SPITest.h"
#include "../Listener/Listener_SPITest.h"

// Model의 데이터를 Listener의 데이터와 동기화하는 핵심 함수
void Presenter_SPITest_SyncData(Model_SPITest_t* model);

#endif
