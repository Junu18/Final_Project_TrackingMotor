
`timescale 1ns / 1ps

module pixel_mixer_manual (
    // [중요] 시스템 클럭
    input logic clk,

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

    // 키보드 데이터
    input logic [7:0] keyboard_data,

    // MOSI (모터 데이터)
    input logic [7:0] mortor_xdata,  // 0~180 (Center 90)
    input logic [6:0] mortor_ydata,  // 0~90  (Center 45)

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
    localparam logic [11:0] GRAY = 12'h888;  // [추가] 바 배경색
    localparam logic [11:0] BLUE = 12'h00F;  // [추가] 인디케이터 색상

    localparam logic [11:0] AIM_COLOR = RED;
    localparam logic [11:0] BOX_COLOR = GREEN;

    // =================================================================
    // 1. 비트맵 정의
    // =================================================================
    // 기존 문자들
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

    // MANUAL 박스용 문자
    localparam logic [0:7][7:0] CHAR_M = '{
        8'b11000011,
        8'b11100111,
        8'b11111111,
        8'b11011011,
        8'b11000011,
        8'b11000011,
        8'b11000011,
        8'b00000000
    };
    localparam logic [0:7][7:0] CHAR_N = '{
        8'b11000011,
        8'b11100011,
        8'b11110011,
        8'b11011011,
        8'b11001111,
        8'b11000111,
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

    // [추가] STOP 박스용 문자 (T, O, P) - S는 기존 CHAR_S 사용
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
    localparam logic [0:7][7:0] CHAR_P = '{
        8'b11111100,
        8'b11000011,
        8'b11000011,
        8'b11111100,
        8'b11000000,
        8'b11000000,
        8'b11000000,
        8'b00000000
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

    // =================================================================
    // 2. UI 위치 파라미터
    // =================================================================
    localparam int KEY_SIZE = 30;
    localparam int GAP = 5;
    localparam int CHAR_OFFSET = (KEY_SIZE - 16) / 2;
    localparam int CX = 320;
    localparam int CY = 240;
    localparam int LOCK_ZONE = 30;

    localparam int WASD_X = 30, WASD_Y = 380;
    localparam int AX1 = WASD_X, AY1 = WASD_Y + KEY_SIZE + GAP;
    localparam int SX1 = WASD_X + KEY_SIZE + GAP, SY1 = AY1;
    localparam int DX1 = WASD_X + 2 * (KEY_SIZE + GAP), DY1 = AY1;
    localparam int WX1 = SX1, WY1 = WASD_Y;
    localparam int L_X = 30, L_Y = 30;
    localparam int LOCK_UI_X = 580, LOCK_UI_Y = 30;

    // MANUAL 박스 파라미터 (상단 중앙)
    localparam int MAN_BOX_W = 130;
    localparam int MAN_BOX_H = 26;
    localparam int MAN_BOX_X = CX - (MAN_BOX_W / 2);
    localparam int MAN_BOX_Y = 10;
    localparam int MAN_BORDER_THICK = 2;
    localparam int MAN_CHAR_STEP = 18;
    localparam int MAN_TEXT_START_X = (MAN_BOX_W - (6 * MAN_CHAR_STEP - 2)) / 2;
    localparam int MAN_TEXT_START_Y = (MAN_BOX_H - 16) / 2;

    // [수정] 모터 수평 바 (Horizontal Bar) - 하단 중앙
    // 0~180 범위를 표현하기 위해 너비를 180px로 설정
    localparam int H_BAR_W = 180;
    localparam int H_BAR_H = 10;
    localparam int H_BAR_X = CX - (H_BAR_W / 2);  // 320 - 90 = 230
    localparam int H_BAR_Y = 460;  // 화면 하단 (480)에서 약간 위
    localparam int H_IND_W = 4;  // 움직이는 막대 너비

    // [수정] 모터 수직 바 (Vertical Bar) - 우측
    // 0~90 범위를 2배 확대하여 180px 높이로 표현 (잘 보이게)
    localparam int V_BAR_W = 10;
    localparam int V_BAR_H = 180;
    localparam int V_BAR_X = 620;  // 화면 우측 (640)에서 약간 왼쪽
    localparam int V_BAR_Y = CY - (V_BAR_H / 2);  // 240 - 90 = 150
    localparam int V_IND_H = 4;  // 움직이는 막대 높이

    // [추가] STOP 박스 파라미터 (우측 하단)
    localparam int STOP_BOX_W = 100; // 4글자 정도 너비
    localparam int STOP_BOX_H = 26;
    localparam int STOP_BOX_X = 640 - STOP_BOX_W - 20; // 우측 여백 20
    localparam int STOP_BOX_Y = 480 - STOP_BOX_H - 20; // 하단 여백 20
    localparam int STOP_TEXT_START_X = (STOP_BOX_W - (4 * MAN_CHAR_STEP - 2)) / 2;
    localparam int STOP_TEXT_START_Y = (STOP_BOX_H - 16) / 2;

    // 내부 변수
    logic on_box, on_aim;
    logic is_wasd_ui, is_wasd_char;
    logic [11:0] wasd_color;
    logic is_l_ui, is_l_char;
    logic [11:0] l_color;
    logic on_crosshair;
    logic is_locked_on;
    logic is_lock_ui, is_lock_icon;
    logic is_man_box, is_man_border, is_man_text;

    // [수정] 새로운 바 관련 신호
    logic is_h_bar_bg, is_h_bar_ind;  // 수평 바 배경, 인디케이터
    logic is_v_bar_bg, is_v_bar_ind;  // 수직 바 배경, 인디케이터

    // [추가] STOP 박스 관련 신호
    logic is_stop_box, is_stop_border, is_stop_text;
    logic stop_triggered;  // 모터 제한값 도달 시 1

    int rel_x, rel_y;
    int char_x, char_y;
    int dist_x, dist_y;

    // [파이프라인] 임시 저장 변수
    logic [3:0] r_next, g_next, b_next;

    assign y_coor = keyboard_data[0] ? 10'd150 : keyboard_data[2] ? 10'd330 : 10'd240;
    assign x_coor = keyboard_data[1] ? 10'd210 : keyboard_data[3] ? 10'd410 : 10'd320;
    assign shoot = keyboard_data[4] ? 1'b1 : 1'b0;
    assign stop_triggered = (mortor_xdata == 0) || (mortor_xdata == 8'd180) || (mortor_ydata == 7'd90) || (mortor_ydata == 0);

    // [삭제] stop_triggered 로직 제거

    // (1) 락온 감지 로직
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

    // (2) 화면 그리기 로직 (Combinational Logic)
    always_comb begin
        // 초기화
        on_box        = 0;
        on_aim        = 0;
        on_crosshair  = 0;
        is_wasd_ui    = 0;
        is_wasd_char  = 0;
        wasd_color    = WHITE;
        is_l_ui       = 0;
        is_l_char     = 0;
        l_color       = WHITE;
        is_lock_ui    = 0;
        is_lock_icon  = 0;
        is_man_box    = 0;
        is_man_border = 0;
        is_man_text   = 0;

        // [수정] 새로운 바 초기화
        is_h_bar_bg   = 0;
        is_h_bar_ind  = 0;
        is_v_bar_bg   = 0;
        is_v_bar_ind  = 0;

        is_stop_box    = 0;
        is_stop_border = 0;
        is_stop_text   = 0;

        rel_x         = 0;
        rel_y         = 0;
        char_x        = 0;
        char_y        = 0;

        // --- A. 중앙 조준선 ---
        dist_x        = (x_pixel > CX) ? (x_pixel - CX) : (CX - x_pixel);
        dist_y        = (y_pixel > CY) ? (y_pixel - CY) : (CY - y_pixel);
        if ((dist_x * dist_x + dist_y * dist_y) <= 36) on_crosshair = 1;
        if (dist_x < 2 && dist_y >= 12 && dist_y <= 22) on_crosshair = 1;
        if (dist_y < 2 && dist_x >= 12 && dist_x <= 22) on_crosshair = 1;
        if (dist_y >= 33 && dist_y <= 35 && dist_x >= 20 && dist_x <= 35)
            on_crosshair = 1;
        if (dist_x >= 33 && dist_x <= 35 && dist_y >= 20 && dist_y <= 35)
            on_crosshair = 1;

        // --- B. 타겟 박스 ---
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

        // --- E. 락온 UI ---
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

        // --- F. MANUAL 박스 UI ---
        if (x_pixel >= MAN_BOX_X && x_pixel < MAN_BOX_X + MAN_BOX_W && y_pixel >= MAN_BOX_Y && y_pixel < MAN_BOX_Y + MAN_BOX_H) begin
            is_man_box = 1;
            rel_x = x_pixel - MAN_BOX_X;
            rel_y = y_pixel - MAN_BOX_Y;
            if (rel_x < MAN_BORDER_THICK || rel_x >= MAN_BOX_W - MAN_BORDER_THICK ||
                rel_y < MAN_BORDER_THICK || rel_y >= MAN_BOX_H - MAN_BORDER_THICK) begin
                is_man_border = 1;
            end else begin
                int text_rel_x, text_rel_y;
                logic [2:0] char_idx;
                text_rel_y = rel_y - MAN_TEXT_START_Y;
                if (text_rel_y >= 0 && text_rel_y < 16) begin
                    text_rel_x = rel_x - MAN_TEXT_START_X;
                    if (text_rel_x >= 0 && text_rel_x < (6 * MAN_CHAR_STEP - 2)) begin
                        char_idx = text_rel_x / MAN_CHAR_STEP;
                        char_x   = (text_rel_x % MAN_CHAR_STEP) >> 1;
                        char_y   = text_rel_y >> 1;
                        if (char_x < 8) begin
                            case (char_idx)
                                0:
                                if (CHAR_M[char_y][7-char_x]) is_man_text = 1;
                                1:
                                if (CHAR_A[char_y][7-char_x]) is_man_text = 1;
                                2:
                                if (CHAR_N[char_y][7-char_x]) is_man_text = 1;
                                3:
                                if (CHAR_U[char_y][7-char_x]) is_man_text = 1;
                                4:
                                if (CHAR_A[char_y][7-char_x]) is_man_text = 1;
                                5:
                                if (CHAR_L[char_y][7-char_x]) is_man_text = 1;
                            endcase
                        end
                    end
                end
            end
        end

        // --- G. [변경] 수평 바 (Horizontal Bar) : mortor_xdata ---
        // 하단 중앙 위치, mortor_xdata(0~180)에 따라 인디케이터 좌우 이동
        if (x_pixel >= H_BAR_X && x_pixel < H_BAR_X + H_BAR_W && 
            y_pixel >= H_BAR_Y && y_pixel < H_BAR_Y + H_BAR_H) begin

            int current_ind_x;
            is_h_bar_bg   = 1;  // 기본 배경 ON

            // 인디케이터 위치 계산: 바 시작점 + 모터값 (1:1 매핑)
            // mortor_xdata=0 -> 왼쪽 끝, 90 -> 중앙, 180 -> 오른쪽 끝
            current_ind_x = H_BAR_X + mortor_xdata;

            // 인디케이터 그리기 (약간의 두께 H_IND_W)
            if (x_pixel >= current_ind_x - (H_IND_W/2) && x_pixel < current_ind_x + (H_IND_W/2)) begin
                is_h_bar_ind = 1;
            end
        end

        // --- G. [추가] STOP 알림 박스 (우측 하단) ---
        if (x_pixel >= STOP_BOX_X && x_pixel < STOP_BOX_X + STOP_BOX_W && 
            y_pixel >= STOP_BOX_Y && y_pixel < STOP_BOX_Y + STOP_BOX_H) begin

            is_stop_box = 1; 
            rel_x = x_pixel - STOP_BOX_X; 
            rel_y = y_pixel - STOP_BOX_Y;

            // 테두리
            if (rel_x < MAN_BORDER_THICK || rel_x >= STOP_BOX_W - MAN_BORDER_THICK ||
                rel_y < MAN_BORDER_THICK || rel_y >= STOP_BOX_H - MAN_BORDER_THICK) begin
                is_stop_border = 1;
            end else begin
                // 글자: STOP
                int text_rel_x, text_rel_y; logic [2:0] char_idx;
                text_rel_y = rel_y - STOP_TEXT_START_Y;

                if (text_rel_y >= 0 && text_rel_y < 16) begin
                    text_rel_x = rel_x - STOP_TEXT_START_X;
                    // 4글자 * 18px - 2px
                    if (text_rel_x >= 0 && text_rel_x < (4 * MAN_CHAR_STEP - 2)) begin
                        char_idx = text_rel_x / MAN_CHAR_STEP;
                        char_x = (text_rel_x % MAN_CHAR_STEP) >> 1;
                        char_y = text_rel_y >> 1;
                        if (char_x < 8) begin
                            case (char_idx)
                                0: if (CHAR_S[char_y][7-char_x]) is_stop_text = 1; // S
                                1: if (CHAR_T[char_y][7-char_x]) is_stop_text = 1; // T
                                2: if (CHAR_O[char_y][7-char_x]) is_stop_text = 1; // O
                                3: if (CHAR_P[char_y][7-char_x]) is_stop_text = 1; // P
                            endcase
                        end
                    end
                end
            end
        end

        // --- H. [변경] 수직 바 (Vertical Bar) : mortor_ydata ---
        // 우측 위치, mortor_ydata(0~90)에 따라 인디케이터 상하 이동
        // 값 증가 시 위쪽(Y좌표 감소), 값 감소 시 아래쪽(Y좌표 증가)
        if (x_pixel >= V_BAR_X && x_pixel < V_BAR_X + V_BAR_W &&
            y_pixel >= V_BAR_Y && y_pixel < V_BAR_Y + V_BAR_H) begin

            int current_ind_y;
            is_v_bar_bg   = 1;

            // 인디케이터 위치 계산
            // Center(45) -> 화면상 V_BAR_Y + (V_BAR_H/2) = 150 + 90 = 240
            // 식: 330 - 2 * mortor_ydata
            // 예: ydata=45 -> 330 - 90 = 240 (중앙)
            // 예: ydata=90 -> 330 - 180 = 150 (상단, 바 시작점)
            // 예: ydata=0  -> 330 - 0 = 330 (하단, 바 끝점)
            current_ind_y = 330 - (mortor_ydata * 2);

            if (y_pixel >= current_ind_y - (V_IND_H/2) && y_pixel < current_ind_y + (V_IND_H/2)) begin
                is_v_bar_ind = 1;
            end
        end

    end

    // =================================================================
    // 3. [파이프라인] 색상 결정 및 레지스터 출력
    // =================================================================
    always_comb begin
        logic [11:0] pixel_color;
        // 우선순위 결정
        if (is_lock_ui) begin
            if (is_lock_icon) pixel_color = BLACK;
            else pixel_color = is_locked_on ? YELLOW : WHITE;

        end else if (is_stop_box) begin // STOP 박스 우선순위 (경고이므로 높게 설정)
            if (is_stop_border || is_stop_text) pixel_color = BLACK;
            else pixel_color = stop_triggered ? YELLOW : WHITE;
        end else if (is_h_bar_ind) begin
            pixel_color = RED;  // 수평 인디케이터 색상
        end else if (is_v_bar_ind) begin
            pixel_color = RED;  // 수직 인디케이터 색상
        end else if (is_h_bar_bg || is_v_bar_bg) begin
            pixel_color = WHITE;  // 바 배경 색상

        end else if (is_man_box) begin
            if (is_man_border || is_man_text) pixel_color = BLACK;
            else pixel_color = WHITE;
        end else if (is_wasd_ui) begin
            pixel_color = is_wasd_char ? BLACK : wasd_color;
        end else if (is_l_ui) begin
            pixel_color = is_l_char ? BLACK : l_color;
        end else if (on_crosshair) begin
            pixel_color = BLACK;
        end else if (on_aim) begin
            pixel_color = AIM_COLOR;
        end else if (on_box) begin
            pixel_color = BOX_COLOR;
        end else begin
            pixel_color = img_bg;
        end

        // 바로 출력(output)하지 않고 next 변수에 담습니다.
        {r_next, g_next, b_next} = pixel_color;
    end

    // [핵심] 출력 레지스터 (Flip-Flop)
    always_ff @(posedge clk) begin
        r_port <= r_next;
        g_port <= g_next;
        b_port <= b_next;
    end

endmodule
