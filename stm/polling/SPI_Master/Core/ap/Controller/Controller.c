


#include "Controller.h"

// Presenter에서 가공 완료된 전역 모델 참조
extern Model_SPITest_t g_spiModel;

void Controller_Init() {
    // Controller 계층에서 필요한 초기화가 있다면 수행
    // (LCD_Init은 보통 하드웨어 설정이므로 Presenter나 Main에서 수행됨을 가정)
}

void Controller_Excute() {
    // 1. 다른 모드 로직 없이 오직 SPI 테스트(방공 시스템 데이터)만 LCD에 업데이트
    Controller_SPITest_UpdateLCD(&g_spiModel);
}
