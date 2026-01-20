/*
 * Listener_SPITest.c
 * 설명: SPI Polling 방식으로 32비트 데이터를 수신하고 CS 핀을 제어함
 */

#include "Listener_SPITest.h"
#include "spi.h"               // hspi1 핸들 사용
#include "../../driver/SPI/SPI.h" // SPI_CS_Low, SPI_CS_High 매크로 사용

/* 전역 변수 메모리 할당 */
RxPacket_t g_rx_packet;

/**
 * @brief 리스너 초기화
 */
void Listener_SPITest_Init(void)
{
    // 초기 상태: 통신 안 함 (High = 문 닫힘)
    // 방금 사용자님이 SPI_Init에서도 High로 맞추셨으니 이중으로 확실해집니다.
//    SPI_CS_High();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
}

/**
 * @brief 데이터 요청 및 수신 함수
 */
void Listener_SPITest_RequestData(void)
{
    /* 송신 헤더: 0xAA (FPGA가 인식할 시작 바이트) */
    uint8_t tx_buff[4] = {0xAA, 0x00, 0x00, 0x00};
    uint8_t rx_buff[4] = {0, };

    /* * [Step 2 - 동작 A] 문 열기 (CS Low)
     * 통신을 시작하겠다고 FPGA에게 알립니다.
     */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
//    SPI_CS_Low();

    /* * [Step 2 - 동작 B] 물건 교환 (데이터 송수신)
     * - Size가 4이므로 클럭 32개가 발생합니다.
     * - 이 함수가 끝날 때까지 CPU는 여기서 대기합니다.
     */
    if (HAL_SPI_TransmitReceive(&hspi1, tx_buff, rx_buff, 4, 100) == HAL_OK)
    {
        // 통신 성공
    }
    else
    {
        // 통신 실패 (필요 시 에러 처리)
    }

    /* * [Step 2 - 동작 C] 문 닫기 (CS High) <--- 요청하신 부분!
     * 전송이 끝났으므로 핀을 올려서 "끝났다"고 알립니다.
     * 이 코드가 있어야 파형이 바닥에 붙지 않고 다시 올라옵니다.
     */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
//    SPI_CS_High();

    /* * [Step 2 - 동작 D] 포장 (데이터 조립)
     * 4개의 바이트를 합쳐서 하나의 32비트 변수로 만듭니다.
     */
    g_rx_packet.raw = (rx_buff[0] << 24) |
                      (rx_buff[1] << 16) |
                      (rx_buff[2] << 8)  |
                      rx_buff[3];
}
