/*
 * SPI.h
 */

#ifndef DRIVER_SPI_SPI_H_
#define DRIVER_SPI_SPI_H_

#include "main.h" // HAL_StatusTypeDef, SPI_HandleTypeDef 등을 위해 필요

// 32비트 데이터를 바이트 단위로 접근하기 위한 공용체
typedef union {
    uint32_t data;
    uint8_t bytes[4];
} SPI_Data32;

// 함수 프로토타입
void SPI_Init(void);
HAL_StatusTypeDef SPI_Transfer32(uint32_t tx_data, uint32_t* rx_data);

// 필요한 경우 외부에서 CS를 직접 제어할 수 있도록 노출 (옵션)
void SPI_CS_Low(void);
void SPI_CS_High(void);

#endif /* DRIVER_SPI_SPI_H_ */
