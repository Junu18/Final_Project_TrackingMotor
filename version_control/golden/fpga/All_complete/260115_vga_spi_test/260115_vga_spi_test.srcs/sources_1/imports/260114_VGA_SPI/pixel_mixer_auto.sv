`timescale 1ns / 1ps

module pixel_mixer_auto (
    input logic [11:0] img_bg,  // 카메라 영상

    input logic [9:0] aim_x,        // Red Tracker에서 온 X 좌표
    input logic [9:0] aim_y,        // Red Tracker에서 온 Y 좌표
    input logic       aim_detected, // 감지 여부

    input logic [9:0] x_pixel,  // 현재 VGA 픽셀 X
    input logic [9:0] y_pixel,  // 현재 VGA 픽셀 Y

    input logic [11:0] box_x_min,
    input logic [11:0] box_x_max,
    input logic [11:0] box_y_min,
    input logic [11:0] box_y_max,

    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,

    output logic       shoot
);

    // --- 색상 정의 ---
    localparam logic [11:0] RED = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0;
    localparam logic [11:0] BLUE = 12'h00F;
    localparam logic [11:0] WHITE = 12'hFFF;
    localparam logic [11:0] BLACK = 12'h000;
    localparam logic [11:0] YELLOW = 12'hFF0;  // 락온 알림 배경색

    localparam logic [11:0] AIM_COLOR = RED;
    localparam logic [11:0] TEXT_COLOR = GREEN;
    localparam logic [11:0] BOX_COLOR = GREEN;

    // --- 빨간 물체 조준점(십자선) 크기 ---
    localparam THK = 1;
    localparam LEN = 10;

    // --- 중앙 조준선 & 락온 UI 파라미터 ---
    localparam int CX = 320;
    localparam int CY = 240;
    localparam int LOCK_ZONE = 30;  // 락온 인식 범위 (+/- 30픽셀)

    // 우측 상단 UI 크기 및 위치
    localparam int KEY_SIZE = 30;
    localparam int CHAR_OFFSET = (KEY_SIZE - 16) / 2;
    localparam int LOCK_UI_X = 580;  // 우측 상단
    localparam int LOCK_UI_Y = 30;

    // --- 조준 완료 아이콘 (과녁 모양 비트맵) ---
    localparam logic [0:7][7:0] ICON_LOCK = '{
        8'b00111100,
        8'b01000010,
        8'b10011001,
        8'b10100101,
        8'b10100101,
        8'b10011001,
        8'b01000010,
        8'b00111100
    };

    // --- 내부 신호 ---
    logic is_locked_on;
    logic on_crosshair;
    logic is_lock_ui, is_lock_icon;
    logic [5:0] rel_x, rel_y;
    logic [2:0] char_x, char_y;
    int dist_x, dist_y;

    // --- 좌표 숫자 그리기 (Seven Segment Style) ---
    localparam w = 8;
    localparam h = 16;
    localparam scale = 2;

    function logic draw_digit(input [3:0] num, input [9:0] px, input [9:0] py,
                              input [9:0] ox, input [9:0] oy);
        logic sa, sb, sc, sd, se, sf, sg, on;
        int dx, dy;
        dx = (px - ox) / scale;
        dy = (py - oy) / scale;
        if (dx < 0 || dx > w || dy < 0 || dy > h) return 0;
        sa = (dy == 0) && (dx > 0 && dx < w);
        sb = (dx == w) && (dy > 0 && dy < (h / 2));
        sc = (dx == w) && (dy > (h / 2) && dy < h);
        sd = (dy == h) && (dx > 0 && dx < w);
        se = (dx == 0) && (dy > (h / 2) && dy < h);
        sf = (dx == 0) && (dy > 0 && dy < (h / 2));
        sg = (dy == (h / 2)) && (dx > 0 && dx < w);
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

    assign x_100 = aim_x / 100;
    assign x_10  = (aim_x / 10) % 10;
    assign x_1   = aim_x % 10;

    assign y_100 = aim_y / 100;
    assign y_10  = (aim_y / 10) % 10;
    assign y_1   = aim_y % 10;

    // --- 텍스트 렌더링 위치 설정 ---
    always_comb begin
        is_text_pixel = 0;
        if (draw_digit(x_100, x_pixel, y_pixel, 10, 10)) is_text_pixel = 1;
        if (draw_digit(x_10, x_pixel, y_pixel, 30, 10)) is_text_pixel = 1;
        if (draw_digit(x_1, x_pixel, y_pixel, 50, 10)) is_text_pixel = 1;

        if (draw_digit(y_100, x_pixel, y_pixel, 10, 50)) is_text_pixel = 1;
        if (draw_digit(y_10, x_pixel, y_pixel, 30, 50)) is_text_pixel = 1;
        if (draw_digit(y_1, x_pixel, y_pixel, 50, 50)) is_text_pixel = 1;
    end

    // --- 메인 그래픽 로직 ---
    always_comb begin
        // 초기화
        on_crosshair = 0;
        is_lock_ui = 0;
        is_lock_icon = 0;
        is_locked_on = 0;
        shoot        = 0;
        rel_x = 0;
        rel_y = 0;
        char_x = 0;
        char_y = 0;

        // 1. 락온 감지 (빨간 물체가 중앙 근처에 있는지 확인)
        if (aim_detected) begin
            if ( (aim_x > CX - LOCK_ZONE) && (aim_x < CX + LOCK_ZONE) &&
                 (aim_y > CY - LOCK_ZONE) && (aim_y < CY + LOCK_ZONE) ) begin
                is_locked_on = 1;
                shoot        = 1;
            end
        end

        // 2. 중앙 조준선 그리기 (보내주신 이미지 스타일)
        dist_x = (x_pixel > CX) ? (x_pixel - CX) : (CX - x_pixel);
        dist_y = (y_pixel > CY) ? (y_pixel - CY) : (CY - y_pixel);

        // 중앙 원
        if ((dist_x * dist_x + dist_y * dist_y) <= 36) on_crosshair = 1;
        // 내부 십자선
        if (dist_x < 2 && dist_y >= 12 && dist_y <= 22) on_crosshair = 1;
        if (dist_y < 2 && dist_x >= 12 && dist_x <= 22) on_crosshair = 1;
        // 외부 코너 브라켓
        if (dist_y >= 33 && dist_y <= 35 && dist_x >= 20 && dist_x <= 35)
            on_crosshair = 1;
        if (dist_x >= 33 && dist_x <= 35 && dist_y >= 20 && dist_y <= 35)
            on_crosshair = 1;

        // 3. 우측 상단 락온 알림 UI
        if (x_pixel >= LOCK_UI_X && x_pixel < LOCK_UI_X + KEY_SIZE && y_pixel >= LOCK_UI_Y && y_pixel < LOCK_UI_Y + KEY_SIZE) begin
            is_lock_ui = 1;
            rel_x = x_pixel - LOCK_UI_X;
            rel_y = y_pixel - LOCK_UI_Y;
            // 아이콘 비트맵 체크
            if (rel_x >= CHAR_OFFSET && rel_x < CHAR_OFFSET + 16 && rel_y >= CHAR_OFFSET && rel_y < CHAR_OFFSET + 16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (ICON_LOCK[char_y][7-char_x]) is_lock_icon = 1;
            end
        end
    end

    // --- 최종 픽셀 믹싱 (우선순위 적용) ---
    always_comb begin
        logic [11:0] pixel_color;

        // 우선순위 1: 우측 상단 락온 알림
        if (is_lock_ui) begin
            if (is_lock_icon)
                pixel_color = BLACK;  // 아이콘은 항상 검은색
            else
                pixel_color = is_locked_on ? YELLOW : WHITE; // 배경은 락온 시 주황색, 평소 흰색
        end  // 우선순위 2: 좌표 텍스트 (좌측 상단 초록 숫자)
        else if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
        end  // 우선순위 3: 중앙 조준선
        else if (on_crosshair) begin
            pixel_color = BLACK;
        end  // 우선순위 4: 타겟 물체의 조준점 (빨간 십자선)
        else if (aim_detected && 
                 ( ((y_pixel >= aim_y - THK) && (y_pixel <= aim_y + THK) && (x_pixel >= aim_x - LEN) && (x_pixel <= aim_x + LEN)) ||
                   ((x_pixel >= aim_x - THK) && (x_pixel <= aim_x + THK) && (y_pixel >= aim_y - LEN) && (y_pixel <= aim_y + LEN)) )) begin
            pixel_color = AIM_COLOR;
        end  // 우선순위 5: 타겟 물체의 바운딩 박스
        else if (aim_detected && 
                 ( ((y_pixel == box_y_min || y_pixel == box_y_max) && (x_pixel >= box_x_min && x_pixel <= box_x_max)) ||
                   ((x_pixel == box_x_min || x_pixel == box_x_max) && (y_pixel >= box_y_min && y_pixel <= box_y_max)) )) begin
            pixel_color = BOX_COLOR;
        end  // 6. 기본 카메라 배경
        else begin
            pixel_color = img_bg;
        end

        {r_port, g_port, b_port} = pixel_color;
    end

endmodule
