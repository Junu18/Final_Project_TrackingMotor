
`timescale 1ns / 1ps

module pixel_mixer_auto (
    // [중요] 시스템 클럭 추가
    input logic clk,

    input logic [11:0] img_bg,
    input logic [ 9:0] aim_x,
    input logic [ 9:0] aim_y,
    input logic        aim_detected,
    input logic [ 9:0] x_pixel,
    input logic [ 9:0] y_pixel,
    input logic [11:0] box_x_min,
    input logic [11:0] box_x_max,
    input logic [11:0] box_y_min,
    input logic [11:0] box_y_max,
    input logic        vir_signal,

    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,

    output logic shoot
);
    // --- 색상 및 파라미터 (기존 동일) ---
    localparam logic [11:0] RED = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0;
    localparam logic [11:0] WHITE = 12'hFFF;
    localparam logic [11:0] BLACK = 12'h000;
    localparam logic [11:0] YELLOW = 12'hFF0;

    localparam logic [11:0] AIM_COLOR = RED;
    localparam logic [11:0] TEXT_COLOR = GREEN;
    localparam logic [11:0] BOX_COLOR = GREEN;

    localparam THK = 1, LEN = 10;
    localparam int CX = 320, CY = 240, LOCK_ZONE = 30;
    localparam int KEY_SIZE = 30, CHAR_OFFSET = (KEY_SIZE - 16) / 2;
    localparam int LOCK_UI_X = 580, LOCK_UI_Y = 30;

    // 비트맵 (기존 동일)
    localparam logic [0:7][7:0] CHAR_A = '{
        8'b00111100,
        8'b01100110,
        8'b11000011,
        8'b11111111,
        8'b11111111,
        8'b11000011,
        8'b11000011,
        8'b00000000
    };
    localparam logic [0:7][7:0] CHAR_U = '{
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b01111110,
        8'b00000000
    };
    localparam logic [0:7][7:0] CHAR_T = '{
        8'b11111111,
        8'b00011000,
        8'b00011000,
        8'b00011000,
        8'b00011000,
        8'b00011000,
        8'b00011000,
        8'b00000000
    };
    localparam logic [0:7][7:0] CHAR_O = '{
        8'b00111100,
        8'b01100110,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b01100110,
        8'b00111100
    };
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

    // AUTO 박스 파라미터
    localparam int AUTO_BOX_W = 100, AUTO_BOX_H = 26;
    localparam int AUTO_BOX_X = CX - (AUTO_BOX_W / 2), AUTO_BOX_Y = 10;
    localparam int AUTO_BORDER_THICK = 2, AUTO_CHAR_STEP = 18;
    localparam int AUTO_TEXT_START_X = (AUTO_BOX_W - (4 * AUTO_CHAR_STEP - 2)) / 2;
    localparam int AUTO_TEXT_START_Y = (AUTO_BOX_H - 16) / 2;

    // 내부 신호
    logic is_locked_on;
    logic on_crosshair;
    logic is_lock_ui, is_lock_icon;
    logic is_auto_box, is_auto_border, is_auto_text;
    logic shoot_next;
    int rel_x, rel_y, char_x, char_y, dist_x, dist_y;

    // [파이프라인] 임시 색상 변수
    logic [3:0] r_next, g_next, b_next;

    // --- 좌표 숫자 그리기 (함수 등 기존 동일) ---
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

    logic is_text_pixel;
    logic [3:0] x_100, x_10, x_1, y_100, y_10, y_1;
    assign x_100 = aim_x / 100;
    assign x_10  = (aim_x / 10) % 10;
    assign x_1   = aim_x % 10;
    assign y_100 = aim_y / 100;
    assign y_10  = (aim_y / 10) % 10;
    assign y_1   = aim_y % 10;

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
        shoot_next = 0;
        is_auto_box = 0;
        is_auto_border = 0;
        is_auto_text = 0;
        rel_x = 0;
        rel_y = 0;
        char_x = 0;
        char_y = 0;

        if (aim_detected) begin
            if ( (aim_x > CX - LOCK_ZONE) && (aim_x < CX + LOCK_ZONE) &&
                 (aim_y > CY - LOCK_ZONE) && (aim_y < CY + LOCK_ZONE) ) begin
                is_locked_on = 1;
                shoot_next = 1;
            end
        end

        dist_x = (x_pixel > CX) ? (x_pixel - CX) : (CX - x_pixel);
        dist_y = (y_pixel > CY) ? (y_pixel - CY) : (CY - y_pixel);
        if ((dist_x * dist_x + dist_y * dist_y) <= 36) on_crosshair = 1;
        if (dist_x < 2 && dist_y >= 12 && dist_y <= 22) on_crosshair = 1;
        if (dist_y < 2 && dist_x >= 12 && dist_x <= 22) on_crosshair = 1;
        if (dist_y >= 33 && dist_y <= 35 && dist_x >= 20 && dist_x <= 35)
            on_crosshair = 1;
        if (dist_x >= 33 && dist_x <= 35 && dist_y >= 20 && dist_y <= 35)
            on_crosshair = 1;

        if (x_pixel >= LOCK_UI_X && x_pixel < LOCK_UI_X + KEY_SIZE && y_pixel >= LOCK_UI_Y && y_pixel < LOCK_UI_Y + KEY_SIZE) begin
            is_lock_ui = 1;
            rel_x = x_pixel - LOCK_UI_X;
            rel_y = y_pixel - LOCK_UI_Y;
            if (rel_x >= CHAR_OFFSET && rel_x < CHAR_OFFSET + 16 && rel_y >= CHAR_OFFSET && rel_y < CHAR_OFFSET + 16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (ICON_LOCK[char_y][7-char_x]) is_lock_icon = 1;
            end
        end

        if (x_pixel >= AUTO_BOX_X && x_pixel < AUTO_BOX_X + AUTO_BOX_W &&
            y_pixel >= AUTO_BOX_Y && y_pixel < AUTO_BOX_Y + AUTO_BOX_H) begin
            is_auto_box = 1;
            rel_x = x_pixel - AUTO_BOX_X;
            rel_y = y_pixel - AUTO_BOX_Y;
            if (rel_x < AUTO_BORDER_THICK || rel_x >= AUTO_BOX_W - AUTO_BORDER_THICK ||
                rel_y < AUTO_BORDER_THICK || rel_y >= AUTO_BOX_H - AUTO_BORDER_THICK) begin
                is_auto_border = 1;
            end else begin
                int text_rel_x, text_rel_y;
                logic [2:0] char_idx;
                text_rel_y = rel_y - AUTO_TEXT_START_Y;
                if (text_rel_y >= 0 && text_rel_y < 16) begin
                    text_rel_x = rel_x - AUTO_TEXT_START_X;
                    if (text_rel_x >= 0 && text_rel_x < (4 * AUTO_CHAR_STEP - 2)) begin
                        char_idx = text_rel_x / AUTO_CHAR_STEP;
                        char_x   = (text_rel_x % AUTO_CHAR_STEP) >> 1;
                        char_y   = text_rel_y >> 1;
                        if (char_x < 8) begin
                            case (char_idx)
                                0:
                                if (CHAR_A[char_y][7-char_x]) is_auto_text = 1;
                                1:
                                if (CHAR_U[char_y][7-char_x]) is_auto_text = 1;
                                2:
                                if (CHAR_T[char_y][7-char_x]) is_auto_text = 1;
                                3:
                                if (CHAR_O[char_y][7-char_x]) is_auto_text = 1;
                            endcase
                        end
                    end
                end
            end
        end
    end

    // =================================================================
    // 3. [파이프라인] 색상 결정 및 레지스터 출력
    // =================================================================
    always_comb begin
        logic [11:0] pixel_color;

        if (is_lock_ui) begin
            if (is_lock_icon) pixel_color = BLACK;
            else pixel_color = is_locked_on ? YELLOW : WHITE;
        end else if (is_auto_box) begin
            if (is_auto_border || is_auto_text) pixel_color = BLACK;
            else pixel_color = vir_signal ? YELLOW : WHITE;
        end else if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
        end else if (on_crosshair) begin
            pixel_color = BLACK;
        end else if (aim_detected && 
                 ( ((y_pixel >= aim_y - THK) && (y_pixel <= aim_y + THK) && (x_pixel >= aim_x - LEN) && (x_pixel <= aim_x + LEN)) ||
                   ((x_pixel >= aim_x - THK) && (x_pixel <= aim_x + THK) && (y_pixel >= aim_y - LEN) && (y_pixel <= aim_y + LEN)) )) begin
            pixel_color = AIM_COLOR;
        end else if (aim_detected && 
                 ( ((y_pixel == box_y_min || y_pixel == box_y_max) && (x_pixel >= box_x_min && x_pixel <= box_x_max)) ||
                   ((x_pixel == box_x_min || x_pixel == box_x_max) && (y_pixel >= box_y_min && y_pixel <= box_y_max)) )) begin
            pixel_color = BOX_COLOR;
        end else begin
            pixel_color = img_bg;
        end

        // 임시 변수에 저장
        {r_next, g_next, b_next} = pixel_color;
    end

    // [핵심] 출력 레지스터
    always_ff @(posedge clk) begin
        r_port <= r_next;
        g_port <= g_next;
        b_port <= b_next;
        shoot  <= shoot_next;
    end

endmodule
