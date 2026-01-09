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
    // SPI 리스너 초기화 (패킷 버퍼 및 헤더 세팅)
    Listener_SPITest_Init();
}

void Listener_Excute() {
    // FPGA에 데이터를 요청하는 시퀀스 실행
    Listener_SPITest_RequestData();
}
