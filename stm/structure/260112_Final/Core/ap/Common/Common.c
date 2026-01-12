/*
 * Common.c
 *
 *  Created on: Jan 6, 2026
 *      Author: kccistc
 */

#include "Common.h"

void Common_StartTIMInterrupt() {
	HAL_TIM_Base_Start_IT(&htim3);
}

void Common_StopTIMInterrupt() {
	HAL_TIM_Base_Stop_IT(&htim3);
}

/*
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == SPI1) {
		osMessagePut(trackingEventMsgBox, EVENT_SPI_CMPLT, 0);
	}
}
*/
