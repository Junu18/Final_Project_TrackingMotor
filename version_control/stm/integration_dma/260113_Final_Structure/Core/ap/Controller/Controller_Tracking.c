/*
 * Controller_Traking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Controller_Tracking.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../driver/SPI/SPI.h"

#define DEBUG_GAIN_X_POS	10
#define DEBUG_GAIN_Y_POS	10

tracking_t trackingData;
RxPacket_t g_rx_packet_tracking;

void Controller_Tracking_Init() {
	Common_StartTIMInterrupt();
	Controller_Tracking_ResetData();
}

void Controller_Tracking_Excute() {
	osEvent evt = osMessageGet(trackingEventMsgBox, 0);
	uint16_t currEvent;
	if (evt.status != osEventMessage)
		return;
	currEvent = evt.value.v;

	Controller_Tracking_UpdateState(currEvent);
	Controller_Tracking_HandleSignal(currEvent);
}

void Controller_Tracking_HandleSignal(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();
	static trackingState_t prevState = TRACKING_IDLE;

	if (state != prevState) {
		Controller_Tracking_PushData();
		prevState = state;
	}

	if (currEvent == EVENT_SERVO_TICK) {
		if (state != TRACKING_IDLE) {
			Controller_Tracking_PushData();
		}
	}

	if (currEvent == EVENT_FPGA_DATA_RECEIVED) {
//		SPI_GetRxBuffer();
		Controller_Tracking_Unpack();
	}

	if (currEvent == EVENT_DEBUG && state != TRACKING_IDLE) {
		Controller_Tracking_ComputeServoAngle();
//		HAL_UART_Transmit(&huart2, (uint8_t*) "compute\n", 8, 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_PLUS) {
		if (trackingData.x_pos <= MAX_X - DEBUG_GAIN_X_POS) {
			trackingData.x_pos += DEBUG_GAIN_X_POS;
		}
//		char debugStr[30];
//		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
//		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);
	}

	if (currEvent == EVENT_DEBUG_PAN_MINUS) {
		if (trackingData.x_pos >= MIN_X + DEBUG_GAIN_X_POS) {
			trackingData.x_pos -= DEBUG_GAIN_X_POS;
		}
		char debugStr[30];
		sprintf(debugStr, "Current X_POS: %d\r\n", trackingData.x_pos);
		HAL_UART_Transmit(&huart2, (uint8_t*) debugStr, strlen(debugStr), 10);

	}

}

void Controller_Tracking_UpdateState(uint16_t currEvent) {
	trackingState_t state = Model_GetTrackingState();

	switch (state) {
	case TRACKING_IDLE:
		Controller_Tracking_Idle(currEvent);
		break;
	case TRACKING_SEARCH:
		Controller_Tracking_Search(currEvent);
		break;
	case TRACKING_FOLLOW:
		Controller_Tracking_Follow(currEvent);
		break;
	case TRACKING_LOST:
		Controller_Tracking_Lost(currEvent);
		break;
	case TRACKING_AIMED:
		Controller_Tracking_Aimed(currEvent);
		break;
	}
	trackingState_t currState = Model_GetTrackingState();
	const char *stateNames[] = { "IDLE", "SEARCH", "FOLLOW", "LOST", "AIMED" };
	char str[50];
	if (state != currState) {
		sprintf(str, "\r\n[EVENT: %d] STATE CHANGE: %s -> %s\r\n", currEvent,
				stateNames[state], stateNames[currState]);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 10);
	}
}

void Controller_Tracking_Idle(uint16_t currEvent) {
	if (currEvent == EVENT_START) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_Search(uint16_t currEvent) {
	if (currEvent == EVENT_STOP) {
		Model_SetTrackingState(TRACKING_IDLE);
	} else if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Follow(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_LOST) {
		Model_SetTrackingState(TRACKING_LOST);
	} else if (currEvent == EVENT_TARGET_AIMED) {
		Model_SetTrackingState(TRACKING_AIMED);
	}
}

void Controller_Tracking_Lost(uint16_t currEvent) {
	if (currEvent == EVENT_TARGET_ON) {
		Model_SetTrackingState(TRACKING_FOLLOW);
	}
}

void Controller_Tracking_Aimed(uint16_t currEvent) {
	if (currEvent == EVENT_CLEAR) {
		Model_SetTrackingState(TRACKING_SEARCH);
	}
}

void Controller_Tracking_ResetData() {
	trackingData.x_pos = CENTER_X;
	trackingData.y_pos = CENTER_Y;
	trackingData.angle_pan = CENTER_PAN;
	trackingData.angle_tilt = CENTER_TILT;
	trackingData.isDetected = false;
	trackingData.isAimed = false;
}

void Controller_Tracking_ComputeServoAngle() {
	int diff_x = trackingData.x_pos - CENTER_X;
	int diff_y = trackingData.y_pos - CENTER_Y;

	// Apply Deadzone  (Protect from vibration)
	if (abs(diff_x) < 5)
		diff_x = 0;
	if (abs(diff_y) < 5)
		diff_y = 0;

	trackingData.angle_pan += (float) diff_x * GAIN_X;
	trackingData.angle_tilt += (float) diff_y * GAIN_Y;
}

void Controller_Tracking_PushData() {
	static tracking_t prevData;

//	if (!memcmp(&trackingData, &prevData, sizeof(tracking_t)))
//		return;
	memcpy(&prevData, &trackingData, sizeof(tracking_t));

	tracking_t *pTrackingData = osPoolAlloc(poolTrackingData);
	if (pTrackingData != NULL) {
		memcpy(pTrackingData, &trackingData, sizeof(tracking_t));
		osMessagePut(trackingDataMsgBox, (uint32_t) pTrackingData, 0);
	}
}




// [설정] 조준으로 인정할 오차 범위 (픽셀)
#define AIM_TOLERANCE 30

void Controller_Tracking_Unpack() {


    // 1. [안전장치] 헤더 검사 (필수!)
    // 헤더가 깨졌으면 신뢰할 수 없는 데이터이므로 즉시 리턴
    if (g_rx_packet_tracking.fields.header != 0x55) {
//        trackingData.rx_error_count++;
        return;
    }

    // 2. 데이터 파싱
    // FPGA에서 받은 Raw 데이터를 우리가 쓰기 좋게 변수에 저장
    trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
    trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
    bool isDetectedNow = g_rx_packet_tracking.fields.target_on_box_fpga;

    // [중요] FPGA는 조준 여부를 안 주므로, MCU가 계산해야 함
    bool isAimedNow = false;
    if (isDetectedNow) {
        int diff_x = abs(trackingData.x_pos - CENTER_X);
        int diff_y = abs(trackingData.y_pos - CENTER_Y);
        // 오차 범위 이내면 조준된 것으로 판단
        if (diff_x < AIM_TOLERANCE && diff_y < AIM_TOLERANCE) {
            isAimedNow = true;
        }
    }

    // 3. 상태 변화 감지 및 이벤트 전송
    // (사용자 님이 스케치한 Edge Detection 로직의 업그레이드 버전)

    trackingState_t currentState = Model_GetTrackingState();

    if (isDetectedNow) {
        // A. 타겟이 감지된 상황
        if (isAimedNow) {
            // 감지됨 + 조준됨 -> AIMED 이벤트
            // (현재 상태가 이미 AIMED가 아닐 때만 전송)
            if (currentState != TRACKING_AIMED) {
                osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
            }
        }
        else {
            // 감지됨 + 조준 안 됨 -> FOLLOW (추적) 이벤트
            // (SEARCH 상태였거나, AIMED에서 벗어났을 때 전송)
            if (currentState != TRACKING_FOLLOW) {
                osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
            }
        }
    }
    else {
        // B. 타겟이 없는 상황 (LOST)
        // (이미 LOST 상태가 아닐 때만 전송)
        if (currentState != TRACKING_LOST && currentState != TRACKING_IDLE) {
            osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
        }
    }

    // 4. 데이터 갱신 (디버깅 표시용)
    trackingData.isDetected = isDetectedNow;
    trackingData.isAimed = isAimedNow;
//    trackingData.rx_count++;
}



//
//#define AIM_TOLERANCE 30  // 조준으로 인정할 오차 범위 (픽셀 단위)
//
//void Controller_Tracking_Unpack() {
//   extern RxPacket_t g_rx_packet_tracking;
//
//   // 1. 헤더 검증
//   if (g_rx_packet_tracking.fields.header != 0x55) {
////       trackingData.rx_error_count++;
//       return;
//   }
//
//   // 2. 데이터 추출
//   trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
//   trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
//// trackingData.rx_count++; // 수신 카운트 증가(디버깅)
//
//   // 플래그 추출 (1비트 꺼내오는거)
//   bool isTargetDetected = g_rx_packet_tracking.fields.target_on_box_fpga;
//   bool isLaserFired = g_rx_packet_tracking.fields.laser_fire_flag;
//
//
//   // 버튼 대신에 이벤트 가져오기
//
//   // 현재 상태 가져오기 (중복 이벤트 방지용)
//   static trackingState_t currentState = Model_GetTrackingState();
//
//
//   if (isTargetDetected) {
//       // [상황 A] 타겟이 감지됨
//
//       // 중심점과의 거리 계산 (절댓값)
//       int diff_x = abs(trackingData.x_pos - CENTER_X);
//       int diff_y = abs(trackingData.y_pos - CENTER_Y);
//
//       // 조준 범위 안에 들어왔는지 확인
//       if (diff_x < AIM_TOLERANCE && diff_y < AIM_TOLERANCE) {
//           // 오차 범위 이내 -> "조준 완료(AIMED)" 이벤트 발생
//           if (currentState != TRACKING_AIMED) {
//               osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
//           }
//       }
//       else {
//           // 오차 범위 밖 -> "추적 중(FOLLOW)" 이벤트 발생
//           // (SEARCH 상태였거나, AIMED에서 벗어났을 때)
//           if (currentState != TRACKING_FOLLOW) {
//               osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
//           }
//       }
//   }
//   else {
//       // [상황 B] 타겟이 없음 (FPGA가 놓침
//       // 이미 LOST 상태가 아니라면 "분실(LOST)" 이벤트 발생
//       if (currentState != TRACKING_LOST && currentState != TRACKING_SEARCH) {
//           osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
//       }
//   }
//







//   // (옵션) 레이저 발사 확인 플래그가 오면 처리
//   if (isLaserFired) {
//       // 필요하다면 발사 완료 처리 등 수행
//   }



//void Controller_Tracking_Unpack() {
//   extern RxPacket_t g_rx_packet_tracking;
//
//   // 1. 헤더 검증 (맨 앞 8비트가 0x55인지 확인)
//   if (g_rx_packet_tracking.fields.header != 0x55) {
//       // 헤더가 깨졌거나, 노이즈로 인해 비트가 밀린 경우 -> 버림
//       return;
//   }
//
//   // 2. 데이터 추출 (헤더가 맞으면 데이터도 안전함)
//   trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
//   trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
//
//   // 플래그 업데이트
//   bool isRed = g_rx_packet_tracking.fields.red_detect;
//   bool isTarget = g_rx_packet_tracking.fields.target_on_box_fpga;
//
//
//}


//void Controller_Tracking_Unpack() {
//   // SPI ISR에서 저장한 FPGA 패킷 참조
//   extern RxPacket_t g_rx_packet_tracking;
//
//   // 패킷에서 헤더 검증 (0x55)
//   if (g_rx_packet_tracking.fields.header != 0x55) {
//      trackingData.rx_error_count++;
//      return;
//   }
//
//   // 좌표 추출
//   trackingData.x_pos = g_rx_packet_tracking.fields.x_pos;
//   trackingData.y_pos = g_rx_packet_tracking.fields.y_pos;
//   trackingData.rx_packet.raw = g_rx_packet_tracking.raw;
//   trackingData.rx_count++;
//   trackingData.isDetected = true;
//   // 전부 비교해서 이벤트 완성
//}

