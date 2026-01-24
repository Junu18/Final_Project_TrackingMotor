/*
 * SPI.h
 */

#ifndef DRIVER_SPI_SPI_H_
#define DRIVER_SPI_SPI_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

#define SPI_CS_PORT GPIOC
#define SPI_CS_PIN  GPIO_PIN_5

typedef void (*SPICallback)(void);

typedef union {
    uint32_t all;
    uint8_t  bytes[4];
} SPI_Raw32_t;

typedef struct {
    SPI_HandleTypeDef *phspi;
    SPI_Raw32_t rx;
    SPI_Raw32_t tx;
    SPICallback onComplete;
} hSpi32_t;

void SPI_Init(SPI_HandleTypeDef *phspi);
void SPI_RegisterCallback(SPICallback cb);
void SPI_StartTransfer_DMA(void);
uint32_t SPI_GetRxData(void);
void SPI_SetTxData(uint32_t data);

static inline void SPI_CS_Low() {
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
}
static inline void SPI_CS_High() {
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);
}

#endif
