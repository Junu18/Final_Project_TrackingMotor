/*
 * Controller_SPITest.c
 *
 * Created on: Jan 7, 2026
 * Author: kccistc
 * Description: Model 데이터를 LCD에 출력 (View 역할)
 */

#include "Controller_SPITest.h"
#include "../../driver/lcd/lcd.h"
#include <stdio.h>

void Controller_SPITest_UpdateLCD(Model_SPITest_t* model) {
    char buff[17]; // LCD 한 줄(16자) + NULL 문자

    // 1. 첫 번째 줄: 상태 및 수신 카운트
    // 출력 예시: "RX:1234 ERR:0   "
    // RX 카운트가 길어질 수 있으니 공간 배분에 주의
    sprintf(buff, "RX:%lu ERR:%lu   ", model->rx_count, model->rx_error_count);
    LCD_WriteStringXY(0, 0, buff);

    // 2. 두 번째 줄: 좌표 및 Aim 상태 표시
    // 출력 예시: "A:1 X:0640 Y:480" (딱 16글자)
    // A: Aim 상태 (0~3)
    // X: X좌표 (0~1023, 4자리)
    // Y: Y좌표 (0~511, 3자리 혹은 4자리)
    sprintf(buff, "A:%d X:%04lu Y:%03lu ",
            model->rx_packet.fields.aim_state,
            model->rx_packet.fields.x_pos,
            model->rx_packet.fields.y_pos);

    LCD_WriteStringXY(1, 0, buff);
}
