/*
 * Model_Tracking.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_TRACKING_H_
#define AP_MODEL_MODEL_TRACKING_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

#define GAIN_X		0.005f
#define GAIN_Y		0.005f
#define MIN_X		0
#define MIN_Y		0
#define MAX_X		640 - 1
#define MAX_Y		480 - 1
#define CENTER_X	320
#define CENTER_Y	240
#define CENTER_PAN	90.0f
#define CENTER_TILT	135.0f
#define HALF_TILT	90.0f
#define PAN_MIN		0.0f
#define PAN_MAX		180.0f
#define TILT_MIN	90.0f
#define TILT_MAX	180.0f

typedef union {
	uint32_t raw;
	struct {
		uint32_t reserved :10;  // Bits 0~9
		uint32_t target_off :1;  // Bit 10
		uint32_t shoot :1;  // Bit 11
		uint32_t red_detect :1;  // Bit 12
		uint32_t y_pos :9;  // Bits 13~21
		uint32_t x_pos :10; // Bits 22~31
	} fields;
} RxPacket_t;


typedef union {
	uint32_t raw;
	struct {
		uint32_t reserved :14;	// Bits 0~13
		uint32_t stm_state :3;	// Bits 14~16
		uint32_t motor_tilt_data :7;	// Bits 17~23
		uint32_t motor_pan_data :8;		// Bits 24~31
	} fields;
} TxPacket_t;

typedef enum {
	TRACKING_IDLE, TRACKING_SEARCH, TRACKING_FOLLOW, TRACKING_LOST, TRACKING_AIMED
} trackingState_t;

typedef enum {
	EVENT_START,
	EVENT_STOP,
	EVENT_SERVO_TICK,
	EVENT_FPGA_DATA_RECEIVED
} trackingEvent_t;

typedef struct {
	float angle_pan;
	float angle_tilt;
	int x_pos;
	int y_pos;
	bool isDetected;
	bool isAimed;
	bool isVirFailed;
} tracking_t;

extern trackingState_t trackingState;
extern osMessageQId trackingEventMsgBox;
extern osMessageQId trackingDataMsgBox;
extern osPoolId poolTrackingEvent;
extern osPoolId poolTrackingData;

void Model_TrackingInit();
void Model_SetTrackingState(trackingState_t state);
trackingState_t Model_GetTrackingState();

#endif /* AP_MODEL_MODEL_TRACKING_H_ */
