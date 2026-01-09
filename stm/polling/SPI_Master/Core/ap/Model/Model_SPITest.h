/*
 * Model_SPITest.h
 *
 * Created on: Jan 7, 2026
 * Author: kccistc
 */

#ifndef AP_MODEL_MODEL_SPITEST_H_
#define AP_MODEL_MODEL_SPITEST_H_

#include "../Common/Common.h"

// [추가] STM32에서 FPGA로 보내는 32비트 패킷 구조 정의
typedef union {
    uint32_t raw;
    struct {
        uint32_t header : 8;  // 0xAA (STM 송신용 헤더)
        uint32_t x_pos  : 12; // 테스트용 혹은 모터 제어용 X (필요에 따라 angle로 변경 가능)
        uint32_t y_pos  : 12; // 테스트용 혹은 모터 제어용 Y (필요에 따라 angle로 변경 가능)
    } fields;
} TxPacket_t;

// 32비트 패킷의 내부 구조를 정의 (FPGA에서 수신용)
typedef union {
    uint32_t raw;
    struct {
        uint32_t header : 8;  // 0x55 (검증용)
        uint32_t x_pos  : 12; // 객체 X 좌표 (0~4095)
        uint32_t y_pos  : 12; // 객체 Y 좌표 (0~4095)
    } fields;
} RxPacket_t;

typedef struct {
    // FPGA에서 받은 데이터
    RxPacket_t rx_packet;

    // 통계 및 상태
    uint32_t rx_count;          // 수신 성공 횟수
    uint32_t rx_error_count;    // 수신 실패 횟수
} Model_SPITest_t;

void Model_SPITest_Init(Model_SPITest_t* model);
void Model_SPITest_UpdateRxData(Model_SPITest_t* model, RxPacket_t* rx);
uint8_t Model_SPITest_ValidateRx(Model_SPITest_t* model);

#endif /* AP_MODEL_MODEL_SPITEST_H_ */
