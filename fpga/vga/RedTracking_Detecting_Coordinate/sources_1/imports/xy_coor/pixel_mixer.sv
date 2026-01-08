`timescale 1ns / 1ps

module pixel_mixer (
    input  logic [11:0] img_bg,       // 카메라 영상

    input  logic [ 9:0] aim_x,        // Red Tracker에서 온 X 좌표
    input  logic [ 9:0] aim_y,        // Red Tracker에서 온 Y 좌표
    input  logic        aim_detected, // 감지 여부

    input  logic [ 9:0] x_pixel,      // 현재 VGA 픽셀 X
    input  logic [ 9:0] y_pixel,      // 현재 VGA 픽셀 Y

    input  logic [11:0] box_x_min,
    input  logic [11:0] box_x_max,
    input  logic [11:0] box_y_min,
    input  logic [11:0] box_y_max,

    output logic [ 3:0] r_port,
    output logic [ 3:0] g_port,
    output logic [ 3:0] b_port
);

    // --- 색상 정의 ---
    localparam logic [11:0] RED   = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0; // 좌표 글씨 색상 (초록색)
    localparam logic [11:0] BLUE  = 12'h00F;
    localparam logic [11:0] WHITE = 12'hFFF;

    localparam logic [11:0] AIM_COLOR = RED;
    localparam logic [11:0] TEXT_COLOR = GREEN;
    localparam logic [11:0] CENTER_COLOR = BLUE;
    localparam logic [11:0] LOCK_COLOR = WHITE;
    localparam logic [11:0] BOX_COLOR = GREEN;

    // --- 조준점 크기 ---
    localparam THK = 1;  
    localparam LEN = 10; 

    // 좌표 표시
    localparam w     = 8; 
    localparam h     = 16; 
    localparam scale = 2; 

    logic is_locked;
    assign is_locked = (aim_x >= 288 && aim_x <= 351) && (aim_y >= 208 && aim_y <= 271);

    function logic draw_digit(input [3:0] num, input [9:0] px, input [9:0] py,
                              input [9:0] ox, input [9:0] oy);

        // 7-segment 논리 변수
        logic sa, sb, sc, sd, se, sf, sg; 
        logic on;
        int dx, dy;

        // 현재 픽셀을 글자 내부 좌표로 변환
        dx = (px - ox) / scale;
        dy = (py - oy) / scale;

        // 글자 범위 밖이면 그리지 않음
        if (dx < 0 || dx > w || dy < 0 || dy > h) return 0;

        // 세그먼트 정의 (디지털 숫자 모양)
        sa = (dy == 0) && (dx > 0 && dx < w);             // 상단 가로
        sb = (dx == w) && (dy > 0 && dy < (h/2));         // 우측 상단 세로
        sc = (dx == w) && (dy > (h/2) && dy < h);         // 우측 하단 세로
        sd = (dy == h) && (dx > 0 && dx < w);             // 하단 가로
        se = (dx == 0) && (dy > (h/2) && dy < h);         // 좌측 하단 세로
        sf = (dx == 0) && (dy > 0 && dy < (h/2));         // 좌측 상단 세로
        sg = (dy == (h/2)) && (dx > 0 && dx < w);         // 중간 가로

        case (num)
            0: on = sa | sb | sc | sd | se | sf;
            1: on = sb | sc;
            2: on = sa | sb | sd | se | sg;
            3: on = sa | sb | sc | sd | sg;
            4: on = sb | sc | sf | sg;
            5: on = sa | sc | sd | sf | sg;
            6: on = sa | sc | sd | se | sf | sg;
            7: on = sa | sb | sc | sf;
            8: on = sa | sb | sc | sd | se | sf | sg;
            9: on = sa | sb | sc | sd | sf | sg;
            default: on = 0;
        endcase
        return on;
    endfunction

    // --- 좌표 숫자 추출 로직 ---
    logic is_text_pixel;
    logic [3:0] x_100, x_10, x_1;
    logic [3:0] y_100, y_10, y_1;

    // 좌표를 100의 자리, 10의 자리, 1의 자리로 분리
    assign x_100 = aim_x / 100;
    assign x_10  = (aim_x / 10) % 10;
    assign x_1   = aim_x % 10;

    assign y_100 = aim_y / 100;
    assign y_10  = (aim_y / 10) % 10;
    assign y_1   = aim_y % 10;

    // --- 텍스트 렌더링 위치 설정 ---
    always_comb begin
        is_text_pixel = 0;

        // [X 좌표 표시] 위치: (10, 10) 부터
        if (draw_digit(x_100, x_pixel, y_pixel, 10, 10)) is_text_pixel = 1; // 100의 자리
        if (draw_digit(x_10,  x_pixel, y_pixel, 30, 10)) is_text_pixel = 1; // 10의 자리
        if (draw_digit(x_1,   x_pixel, y_pixel, 50, 10)) is_text_pixel = 1; // 1의 자리

        // [Y 좌표 표시] 위치: (10, 50) 부터 (X좌표 아래)
        if (draw_digit(y_100, x_pixel, y_pixel, 10, 50)) is_text_pixel = 1; 
        if (draw_digit(y_10,  x_pixel, y_pixel, 30, 50)) is_text_pixel = 1; 
        if (draw_digit(y_1,   x_pixel, y_pixel, 50, 50)) is_text_pixel = 1; 
    end


    // --- 최종 픽셀 믹싱 ---
    always_comb begin
        logic [11:0] pixel_color;

        // 1. 기본 배경: 카메라 영상
        pixel_color = img_bg;

        // 2. 센터 박스: 파랑색
        if (y_pixel == 192+16 || y_pixel == 287-16) begin
            if (x_pixel >= 256+32 && x_pixel <= 383-32)
                pixel_color = CENTER_COLOR;
        end else if (x_pixel == 256+32 || x_pixel == 383-32) begin
            if (y_pixel > 192+16 && y_pixel < 287-16)
                pixel_color = CENTER_COLOR;
        end

        // 2. 조준점 (Aim): 빨간색
        // 4. 조준점 (Aim) 그리기
        if (aim_detected) begin
            if ( ((y_pixel >= aim_y - THK) && (y_pixel <= aim_y + THK) && (x_pixel >= aim_x - LEN) && (x_pixel <= aim_x + LEN)) ||
                 ((x_pixel >= aim_x - THK) && (x_pixel <= aim_x + THK) && (y_pixel >= aim_y - LEN) && (y_pixel <= aim_y + LEN)) ) begin
                
                // 락온 상태면 하얀색, 아니면 빨간색
                pixel_color = is_locked ? LOCK_COLOR : AIM_COLOR;
            end
        end
        
        // 3. 바운딩 박스 (Bounding Box) 그리기
        // 조건: 감지됨(aim_detected) AND 락온되지 않음(!is_locked)
        if (aim_detected && !is_locked) begin
            // 테두리만 그리기 (위/아래 선 OR 왼쪽/오른쪽 선)
            if ( ((y_pixel == box_y_min || y_pixel == box_y_max) && (x_pixel >= box_x_min && x_pixel <= box_x_max)) ||
                 ((x_pixel == box_x_min || x_pixel == box_x_max) && (y_pixel >= box_y_min && y_pixel <= box_y_max)) ) begin
                pixel_color = BOX_COLOR; // 초록색 박스
            end
        end

        // 3. 텍스트 (좌표): 초록색 (가장 위에 그림)
        if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
        end

        {r_port, g_port, b_port} = pixel_color;
    end

endmodule
