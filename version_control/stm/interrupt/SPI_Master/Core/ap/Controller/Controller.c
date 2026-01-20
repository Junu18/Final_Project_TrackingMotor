


#include "Controller.h"

// Tracking 전역 모델 참조
extern Model_Tracking_t g_tracking_model;

void Controller_Init() {
    // Tracking Controller 초기화
    Controller_Tracking_Init();

    printf("[Controller] Task Started\r\n");
}

void Controller_Excute() {
    // Tracking Execute 호출 (TaskNotify 대기 및 각도 계산)
    Controller_Tracking_Excute();
}
