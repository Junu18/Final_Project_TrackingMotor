/*
 * ListenerTracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_LISTENER_LISTENERTRACKING_H_
#define AP_LISTENER_LISTENERTRACKING_H_

#include "../Model/ModelTracking.h"

/* ========================================
 * 전역 변수 선언 (extern)
 * ======================================== */
extern RxPacket_t g_rx_packet_tracking;  // SPI 수신 버퍼 (Callback에서 사용)

/* ========================================
 * 함수 프로토타입
 * ======================================== */
void Listener_Tracking_Init(void);
void Listener_Tracking_StartReceive(void);
void Listener_Tracking_Excute(void);
void Listener_Tracking_ProcessData(Model_Tracking_t* model);

#endif /* AP_LISTENER_LISTENERTRACKING_H_ */
