/*
 * Controller_SPITest.c
 *
 * Created on: Jan 7, 2026
 * Author: kccistc
 */

#include "Controller_SPITest.h"
#include "../../driver/lcd/lcd.h"
#include <stdio.h>

void Controller_SPITest_UpdateLCD(Model_SPITest_t* model) {
    char buff[17];

    // 1. 첫 번째 줄: 상태 및 수신 카운트 표시
    // 예: "RX:123 ERR:0   "
    sprintf(buff, "RX:%lu ERR:%lu   ", model->rx_count, model->rx_error_count);
    LCD_WriteStringXY(0, 0, buff);

    // 2. 두 번째 줄: 좌표 표시
    // 예: "X:1024 Y:2048  "
    sprintf(buff, "X:%04lu Y:%04lu   ",
            model->rx_packet.fields.x_pos,
            model->rx_packet.fields.y_pos);
    LCD_WriteStringXY(1, 0, buff);
}
