/*
 * Presenter_SPITest.c
 */
#include "Presenter_SPITest.h"
#include "../Listener/Listener_SPITest.h"
#include "../Model/Model_SPITest.h" // Update 함수를 쓰기 위해 필요

void Presenter_SPITest_Init(Model_SPITest_t *p_model)
{
}

void Presenter_SPITest_SyncData(Model_SPITest_t *p_model)
{
    /* 1. Listener에서 최신 Raw 데이터를 가져옵니다. */
    RxPacket_t new_data;
    new_data.raw = g_rx_packet.raw;

    /* 2. [핵심 수정] 직접 대입하지 않고, Model의 검증 함수를 호출합니다.
     * 이제 Model 내부에서 헤더(0x02)를 검사하고,
     * 통과했을 때만 rx_count를 올리고 데이터를 저장합니다.
     */
    Model_SPITest_UpdateRxData(p_model, &new_data);
}

