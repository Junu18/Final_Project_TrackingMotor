/*
 * SPI.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "SPI.h"

static hSpi32_t hSpi32;

void SPI_Init(SPI_HandleTypeDef *phspi) {
    memset(&hSpi32, 0, sizeof(hSpi32));
    hSpi32.phspi = phspi;
    SPI_CS_High();
}

void SPI_RegisterCallback(SPICallback cb) {
    hSpi32.onComplete = cb;
}

uint32_t SPI_GetRxData(void) {
	uint32_t raw_data = hSpi32.rx.all;
	return __REV(raw_data);
}

void SPI_SetTxData(uint32_t data) {
    hSpi32.tx.all = data;
}

void SPI_StartTransfer_DMA(void) {
    if (hSpi32.phspi == NULL) return;
    SPI_CS_Low();
    for(volatile int i=0; i<200; i++);
    HAL_SPI_TransmitReceive_DMA(hSpi32.phspi, hSpi32.tx.bytes, hSpi32.rx.bytes, 4);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hSpi32.phspi != NULL && hspi->Instance == hSpi32.phspi->Instance) {
        SPI_CS_High();
        if (hSpi32.onComplete != NULL) {
            hSpi32.onComplete();
        }
    }
}
