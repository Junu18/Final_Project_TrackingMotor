/*
 * Presenter_SPITest.c
 */
#include "Presenter_SPITest.h"
#include "../Listener/Listener_SPITest.h" // Listener의 데이터를 가져오기 위해 헤더 포함

void Presenter_SPITest_Init(Model_SPITest_t *p_model)
{
    // 초기화 로직 (필요 시 작성)
}

/**
 * @brief Listener에 있는 최신 데이터를 Model로 복사해오는 함수
 */
void Presenter_SPITest_SyncData(Model_SPITest_t *p_model)
{
    /* * [핵심] 잃어버린 연결고리 연결!
     * Listener_SPITest.c에 있는 전역변수 g_rx_packet 값을
     * Model(p_model->rx_packet)로 복사합니다.
     */
    p_model->rx_packet.raw = g_rx_packet.raw;

    // 수신 카운트 증가
    p_model->rx_count++;
}
