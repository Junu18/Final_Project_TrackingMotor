/*
 * SPI.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kccistc
 */

#include "SPI.h"

/*
typedef enum {
	SPI_STATE_READY, SPI_STATE_BUSY_TX, SPI_STATE_BUSY_RX, SPI_STATE_ERROR
} SPI_State_t;

typedef struct {
	SPI_HandleTypeDef *hspi;    // HAL 인스턴스 연결
	GPIO_TypeDef *csPort;       // CS 제어용 포트
	uint16_t csPin;             // CS 제어용 핀
	volatile int isBusy;
	uint8_t *pTxBuffer;         // 송신 버퍼 포인터
	uint8_t *pRxBuffer;         // 수신 버퍼 포인터 추가

	uint16_t size;              // 전송/수신할 총 크기 (전이중은 송수신 크기가 같음)
	volatile SPI_State_t state; // 드라이버 상태 (volatile 추가 권장)

	void (*TransferComplete)(void); // 완료 후 실행할 콜백
} SPI32_Handler_t;

SPI32_Handler_t *hSPI;

void SPI_Init(SPI_HandleTypeDef *phspi, GPIO_TypeDef *csPort, uint16_t csPin) {
	hSPI->hspi = phspi;
	hSPI->csPort = csPort;
	hSPI->csPin = csPin;
}

HAL_StatusTypeDef MySPI_Transmit_IT(SPI_Driver_t *pDriver, uint8_t *pData,
		uint16_t Size) {
	if (pDriver->state != SPI_STATE_READY)
		return HAL_BUSY;

	pDriver->pTxBuffer = pData;
	pDriver->TxSize = Size;
	pDriver->TxCount = 0;
	pDriver->state = SPI_STATE_BUSY_TX;

	// 1. SPI 인터럽트(TX Empty) 활성화
	__HAL_SPI_ENABLE_IT(pDriver->hspi, SPI_IT_TXE);

	return HAL_OK;
}

void MySPI_IRQHandler(SPI_Driver_t *pDriver) {
	SPI_HandleTypeDef *hspi = pDriver->hspi;

	// TXE (Transmit buffer empty) 인터럽트 발생 시
	if (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE)
			&& __HAL_SPI_GET_IT_SOURCE(hspi, SPI_IT_TXE)) {
		if (pDriver->TxCount < pDriver->TxSize) {
			// 데이터 한 바이트 전송
			hspi->Instance->DR = pDriver->pTxBuffer[pDriver->TxCount++];
		} else {
			// 전송 완료 시 인터럽트 끄기 및 상태 변경
			__HAL_SPI_DISABLE_IT(hspi, SPI_IT_TXE);
			pDriver->state = SPI_STATE_READY;

			if (pDriver->TransferComplete) {
				pDriver->TransferComplete(); // 완료 콜백 실행
			}
		}
	}
}

static struct {
	SPI_HandleTypeDef *hspi;
	GPIO_TypeDef *csPort;
	uint16_t csPin;
	volatile int isBusy;
	uint8_t txBuf[4];
	uint8_t rxBuf[4];
	uint32_t lastRxData;
} hSPI32_t;

hSPI32_t hSPI32;

void SPI32_Init(SPI_HandleTypeDef *phspi, GPIO_TypeDef *csPort, uint16_t csPin) {
	hSPI32.hspi = phspi;
	hSPI32.csPort = csPort;
	hSPI32.csPin = csPin;
	hSPI32.isBusy = 0;

	SPI32_CSHigh();
}

void SPI32_CSHigh() {
	HAL_GPIO_WritePin(hSPI32.csPort, hSPI32.csPin, GPIO_PIN_SET);
}

void SPI32_CSLow() {
	HAL_GPIO_WritePin(hSPI32.csPort, hSPI32.csPin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef SPI32_Transfer(uint32_t txData) {
	if (g_spi32.isBusy)
		return HAL_BUSY;

	g_spi32.isBusy = 1;

	// 32비트 데이터를 8비트 버퍼로 분해 (Big Endian)
	g_spi32.txBuf[0] = (uint8_t) (txData >> 24);
	g_spi32.txBuf[1] = (uint8_t) (txData >> 16);
	g_spi32.txBuf[2] = (uint8_t) (txData >> 8);
	g_spi32.txBuf[3] = (uint8_t) txData;

	HAL_GPIO_WritePin(g_spi32.csPort, g_spi32.csPin, GPIO_PIN_RESET); // CS Low

	// 4바이트 고정 전송 (DMA)
	if (HAL_SPI_TransmitReceive_DMA(g_spi32.hspi, g_spi32.txBuf, g_spi32.rxBuf,
			4) != HAL_OK) {
		HAL_GPIO_WritePin(g_spi32.csPort, g_spi32.csPin, GPIO_PIN_SET);
		g_spi32.isBusy = 0;
		return HAL_ERROR;
	}
	return HAL_OK;
}

uint32_t SPI32_GetReceivedData(void) {
	return g_spi32.lastRxData;
}

int SPI32_IsBusy(void) {
	return g_spi32.isBusy;
}

// HAL 콜백: 전송 완료 시 자동 호출
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi == g_spi32.hspi) {
		HAL_GPIO_WritePin(g_spi32.csPort, g_spi32.csPin, GPIO_PIN_SET); // CS High

		// 수신된 4바이트를 32비트로 조립
		g_spi32.lastRxData = ((uint32_t) g_spi32.rxBuf[0] << 24)
				| ((uint32_t) g_spi32.rxBuf[1] << 16)
				| ((uint32_t) g_spi32.rxBuf[2] << 8)
				| ((uint32_t) g_spi32.rxBuf[3]);

		g_spi32.isBusy = 0; // 상태 해제
	}
}

*/
