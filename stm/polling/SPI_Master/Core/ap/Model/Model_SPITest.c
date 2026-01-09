/*
 * Model_SPITest.c
 *
 * Created on: Jan 7, 2026
 * Author: kccistc
 * Description: 수신된 패킷 데이터를 검증하고 모델에 업데이트함
 */


#include "Model_SPITest.h"
#include <string.h>

/* 모델 초기화 */
void Model_SPITest_Init(Model_SPITest_t* model) {
    memset(model, 0, sizeof(Model_SPITest_t));
}

/* 수신된 Raw 데이터를 모델에 업데이트하고 검증 */
void Model_SPITest_UpdateRxData(Model_SPITest_t* model, RxPacket_t* rx) {
    RxPacket_t temp_rx = *rx;

    /* [검증 로직]
     * 헤더가 사라졌으므로, Padding(Empty) 영역이 0인지 확인
     */
    if (temp_rx.fields.padding == 0) {
        model->rx_packet = temp_rx;
        model->rx_count++;
    } else {
        model->rx_error_count++;
    }
}

/* 외부 검증 함수 (수정됨) */
uint8_t Model_SPITest_ValidateRx(Model_SPITest_t* model) {
    // [에러 수정] header 변수는 더 이상 존재하지 않습니다.
    // 대신 padding이 0인지 확인하는 것으로 변경합니다.
    if (model->rx_packet.fields.padding != 0) {
        return 0;  // 실패 (0이 아니면 에러)
    }
    return 1;  // 성공
}
