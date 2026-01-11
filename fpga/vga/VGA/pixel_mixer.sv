`timescale 1ns / 1ps

module pixel_mixer (
    input logic [11:0] img_bg,

    // 타겟 정보
    input logic [15:0][9:0] aim_x_all,
    input logic [15:0][9:0] aim_y_all,
    input logic [15:0]      aim_detected_all,

    input logic [15:0][11:0] box_x_min_all,
    input logic [15:0][11:0] box_x_max_all,
    input logic [15:0][11:0] box_y_min_all,
    input logic [15:0][11:0] box_y_max_all,

    // 마우스 정보
    input [9:0] mouse_x_pixel,
    input [9:0] mouse_y_pixel,
    input       click_l,
    click_r,
    click_m,

    // 제어 신호
    input logic target_off,
    input logic is_locked,  // 락온 모드 여부
    input logic [3:0] locked_idx,  // 락온된 인덱스
    input logic center_hit,  // 중앙 정렬 여부 (에임 색상 변경용)

    input logic [9:0] x_pixel,
    input logic [9:0] y_pixel,

    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,

    // target coor
    output logic [9:0] target_x_coor,
    output logic [9:0] target_y_coor

);


    // --- 색상 정의 ---
    localparam logic [11:0] RED = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0;
    localparam logic [11:0] BLUE = 12'h00F;
    localparam logic [11:0] WHITE = 12'hFFF;
    localparam logic [11:0] YELLOW = 12'hFF0;

    logic [11:0] aim_color_curr;  // 동적 에임 색상
    assign aim_color_curr = (is_locked && center_hit) ? WHITE : RED;

    localparam logic [11:0] BOX_COLOR = GREEN;
    localparam logic [11:0] TEXT_COLOR = GREEN;
    localparam logic [11:0] CENTER_BOX_COLOR = BLUE;
    localparam logic [11:0] MOUSE_COLOR = WHITE;
    localparam logic [11:0] CLICK_COLOR = YELLOW;

    localparam int AIM_LEN = 10;
    localparam int AIM_THK = 1;

    // --- 텍스트 표시 기능 (Draw Digit) ---
    localparam w = 8, h = 16, scale = 2;
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

    // 텍스트 픽셀 판별
    logic is_text_pixel;
    logic [9:0] target_x, target_y;  // 텍스트로 띄울 좌표
    logic [3:0] x_100, x_10, x_1;
    logic [3:0] y_100, y_10, y_1;

    assign target_x_coor = target_x;
    assign target_y_coor = target_y;

    always_comb begin
        is_text_pixel = 0;
        target_x      = 0;
        target_y      = 0;
        // 락온 상태일 때만 좌표 표시
        if (is_locked && aim_detected_all[locked_idx]) begin
            target_x = aim_x_all[locked_idx];
            target_y = aim_y_all[locked_idx];

            x_100 = target_x / 100;
            x_10 = (target_x / 10) % 10;
            x_1 = target_x % 10;
            y_100 = target_y / 100;
            y_10 = (target_y / 10) % 10;
            y_1 = target_y % 10;

            if (draw_digit(x_100, x_pixel, y_pixel, 10, 10)) is_text_pixel = 1;
            if (draw_digit(x_10, x_pixel, y_pixel, 30, 10)) is_text_pixel = 1;
            if (draw_digit(x_1, x_pixel, y_pixel, 50, 10)) is_text_pixel = 1;

            if (draw_digit(y_100, x_pixel, y_pixel, 10, 50)) is_text_pixel = 1;
            if (draw_digit(y_10, x_pixel, y_pixel, 30, 50)) is_text_pixel = 1;
            if (draw_digit(y_1, x_pixel, y_pixel, 50, 50)) is_text_pixel = 1;
        end
    end

    // --- 마우스 아이콘 ---
    logic [11:0] mouse_icon[0:15];
    always_comb begin
        mouse_icon[0]  = 12'b100000000000;
        mouse_icon[1]  = 12'b110000000000;
        mouse_icon[2]  = 12'b111000000000;
        mouse_icon[3]  = 12'b111100000000;
        mouse_icon[4]  = 12'b111110000000;
        mouse_icon[5]  = 12'b111111000000;
        mouse_icon[6]  = 12'b111111100000;
        mouse_icon[7]  = 12'b111111110000;
        mouse_icon[8]  = 12'b111111111000;
        mouse_icon[9]  = 12'b111110000000;
        mouse_icon[10] = 12'b100011000000;
        mouse_icon[11] = 12'b000011000000;
        mouse_icon[12] = 12'b000001100000;
        mouse_icon[13] = 12'b000001100000;
        mouse_icon[14] = 12'b000000110000;
        mouse_icon[15] = 12'b000000000000;
    end

    // --- 그리기 변수 ---
    logic on_box, on_aim, on_mouse, on_center_box;
    int aim_x_int, aim_y_int, x_pix_int, y_pix_int, m_dx, m_dy;

    always_comb begin
        on_box = 0;
        on_aim = 0;
        on_mouse = 0;
        on_center_box = 0;
        x_pix_int = x_pixel;
        y_pix_int = y_pixel;

        // 1. 마우스
        m_dx = x_pix_int - mouse_x_pixel;
        m_dy = y_pix_int - mouse_y_pixel;
        if (m_dx >= 0 && m_dx < 12 && m_dy >= 0 && m_dy < 16) begin
            if (mouse_icon[m_dy][11-m_dx] == 1'b1) on_mouse = 1;
        end

        // 2. 중앙 파란 박스 (락온 모드일 때만)
        if (is_locked) begin
            // X: 256+32 ~ 383-32 => 288 ~ 351
            // Y: 192+16 ~ 287-16 => 208 ~ 271
            if ( ((y_pixel == 208 || y_pixel == 271) && (x_pixel >= 288 && x_pixel <= 351)) ||
                 ((x_pixel == 288 || x_pixel == 351) && (y_pixel >= 208 && y_pixel <= 271)) ) begin
                on_center_box = 1;
            end
        end

        // 3. 타겟 그리기 (16개 루프)
        for (int k = 0; k < 16; k = k + 1) begin
            // 조건: 감지됨 AND (락온이 꺼져있거나, 락온이 켜져있으면 현재 인덱스가 타겟이어야 함)
            if (aim_detected_all[k] && (!is_locked || (is_locked && k == locked_idx))) begin

                // 박스
                if ( ((y_pixel == box_y_min_all[k] || y_pixel == box_y_max_all[k]) && (x_pixel >= box_x_min_all[k] && x_pixel <= box_x_max_all[k])) ||
                     ((x_pixel == box_x_min_all[k] || x_pixel == box_x_max_all[k]) && (y_pixel >= box_y_min_all[k] && y_pixel <= box_y_max_all[k])) ) begin
                    on_box = 1;
                end

                // 에임
                aim_x_int = aim_x_all[k];
                aim_y_int = aim_y_all[k];
                if ( (y_pix_int >= aim_y_int - AIM_THK) && (y_pix_int <= aim_y_int + AIM_THK) && 
                     (x_pix_int >= aim_x_int - AIM_LEN) && (x_pix_int <= aim_x_int + AIM_LEN) )
                    on_aim = 1;
                if ( (x_pix_int >= aim_x_int - AIM_THK) && (x_pix_int <= aim_x_int + AIM_THK) && 
                     (y_pix_int >= aim_y_int - AIM_LEN) && (y_pix_int <= aim_y_int + AIM_LEN) )
                    on_aim = 1;
            end
        end
    end

    // --- 최종 출력 (우선순위 적용) ---
    always_comb begin
        logic [11:0] pixel_color;

        if (on_mouse) begin
            pixel_color = (click_l || click_r) ? CLICK_COLOR : MOUSE_COLOR;
        end else if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
        end else if (on_aim) begin
            pixel_color = aim_color_curr; // 락온&중앙이면 흰색, 아니면 빨강
        end else if (on_center_box) begin
            pixel_color = CENTER_BOX_COLOR;
        end else if (on_box) begin
            pixel_color = BOX_COLOR;
        end else begin
            pixel_color = img_bg;
        end

        {r_port, g_port, b_port} = pixel_color;
    end

endmodule
