/*
 * Listener_SPITest.h
 *
 * 설명: SPI 통신을 담당하는 리스너 헤더
 */

#ifndef AP_LISTENER_LISTENER_SPITEST_H_
#define AP_LISTENER_LISTENER_SPITEST_H_

#include "main.h"
#include "../Model/Model_SPITest.h" // 데이터 구조체(RxPacket_t)를 알기 위해 포함

/* * [중요] 전역 변수 선언 (extern)
 * 실제 메모리 할당은 .c 파일에서 하고, 여기서는 "이런 변수가 있다"고 알림
 */
extern RxPacket_t g_rx_packet;

/* 함수 프로토타입 */
void Listener_SPITest_Init(void);
void Listener_SPITest_RequestData(void);

#endif /* AP_LISTENER_LISTENER_SPITEST_H_ */
