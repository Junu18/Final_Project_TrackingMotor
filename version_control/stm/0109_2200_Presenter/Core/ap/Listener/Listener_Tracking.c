/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Listener_Tracking.h"
#include <stdio.h>

void Listener_Tracking_Init() {
	/* SPI interrupt is initialized in MX_SPI1_Init() */
	/* No button handling needed - using SPI interrupt instead */
	printf("[LIST] Init OK\r\n");
	for(volatile int i = 0; i < 100000; i++);
}

void Listener_Tracking_Excute() {
	/* SPI data reception is handled by interrupt handler */
	/* No polling needed */
}

void Listener_Tracking_StartReceive() {
	/* Start SPI in interrupt mode */
	/* Configure SPI1 for continuous reception */
	
	/* SPI1 configuration for interrupt-driven reception */
	SPI1->CR1 = 0;
	SPI1->CR1 |= SPI_CR1_MSTR;      // Master mode
	SPI1->CR1 |= SPI_CR1_SPE;       // SPI enable
	
	/* SPI1 interrupt enable */
	SPI1->CR2 = 0;
	SPI1->CR2 |= SPI_CR2_RXNEIE;    // RX interrupt enable
	SPI1->CR2 |= SPI_CR2_ERRIE;     // Error interrupt enable
	
	/* Start first SPI reception */
	SPI1->DR;  // Clear RX buffer
	SPI1->CR1 |= SPI_CR1_SPE;
}


