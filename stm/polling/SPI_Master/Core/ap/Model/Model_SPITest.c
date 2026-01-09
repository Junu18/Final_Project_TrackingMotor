/*
 * Model_SPITest.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */


#include "Model_SPITest.h"
#include <string.h>

void Model_SPITest_Init(Model_SPITest_t* model) {
    memset(model, 0, sizeof(Model_SPITest_t));
}

// 수신된 Raw 데이터를 모델에 업데이트하고 검증 결과에 따라 카운트 증가
void Model_SPITest_UpdateRxData(Model_SPITest_t* model, RxPacket_t* rx) {
    // 임시 저장 후 검증
    RxPacket_t temp_rx = *rx;

    // 헤더 체크 (0x55)
    if (temp_rx.fields.header == 0x55) {
        model->rx_packet = temp_rx;
        model->rx_count++;
    } else {
        model->rx_error_count++;
    }
}

// 외부에서 필요할 때 수동으로 검증하기 위한 함수
uint8_t Model_SPITest_ValidateRx(Model_SPITest_t* model) {
    if (model->rx_packet.fields.header != 0x55) {
        return 0;  // 실패
    }
    return 1;  // 성공
}
