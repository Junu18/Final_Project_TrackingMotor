

/*
 * Model_SPITest.h
 */
#ifndef AP_MODEL_MODEL_SPITEST_H_
#define AP_MODEL_MODEL_SPITEST_H_

#include "main.h"

/* * [FPGA 데이터 구조 매핑]
 * Raw Data Example: 0x83C20199
 * STM32(Little Endian) 기준: LSB(하위 비트)부터 정의
 */


/* * [FPGA 데이터 구조 매핑]
 * 총 32비트 데이터
 * * 비트 위치 | 내용         | 비트 수 | 설명
 * ------------------------------------------------
 * [0..10]   | padding_0          | 11 bit  | 0으로 채워짐
 * [    11]  | on_box     		 | 1  bit  | 모니터에서 파란색 박스 (display 전용)
 * [    12] | aim_detect    	 | 1  bit  | 모니터에서 가운데 aim (하얀색 십자가 여기에 감지된 부분이 좌표로 전송)
 * [13..21] | y_pos       		 | 9  bit  | Y좌표 (0~511)
 * [22..31] | x_pos      		 | 10 bit  | X좌표 (0~1023)
 */

typedef union
{
    uint32_t raw;  // 32비트 전체 데이터

    struct
    {
        uint32_t padding   : 11; // [0~10]  LSB
        uint32_t aim_state : 2;  // [11~12]
        uint32_t y_pos     : 9;  // [13~21]
        uint32_t x_pos     : 10; // [22~31] MSB
    } fields;

} RxPacket_t;


/* 모델 데이터 구조체 */
typedef struct
{
    RxPacket_t rx_packet;
    uint32_t   rx_count;
    uint32_t   rx_error_count;
} Model_SPITest_t;

/* 함수 프로토타입 */
void Model_SPITest_Init(Model_SPITest_t *p_model);
void Model_SPITest_UpdateRxData(Model_SPITest_t* model, RxPacket_t* rx);
uint8_t Model_SPITest_ValidateRx(Model_SPITest_t* model);

#endif /* AP_MODEL_MODEL_SPITEST_H_ */
