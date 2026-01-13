/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include <stdio.h>

tracking_t trackingData;
static uint32_t g_event_count = 0;  // Event 수신 횟수

void Controller_Tracking_Init() {
	trackingData.angle = 90.0f;

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
	osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
}

void Controller_Tracking_Excute() {
	trackingState_t state = Model_GetTrackingState();
	switch (state) {
	case TRACKING_0:
		trackingData.angle = 0.0f;
		break;
	case TRACKING_90:
		trackingData.angle = 90.0f;
		break;
	case TRACKING_180:
		trackingData.angle = 180.0f;
		break;
	}
	Controller_Tracking_PutAngle();
}

void Controller_Tracking_PutAngle() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t evtState;

	if (evt.status == osEventMessage) {
		evtState = evt.value.v;
		g_event_count++;

		// ===== DEBUG: Event 수신 =====
		if (g_event_count % 100 == 0) {
			printf("[CTRL_EVENT] Count: %ld | Event: 0x%04X\r\n", g_event_count, evtState);
		}

		if (evtState == EVENT_FPGA_DATA_RECEIVED) {
			// SPI에서 받은 데이터 처리
			printf("[CTRL_SPI_DATA] X: %d, Y: %d | Angle: %.1f\r\n",
				   g_rx_packet_tracking.x, g_rx_packet_tracking.y, trackingData.angle);
		} else if (evtState == EVENT_SERVO_TICK) {
			// 20ms 주기 Servo 업데이트 신호
			printf("[CTRL_SERVO_TICK] Sending servo update - Angle: %.1f\r\n", trackingData.angle);
		}
	} else if (evt.status != osOK && g_event_count % 100 == 0) {
		printf("[CTRL_ERROR] Queue receive error (no message)\r\n");
	}

	static tracking_t prevTrackingData;

	if (!memcmp(&trackingData, &prevTrackingData, sizeof(tracking_t)))
		return;
	memcpy(&prevTrackingData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
	osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
}
