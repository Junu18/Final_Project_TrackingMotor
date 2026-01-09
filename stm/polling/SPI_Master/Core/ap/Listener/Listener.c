/*
 * Listener.c
 *
 *  Created on: Jan 5, 2026
 *      Author: kccistc
 */

/*
 * Listener.c
 */

#include "Listener.h"

void Listener_Init() {
    // Tracking 리스너 초기화 (SPI 인터럽트 모드)
    Listener_Tracking_Init();

    // 첫 번째 SPI 인터럽트 수신 시작
    Listener_Tracking_StartReceive();

    printf("[Listener] Task Started (Interrupt Mode)\r\n");
}

void Listener_Excute() {
    // Tracking Execute 호출 (Queue 대기 및 처리)
    Listener_Tracking_Excute();
}
