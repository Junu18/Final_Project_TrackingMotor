/*
 * Model_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

 /**
 * @brief  include 파일
 */
#include "Model_Tracking.h"
#include <string.h>


/**
 * @brief  FreeRTOS Queue & Pool 정의
 * @note   
 */
trackingState_t trackingState = TRACKING_IDLE; // state shared memory


/**
 * @brief  Event Queue
 * @note   Listener → Controller
 */
osMessageQId trackingEventMsgBox;
osMessageQDef(trackingEventQue, 100, trackingEvent_t);  // 16 → 100 (overflow 방지)


/**
 * @brief  Data Queue
 * @note   Controller → Presenter
 * 		   tracking_t 구조체는 header에 정의
 */
osMessageQId trackingDataMsgBox;
osMessageQDef(trackingDataQue, 100, tracking_t);  // 16 → 100 (overflow 방지) 


/**
 * @brief  Memory Pool
 * @note   동적 메모리 할당 (Dynamic memory allocation provided by RTOS)
 * 		    
 */
osPoolDef(poolTrackingEvent, 16, trackingEvent_t);
osPoolId poolTrackingEvent;
osPoolDef(poolTrackingData, 32, tracking_t);
osPoolId poolTrackingData;


/**
 * @brief  Queue/Pool 초기화 (freertos.c의 MX_FREERTOS_Init에서 호출)
 */
void Model_TrackingInit() {
	poolTrackingEvent = osPoolCreate(osPool(poolTrackingEvent));
	poolTrackingData = osPoolCreate(osPool(poolTrackingData));
	trackingEventMsgBox = osMessageCreate(osMessageQ(trackingEventQue), NULL);
	trackingDataMsgBox = osMessageCreate(osMessageQ(trackingDataQue), NULL);
}

/**
 * @brief XY 좌표 업데이트 (Listener에서 호출)
 */
void Model_Tracking_UpdateXY(tracking_t* model, uint16_t x, uint16_t y) {
    model->x_pos = x;
    model->y_pos = y;
    model->is_Detected = 1;  // 타겟 감지됨
    model->rx_count++;
}

/**
 * @brief 서보모터 각도 업데이트 (Controller에서 호출)
 */
void Model_Tracking_UpdateAngles(tracking_t* model, int16_t pan, int16_t tilt) {
    model->angle_pan = pan;
    model->angle_tilt= tilt;

    // 송신 패킷 구성 (추후 FPGA로 전송용)
    model->tx_packet.fields.header = 0xAA;
    model->tx_packet.fields.angle_pan = pan;
    model->tx_packet.fields.angle_tilt= tilt;
}


/**
 * @brief  IDEL 상태 설정 함수
 * @return void
 */
void Model_SetTrackingState(trackingState_t state) {
	trackingState = state;										 // IDEL state set
}


/**
 * @brief  trackingState 읽기 함수
 * @return trackingState_t 현재 상태 반환
 */
trackingState_t Model_GetTrackingState() {
	return trackingState;
}


///**
// * @brief 수신 데이터 검증 (헤더 체크)
// */
//uint8_t Model_Tracking_ValidateRx(Model_Tracking_t* model) {
//    if (model->rx_packet.fields.header == 0x55) {
//        return 1;  // 유효함
//    } else {
//        model->rx_error_count++;
//        return 0;  // 무효함
//    }
//}
