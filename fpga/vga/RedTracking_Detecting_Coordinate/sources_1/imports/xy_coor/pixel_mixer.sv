// `timescale 1ns / 1ps

// module pixel_mixer (
//     input  logic [11:0] img_bg,       // 카메라 영상

//     input  logic [ 9:0] aim_x,        // Red Tracker에서 온 X 좌표
//     input  logic [ 9:0] aim_y,        // Red Tracker에서 온 Y 좌표
//     input  logic        aim_detected, // 감지 여부

//     input  logic [ 9:0] x_pixel,      // 현재 VGA 픽셀 X
//     input  logic [ 9:0] y_pixel,      // 현재 VGA 픽셀 Y

//     input  logic [11:0] box_x_min,
//     input  logic [11:0] box_x_max,
//     input  logic [11:0] box_y_min,
//     input  logic [11:0] box_y_max,

//     output logic [ 3:0] r_port,
//     output logic [ 3:0] g_port,
//     output logic [ 3:0] b_port
// );

//     // --- 색상 정의 ---
//     localparam logic [11:0] RED   = 12'hF00;
//     localparam logic [11:0] GREEN = 12'h0F0; // 좌표 글씨 색상 (초록색)
//     localparam logic [11:0] BLUE  = 12'h00F;
//     localparam logic [11:0] WHITE = 12'hFFF;
//     localparam logic [11:0] BLACK = 12'h000;

//     localparam logic [11:0] AIM_COLOR = RED;
//     localparam logic [11:0] TEXT_COLOR = GREEN;
//     localparam logic [11:0] CENTER_COLOR = BLUE;
//     localparam logic [11:0] LOCK_COLOR = WHITE;
//     localparam logic [11:0] BOX_COLOR = GREEN;

//     // --- 조준점 크기 ---
//     localparam THK = 1;  
//     localparam LEN = 10; 

//     // 좌표 표시
//     localparam w     = 8; 
//     localparam h     = 16; 
//     localparam scale = 2; 

//     logic is_locked;
//     assign is_locked = (aim_x >= 288 && aim_x <= 351) && (aim_y >= 208 && aim_y <= 271);

//     function logic draw_digit(input [3:0] num, input [9:0] px, input [9:0] py,
//                               input [9:0] ox, input [9:0] oy);

//         // 7-segment 논리 변수
//         logic sa, sb, sc, sd, se, sf, sg; 
//         logic on;
//         int dx, dy;

//         // 현재 픽셀을 글자 내부 좌표로 변환
//         dx = (px - ox) / scale;
//         dy = (py - oy) / scale;

//         // 글자 범위 밖이면 그리지 않음
//         if (dx < 0 || dx > w || dy < 0 || dy > h) return 0;

//         // 세그먼트 정의 (디지털 숫자 모양)
//         sa = (dy == 0) && (dx > 0 && dx < w);             // 상단 가로
//         sb = (dx == w) && (dy > 0 && dy < (h/2));         // 우측 상단 세로
//         sc = (dx == w) && (dy > (h/2) && dy < h);         // 우측 하단 세로
//         sd = (dy == h) && (dx > 0 && dx < w);             // 하단 가로
//         se = (dx == 0) && (dy > (h/2) && dy < h);         // 좌측 하단 세로
//         sf = (dx == 0) && (dy > 0 && dy < (h/2));         // 좌측 상단 세로
//         sg = (dy == (h/2)) && (dx > 0 && dx < w);         // 중간 가로

//         case (num)
//             0: on = sa | sb | sc | sd | se | sf;
//             1: on = sb | sc;
//             2: on = sa | sb | sd | se | sg;
//             3: on = sa | sb | sc | sd | sg;
//             4: on = sb | sc | sf | sg;
//             5: on = sa | sc | sd | sf | sg;
//             6: on = sa | sc | sd | se | sf | sg;
//             7: on = sa | sb | sc | sf;
//             8: on = sa | sb | sc | sd | se | sf | sg;
//             9: on = sa | sb | sc | sd | sf | sg;
//             default: on = 0;
//         endcase
//         return on;
//     endfunction

