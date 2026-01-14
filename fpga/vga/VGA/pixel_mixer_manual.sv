


`timescale 1ns / 1ps

module pixel_mixer_manual (
    input logic [11:0] img_bg,

    // 타겟 정보
    input logic [15:0][9:0] aim_x_all,
    input logic [15:0][9:0] aim_y_all,
    input logic [15:0]      aim_detected_all,

    input logic [9:0] x_pixel,
    input logic [9:0] y_pixel,

    // 박스 정보
    input logic [15:0][11:0] box_x_min_all,
    input logic [15:0][11:0] box_x_max_all,
    input logic [15:0][11:0] box_y_min_all,
    input logic [15:0][11:0] box_y_max_all,

    // 키보드 데이터 (W:0, A:1, S:2, D:3, L:4)
    input logic [7:0] keyboard_data,

    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,

    output logic [9:0] x_coor,
    output logic [9:0] y_coor,
    output logic       shoot
);
    // --- 색상 정의 ---
    localparam logic [11:0] RED = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0;
    localparam logic [11:0] YELLOW = 12'hFF0;
    localparam logic [11:0] WHITE = 12'hFFF;
    localparam logic [11:0] BLACK = 12'h000;

    localparam logic [11:0] AIM_COLOR = RED;
    localparam logic [11:0] BOX_COLOR = GREEN;

    // =================================================================
    // 1. 비트맵 정의 (키보드 UI용)
    // =================================================================
    localparam logic [0:7][7:0] CHAR_W = '{
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11011011,
        8'b11011011,
        8'b01100110,
        8'b00000000
    };
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
    localparam logic [0:7][7:0] CHAR_S = '{
        8'b01111110,
        8'b11000011,
        8'b11000000,
        8'b01111110,
        8'b00000011,
        8'b00000011,
        8'b11000011,
        8'b01111110
    };
    localparam logic [0:7][7:0] CHAR_D = '{
        8'b11111100,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b11111100,
        8'b00000000
    };
    localparam logic [0:7][7:0] CHAR_L = '{
        8'b11000000,
        8'b11000000,
        8'b11000000,
        8'b11000000,
        8'b11000000,
        8'b11000000,
        8'b11111111,
        8'b11111111
    };

    // 조준 완료 아이콘 (과녁 모양)
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

    // =================================================================
    // 2. UI 위치 파라미터
    // =================================================================
    localparam int KEY_SIZE = 30;
    localparam int GAP = 5;
    localparam int CHAR_OFFSET = (KEY_SIZE - 16) / 2;

    // WASD 좌표
    localparam int WASD_X = 30;
    localparam int WASD_Y = 380;
    localparam int AX1 = WASD_X, AY1 = WASD_Y + KEY_SIZE + GAP;
    localparam int SX1 = WASD_X + KEY_SIZE + GAP, SY1 = AY1;
    localparam int DX1 = WASD_X + 2 * (KEY_SIZE + GAP), DY1 = AY1;
    localparam int WX1 = SX1, WY1 = WASD_Y;

    // L키 & 락온 UI 좌표
    localparam int L_X = 30, L_Y = 30;
    localparam int LOCK_UI_X = 580, LOCK_UI_Y = 30;

    // 중앙 조준선 파라미터
    localparam int CX = 320;
    localparam int CY = 240;
    localparam int LOCK_ZONE = 30;  // 락온 인식 범위

    // 내부 변수
    logic on_box, on_aim;
    logic is_wasd_ui, is_wasd_char;
    logic [11:0] wasd_color;
    logic is_l_ui, is_l_char;
    logic [11:0] l_color;
    logic on_crosshair;
    logic is_locked_on;
    logic is_lock_ui, is_lock_icon;

    logic [5:0] rel_x, rel_y;
    logic [2:0] char_x, char_y;

    // 조준선 그리기를 위한 절대거리 변수
    int dist_x, dist_y;

    // assign y_coor = keyboard_data[0] ? 10'd190 : keyboard_data[2] ? 10'd290 : 10'd240;
    // assign x_coor = keyboard_data[1] ? 10'd270 : keyboard_data[3] ? 10'd370 : 10'd320;
    // assign shoot  = keyboard_data[4] ? 1'b1 : 1'b0;

    // =================================================================
    // 3. 로직 구현
    // =================================================================

    // (1) 락온 감지 (빨간 물체가 중앙 범위 내에 있는가?)
    always_comb begin
        is_locked_on = 0;
        for (int k = 0; k < 16; k++) begin
            if (aim_detected_all[k]) begin
                if( (aim_x_all[k] > CX - LOCK_ZONE) && (aim_x_all[k] < CX + LOCK_ZONE) &&
                    (aim_y_all[k] > CY - LOCK_ZONE) && (aim_y_all[k] < CY + LOCK_ZONE) ) begin
                    is_locked_on = 1;
                end
            end
        end
    end

    // miso signal
    always_comb begin
        x_coor = 10'd320;
        y_coor = 10'd240;
        shoot  = 1'b0;
        if(keyboard_data[0]) y_coor = 10'd190;
        else if(keyboard_data[2]) y_coor = 10'd290;
        else if(keyboard_data[1]) x_coor = 10'd270;
        else if(keyboard_data[3]) x_coor = 10'd320;
        else if(keyboard_data[4]) shoot  = 1'b1; 
    end

    // (2) 화면 그리기 로직
    always_comb begin
        // 초기화
        on_box       = 0;
        on_aim       = 0;
        on_crosshair = 0;
        is_wasd_ui   = 0;
        is_wasd_char = 0;
        wasd_color   = WHITE;
        is_l_ui      = 0;
        is_l_char    = 0;
        l_color      = WHITE;
        is_lock_ui   = 0;
        is_lock_icon = 0;
        rel_x        = 0;
        rel_y        = 0;
        char_x       = 0;
        char_y       = 0;


        // --- A. 중앙 조준선 (이미지 스타일 구현) ---
        dist_x       = (x_pixel > CX) ? (x_pixel - CX) : (CX - x_pixel);
        dist_y       = (y_pixel > CY) ? (y_pixel - CY) : (CY - y_pixel);

        // 1. 중앙 원
        if ((dist_x * dist_x + dist_y * dist_y) <= 36) on_crosshair = 1;

        // 2. 내부 십자선
        if (dist_x < 2 && dist_y >= 12 && dist_y <= 22) on_crosshair = 1;
        if (dist_y < 2 && dist_x >= 12 && dist_x <= 22) on_crosshair = 1;

        // 3. 외부 코너 브라켓
        if (dist_y >= 33 && dist_y <= 35 && dist_x >= 20 && dist_x <= 35)
            on_crosshair = 1;
        if (dist_x >= 33 && dist_x <= 35 && dist_y >= 20 && dist_y <= 35)
            on_crosshair = 1;


        // --- B. 기존 타겟 박스 ---
        for (int k = 0; k < 16; k++) begin
            if (aim_detected_all[k]) begin
                if ( ((y_pixel == box_y_min_all[k] || y_pixel == box_y_max_all[k]) && (x_pixel >= box_x_min_all[k] && x_pixel <= box_x_max_all[k])) ||
                     ((x_pixel == box_x_min_all[k] || x_pixel == box_x_max_all[k]) && (y_pixel >= box_y_min_all[k] && y_pixel <= box_y_max_all[k])) )
                    on_box = 1;
                if ( (y_pixel >= aim_y_all[k]-1 && y_pixel <= aim_y_all[k]+1 && x_pixel >= aim_x_all[k]-5 && x_pixel <= aim_x_all[k]+5) ||
                     (x_pixel >= aim_x_all[k]-1 && x_pixel <= aim_x_all[k]+1 && y_pixel >= aim_y_all[k]-5 && y_pixel <= aim_y_all[k]+5) )
                    on_aim = 1;
            end
        end
        // keyboard_data[0] = W 
        // keyboard_data[1] = A
        // keyboard_data[2] = S 
        // keyboard_data[3] = D 
        // keyboard_data[4] = L
        // keyboard_data[5] = F1
        // keyboard_data[6] = F2
        // keyboard_data[7] = F3
        // --- C. WASD UI ---
        if (x_pixel >= WX1 && x_pixel < WX1 + KEY_SIZE && y_pixel >= WY1 && y_pixel < WY1 + KEY_SIZE) begin
            is_wasd_ui = 1;
            wasd_color = keyboard_data[0] ? YELLOW : WHITE;
            rel_x = x_pixel - WX1;
            rel_y = y_pixel - WY1;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (CHAR_W[char_y][7-char_x]) is_wasd_char = 1;
            end
        end else if (x_pixel >= AX1 && x_pixel < AX1 + KEY_SIZE && y_pixel >= AY1 && y_pixel < AY1 + KEY_SIZE) begin
            is_wasd_ui = 1;
            wasd_color = keyboard_data[1] ? YELLOW : WHITE;
            rel_x = x_pixel - AX1;
            rel_y = y_pixel - AY1;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (CHAR_A[char_y][7-char_x]) is_wasd_char = 1;
            end
        end else if (x_pixel >= SX1 && x_pixel < SX1 + KEY_SIZE && y_pixel >= SY1 && y_pixel < SY1 + KEY_SIZE) begin
            is_wasd_ui = 1;
            wasd_color = keyboard_data[2] ? YELLOW : WHITE;
            rel_x = x_pixel - SX1;
            rel_y = y_pixel - SY1;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (CHAR_S[char_y][7-char_x]) is_wasd_char = 1;
            end
        end else if (x_pixel >= DX1 && x_pixel < DX1 + KEY_SIZE && y_pixel >= DY1 && y_pixel < DY1 + KEY_SIZE) begin
            is_wasd_ui = 1;
            wasd_color = keyboard_data[3] ? YELLOW : WHITE;
            rel_x = x_pixel - DX1;
            rel_y = y_pixel - DY1;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (CHAR_D[char_y][7-char_x]) is_wasd_char = 1;
            end
        end

        // --- D. L-Key UI ---
        if (x_pixel >= L_X && x_pixel < L_X + KEY_SIZE && y_pixel >= L_Y && y_pixel < L_Y + KEY_SIZE) begin
            is_l_ui = 1;
            l_color = keyboard_data[4] ? YELLOW : WHITE;
            rel_x   = x_pixel - L_X;
            rel_y   = y_pixel - L_Y;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (CHAR_L[char_y][7-char_x]) is_l_char = 1;
            end
        end

        // --- E. 락온 알림 UI (상시 표시로 변경) ---
        // is_locked_on 조건 제거하여 항상 좌표에 진입하도록 수정
        if (x_pixel >= LOCK_UI_X && x_pixel < LOCK_UI_X + KEY_SIZE && y_pixel >= LOCK_UI_Y && y_pixel < LOCK_UI_Y + KEY_SIZE) begin
            is_lock_ui = 1;
            rel_x = x_pixel - LOCK_UI_X;
            rel_y = y_pixel - LOCK_UI_Y;
            if (rel_x>=CHAR_OFFSET && rel_x<CHAR_OFFSET+16 && rel_y>=CHAR_OFFSET && rel_y<CHAR_OFFSET+16) begin
                char_x = (rel_x - CHAR_OFFSET) >> 1;
                char_y = (rel_y - CHAR_OFFSET) >> 1;
                if (ICON_LOCK[char_y][7-char_x]) is_lock_icon = 1;
            end
        end
    end

    // =================================================================
    // 4. 최종 색상 결정
    // =================================================================
    always_comb begin
        logic [11:0] pixel_color;

        if (is_lock_ui) begin
            if (is_lock_icon) begin
                pixel_color = BLACK;  // 아이콘은 항상 검은색
            end else begin
                // [중요] 조준되면 노랑색, 아니면 하얀색 배경
                pixel_color = is_locked_on ? YELLOW : WHITE;
            end
        end else if (is_wasd_ui) begin
            pixel_color = is_wasd_char ? BLACK : wasd_color;
        end else if (is_l_ui) begin
            pixel_color = is_l_char ? BLACK : l_color;
        end else if (on_crosshair) begin
            pixel_color = BLACK;  // 중앙 조준선
        end else if (on_aim) begin
            pixel_color = AIM_COLOR;
        end else if (on_box) begin
            pixel_color = BOX_COLOR;
        end else begin
            pixel_color = img_bg;
        end

        {r_port, g_port, b_port} = pixel_color;
    end

endmodule
