/*
 * SPI.c
 */

#include "SPI.h"

extern SPI_HandleTypeDef hspi1;

// CS 핀 설정 (사용하시는 보드 핀번호 확인 필수: PA4 기준)
#define SPI_CS_PORT GPIOA
#define SPI_CS_PIN  GPIO_PIN_4

void SPI_Init(void) {
    // 시작 시 CS를 High로 설정하여 통신 대기 상태로 만듦
    SPI_CS_High();
}

void SPI_CS_Low(void) {
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
}

void SPI_CS_High(void) {
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);
}

// 32비트 송수신 함수
HAL_StatusTypeDef SPI_Transfer32(uint32_t tx_data, uint32_t* rx_data) {
    HAL_StatusTypeDef status;

    SPI_CS_Low(); // 1. CS 활성화

    // 2. 32비트(4바이트) 데이터 송수신
    // (uint8_t*)로 캐스팅하여 바이트 단위로 4개 전송
    // Timeout은 100ms로 충분히 설정
    status = HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx_data, (uint8_t*)rx_data, 4, 100);

    SPI_CS_High(); // 3. CS 비활성화

    return status;
}