//     // --- 좌표 숫자 추출 로직 ---
//     logic is_text_pixel;
//     logic [3:0] x_100, x_10, x_1;
//     logic [3:0] y_100, y_10, y_1;

//     // 좌표를 100의 자리, 10의 자리, 1의 자리로 분리
//     assign x_100 = aim_x / 100;
//     assign x_10  = (aim_x / 10) % 10;
//     assign x_1   = aim_x % 10;

//     assign y_100 = aim_y / 100;
//     assign y_10  = (aim_y / 10) % 10;
//     assign y_1   = aim_y % 10;

//     // --- 텍스트 렌더링 위치 설정 ---
//     always_comb begin
//         is_text_pixel = 0;

//         // [X 좌표 표시] 위치: (10, 10) 부터
//         if (draw_digit(x_100, x_pixel, y_pixel, 10, 10)) is_text_pixel = 1; // 100의 자리
//         if (draw_digit(x_10,  x_pixel, y_pixel, 30, 10)) is_text_pixel = 1; // 10의 자리
//         if (draw_digit(x_1,   x_pixel, y_pixel, 50, 10)) is_text_pixel = 1; // 1의 자리

//         // [Y 좌표 표시] 위치: (10, 50) 부터 (X좌표 아래)
//         if (draw_digit(y_100, x_pixel, y_pixel, 10, 50)) is_text_pixel = 1; 
//         if (draw_digit(y_10,  x_pixel, y_pixel, 30, 50)) is_text_pixel = 1; 
//         if (draw_digit(y_1,   x_pixel, y_pixel, 50, 50)) is_text_pixel = 1; 
//     end


//     // --- 최종 픽셀 믹싱 ---
//     always_comb begin
//         logic [11:0] pixel_color;

//         // 1. 기본 배경: 카메라 영상
//         pixel_color = img_bg;

//         // 2. 센터 박스: 파랑색
//         if (y_pixel == 192+16 || y_pixel == 287-16) begin
//             if (x_pixel >= 256+32 && x_pixel <= 383-32)
//                 pixel_color = CENTER_COLOR;
//         end else if (x_pixel == 256+32 || x_pixel == 383-32) begin
//             if (y_pixel > 192+16 && y_pixel < 287-16)
//                 pixel_color = CENTER_COLOR;
//         end

//         // 2. 조준점 (Aim): 빨간색
//         // 4. 조준점 (Aim) 그리기
//         if (aim_detected) begin
//             if ( ((y_pixel >= aim_y - THK) && (y_pixel <= aim_y + THK) && (x_pixel >= aim_x - LEN) && (x_pixel <= aim_x + LEN)) ||
//                  ((x_pixel >= aim_x - THK) && (x_pixel <= aim_x + THK) && (y_pixel >= aim_y - LEN) && (y_pixel <= aim_y + LEN)) ) begin

//                 // 락온 상태면 하얀색, 아니면 빨간색
//                 pixel_color = is_locked ? LOCK_COLOR : AIM_COLOR;
//             end
//         end

//         // 3. 바운딩 박스 (Bounding Box) 그리기
//         // 조건: 감지됨(aim_detected) AND 락온되지 않음(!is_locked)
//         if (aim_detected && !is_locked) begin
//             // 테두리만 그리기 (위/아래 선 OR 왼쪽/오른쪽 선)
//             if ( ((y_pixel == box_y_min || y_pixel == box_y_max) && (x_pixel >= box_x_min && x_pixel <= box_x_max)) ||
//                  ((x_pixel == box_x_min || x_pixel == box_x_max) && (y_pixel >= box_y_min && y_pixel <= box_y_max)) ) begin
//                 pixel_color = BOX_COLOR; // 초록색 박스
//             end
//         end

//         // 3. 텍스트 (좌표): 초록색 (가장 위에 그림)
//         if (is_text_pixel) begin
//             pixel_color = TEXT_COLOR;
//         end

//         {r_port, g_port, b_port} = pixel_color;
//     end

// endmodule





`timescale 1ns / 1ps

module pixel_mixer (
    input  logic [11:0] img_bg,       // 카메라 영상

    input  logic [ 9:0] aim_x,        // Red Tracker X
    input  logic [ 9:0] aim_y,        // Red Tracker Y
    input  logic        aim_detected, // 감지 여부
    input  logic        target_off,   // 3초 이상 미감지 신호

    input  logic [ 9:0] x_pixel,      // 현재 픽셀 X
    input  logic [ 9:0] y_pixel,      // 현재 픽셀 Y

    input  logic [11:0] box_x_min,
    input  logic [11:0] box_x_max,
    input  logic [11:0] box_y_min,
    input  logic [11:0] box_y_max,

    output logic [ 3:0] r_port,
    output logic [ 3:0] g_port,
    output logic [ 3:0] b_port
);

    // --- 색상 정의 ---
    localparam logic [11:0] RED        = 12'hF00;
    localparam logic [11:0] GREEN      = 12'h0F0;
    localparam logic [11:0] DARK_GREEN = 12'h060; 
    localparam logic [11:0] BLUE       = 12'h00F;
    localparam logic [11:0] WHITE      = 12'hFFF;
    localparam logic [11:0] BLACK      = 12'h000;
    
    localparam logic [11:0] AIM_COLOR    = RED;
    localparam logic [11:0] TEXT_COLOR   = GREEN;
    localparam logic [11:0] CENTER_COLOR = BLUE;
    localparam logic [11:0] LOCK_COLOR   = WHITE;
    localparam logic [11:0] BOX_COLOR    = GREEN;

    // --- 레이더 UI 설정 ---
    // 위치: 우측 하단 (중심점: 550, 380 / 반지름: 70)
    localparam int RADAR_CX = 550;
    localparam int RADAR_CY = 380;
    localparam int RADAR_R  = 70;
    localparam int RADAR_R_SQ = RADAR_R * RADAR_R;

    // --- [최종 수정] 좌표 계산 복구 ---
    // 320 - aim_x (반전) -> aim_x - 320 (정방향)으로 복구
    // $signed()는 유지하여 빨간색 글리치 방지
    
    logic signed [10:0] rel_dx, rel_dy;
    logic [10:0] abs_dx, abs_dy;
    logic [3:0] angle_idx; // 0~15

    always_comb begin
        // [수정] 다시 정방향 계산으로 변경
        // aim_x가 640(오른쪽)이면 rel_dx가 양수가 되어야 함 -> (640 - 320 = +320)
        rel_dx = $signed({1'b0, aim_x}) - 320;
        rel_dy = $signed({1'b0, aim_y}) - 240;
        
        // 절댓값 계산
        abs_dx = (rel_dx < 0) ? -rel_dx : rel_dx;
        abs_dy = (rel_dy < 0) ? -rel_dy : rel_dy;

        // 각도 인덱스 결정 (0: 3시, 4: 6시, 8: 9시, 12: 12시)
        if (target_off) begin
            angle_idx = 4'd12; // 홈 포지션 (12시)
        end else begin
            if (rel_dx >= 0 && rel_dy >= 0) begin // Q4 (우하단) -> 4~6시 방향
                if (abs_dy < (abs_dx >> 1))      angle_idx = 0; // 3시 (우측)
                else if (abs_dx < (abs_dy >> 1)) angle_idx = 4; // 6시 (하단)
                else                             angle_idx = 2; // 대각선
            end
            else if (rel_dx < 0 && rel_dy >= 0) begin // Q3 (좌하단) -> 6~9시 방향
                if (abs_dy < (abs_dx >> 1))      angle_idx = 8; // 9시 (좌측)
                else if (abs_dx < (abs_dy >> 1)) angle_idx = 4; // 6시 (하단)
                else                             angle_idx = 6; // 대각선
            end
            else if (rel_dx < 0 && rel_dy < 0) begin // Q2 (좌상단) -> 9~12시 방향
                if (abs_dy < (abs_dx >> 1))      angle_idx = 8; // 9시 (좌측)
                else if (abs_dx < (abs_dy >> 1)) angle_idx = 12;// 12시 (상단)
                else                             angle_idx = 10;// 대각선
            end
            else begin // Q1 (우상단) -> 12~3시 방향
                if (abs_dy < (abs_dx >> 1))      angle_idx = 0; // 3시 (우측)
                else if (abs_dx < (abs_dy >> 1)) angle_idx = 12;// 12시 (상단)
                else                             angle_idx = 14;// 대각선
            end
        end
    end

    // --- LUT: 16방향 Sin/Cos (반지름 256 기준) ---
    // Y축은 아래가 양수이므로, 12시 방향(위)은 Sin=-256
    logic signed [8:0] lut_cos, lut_sin;
    
    always_comb begin
        case(angle_idx)
            // 우측 (3시) ~ 하단 (6시)
            4'd0:  begin lut_cos = 256; lut_sin = 0;    end // 3시
            4'd1:  begin lut_cos = 236; lut_sin = 98;   end
            4'd2:  begin lut_cos = 181; lut_sin = 181;  end // 4:30
            4'd3:  begin lut_cos = 98;  lut_sin = 236;  end
            4'd4:  begin lut_cos = 0;   lut_sin = 256;  end // 6시

            // 하단 (6시) ~ 좌측 (9시)
            4'd5:  begin lut_cos = -98;  lut_sin = 236; end
            4'd6:  begin lut_cos = -181; lut_sin = 181; end // 7:30
            4'd7:  begin lut_cos = -236; lut_sin = 98;  end
            4'd8:  begin lut_cos = -256; lut_sin = 0;   end // 9시

            // 좌측 (9시) ~ 상단 (12시)
            4'd9:  begin lut_cos = -236; lut_sin = -98; end
            4'd10: begin lut_cos = -181; lut_sin = -181;end // 10:30
            4'd11: begin lut_cos = -98;  lut_sin = -236;end
            4'd12: begin lut_cos = 0;    lut_sin = -256;end // 12시 (Home)

            // 상단 (12시) ~ 우측 (3시)
            4'd13: begin lut_cos = 98;   lut_sin = -236;end
            4'd14: begin lut_cos = 181;  lut_sin = -181;end // 1:30
            4'd15: begin lut_cos = 236;  lut_sin = -98; end
        endcase
    end

    // --- 그리기 로직 ---
    logic in_radar;
    logic on_grid;
    logic on_needle;
    logic on_target_blip;
    
    int dx, dy;
    int dist_sq;
    int target_dx, target_dy;
    
    always_comb begin
        dx = x_pixel - RADAR_CX;
        dy = y_pixel - RADAR_CY;
        dist_sq = dx*dx + dy*dy;
        
        // 1. 레이더 영역 판별
        in_radar = (dist_sq <= RADAR_R_SQ);

        // 2. 격자(Grid) - 동심원 & 십자선
        on_grid = 0;
        if (in_radar) begin
            if ((dist_sq >= 22*22 && dist_sq <= 24*24) ||
                (dist_sq >= 45*45 && dist_sq <= 47*47) ||
                (dist_sq >= 68*68 && dist_sq <= 70*70)) 
                on_grid = 1;
            if ((dx >= -1 && dx <= 1) || (dy >= -1 && dy <= 1))
                on_grid = 1;
        end

        // 3. 모터 지시 침(Needle) 그리기
        on_needle = 0;
        if (in_radar) begin
             int cross_prod, dot_prod;
             cross_prod = dx * lut_sin - dy * lut_cos; 
             dot_prod   = dx * lut_cos + dy * lut_sin; 
             
             // 두께 3픽셀 이내 & 방향 일치 시 그리기
             if ( (cross_prod > -600 && cross_prod < 600) && dot_prod > 0 )
                on_needle = 1;
        end

        // 4. 타겟 점(Blip) 그리기
        on_target_blip = 0;
        if (in_radar && aim_detected) begin
            // [수정] 점 위치도 정방향으로 복구
            target_dx = ($signed({1'b0, aim_x}) - 320) / 4; 
            target_dy = ($signed({1'b0, aim_y}) - 240) / 4;
            
            // 점 크기 반지름 3
            if ( (dx - target_dx)*(dx - target_dx) + (dy - target_dy)*(dy - target_dy) <= 9 )
                on_target_blip = 1;
        end
    end

    // --- 텍스트 및 기존 로직 (그대로 유지) ---
    localparam scale = 2; 
    localparam w = 8;
    localparam h = 16;
    localparam THK = 1;  
    localparam LEN = 10;
    
    logic is_locked;
    assign is_locked = (aim_x >= 288 && aim_x <= 351) && (aim_y >= 208 && aim_y <= 271);

    function logic draw_digit(input [3:0] num, input [9:0] px, input [9:0] py, input [9:0] ox, input [9:0] oy);
        logic sa, sb, sc, sd, se, sf, sg, on;
        int dx, dy;
        dx = (px - ox) / scale;
        dy = (py - oy) / scale;
        if (dx < 0 || dx > w || dy < 0 || dy > h) return 0;
        sa = (dy == 0) && (dx > 0 && dx < w);
        sb = (dx == w) && (dy > 0 && dy < (h/2));
        sc = (dx == w) && (dy > (h/2) && dy < h);
        sd = (dy == h) && (dx > 0 && dx < w);
        se = (dx == 0) && (dy > (h/2) && dy < h);
        sf = (dx == 0) && (dy > 0 && dy < (h/2));
        sg = (dy == (h/2)) && (dx > 0 && dx < w);
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
    logic [3:0] x_100, x_10, x_1;
    logic [3:0] y_100, y_10, y_1;
    assign x_100 = aim_x / 100;
    assign x_10  = (aim_x / 10) % 10;
    assign x_1   = aim_x % 10;
    assign y_100 = aim_y / 100;
    assign y_10  = (aim_y / 10) % 10;
    assign y_1   = aim_y % 10;

    always_comb begin
        is_text_pixel = 0;
        if (draw_digit(x_100, x_pixel, y_pixel, 10, 10)) is_text_pixel = 1;
        if (draw_digit(x_10,  x_pixel, y_pixel, 30, 10)) is_text_pixel = 1;
        if (draw_digit(x_1,   x_pixel, y_pixel, 50, 10)) is_text_pixel = 1;
        if (draw_digit(y_100, x_pixel, y_pixel, 10, 50)) is_text_pixel = 1;
        if (draw_digit(y_10,  x_pixel, y_pixel, 30, 50)) is_text_pixel = 1;
        if (draw_digit(y_1,   x_pixel, y_pixel, 50, 50)) is_text_pixel = 1;
    end

    // --- 최종 픽셀 믹싱 ---
    always_comb begin
        logic [11:0] pixel_color;
        
        if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
        end
        else if (in_radar) begin
            if (on_target_blip)      pixel_color = RED;        // 타겟 위치 점 (빨강)
            else if (on_needle)      pixel_color = GREEN;      // 모터 지시 침 (형광 초록)
            else if (on_grid)        pixel_color = DARK_GREEN; // 격자 (어두운 초록)
            else                     pixel_color = BLACK;      // 레이더 배경
        end
        else begin
            pixel_color = img_bg; 

            // 센터 박스
            if ((y_pixel == 192+16 || y_pixel == 287-16) && (x_pixel >= 256+32 && x_pixel <= 383-32))
                pixel_color = CENTER_COLOR;
            else if ((x_pixel == 256+32 || x_pixel == 383-32) && (y_pixel > 192+16 && y_pixel < 287-16))
                pixel_color = CENTER_COLOR;

            // 조준점
            if (aim_detected) begin
                if ( ((y_pixel >= aim_y - THK) && (y_pixel <= aim_y + THK) && (x_pixel >= aim_x - LEN) && (x_pixel <= aim_x + LEN)) ||
                     ((x_pixel >= aim_x - THK) && (x_pixel <= aim_x + THK) && (y_pixel >= aim_y - LEN) && (y_pixel <= aim_y + LEN)) )
                    pixel_color = is_locked ? LOCK_COLOR : AIM_COLOR;
            end

            // 바운딩 박스
            if (aim_detected && !is_locked) begin
                if ( ((y_pixel == box_y_min || y_pixel == box_y_max) && (x_pixel >= box_x_min && x_pixel <= box_x_max)) ||
                     ((x_pixel == box_x_min || x_pixel == box_x_max) && (y_pixel >= box_y_min && y_pixel <= box_y_max)) )
                    pixel_color = BOX_COLOR;
            end
        end

        {r_port, g_port, b_port} = pixel_color;
    end

endmodule