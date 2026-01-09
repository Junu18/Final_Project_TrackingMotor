`timescale 1ns / 1ps

module pixel_mixer (
    input  logic [11:0] img_bg,       // 카메라 영상
    
    // --- 입력: 16개의 타겟 정보 ---
    input  logic [15:0][9:0] aim_x_all,
    input  logic [15:0][9:0] aim_y_all,
    input  logic [15:0]      aim_detected_all,
    
    // --- 입력: 16개의 박스 정보 ---
    input  logic [15:0][11:0] box_x_min_all,
    input  logic [15:0][11:0] box_x_max_all,
    input  logic [15:0][11:0] box_y_min_all,
    input  logic [15:0][11:0] box_y_max_all,
    
    input  logic        target_off,
    input  logic [ 9:0] x_pixel,
    input  logic [ 9:0] y_pixel,

    output logic [ 3:0] r_port,
    output logic [ 3:0] g_port,
    output logic [ 3:0] b_port
);

    // --- 색상 정의 ---
    localparam logic [11:0] RED   = 12'hF00;
    localparam logic [11:0] GREEN = 12'h0F0;
    localparam logic [11:0] BLUE  = 12'h00F;
    
    localparam logic [11:0] AIM_COLOR = RED;   // 조준점 (빨강)
    localparam logic [11:0] BOX_COLOR = GREEN; // 박스 (초록)
    localparam logic [11:0] TEXT_COLOR = GREEN;

    // --- 에임 크기 ---
    localparam int AIM_LEN = 5;  // 십자선 길이 (개별이라 조금 작게)
    localparam int AIM_THK = 1;  // 두께

    // --- 그리기 로직 ---
    logic on_box;
    logic on_aim;

    always_comb begin
        on_box = 0;
        on_aim = 0;

        // 16개 구역 루프
        for(int k=0; k<16; k=k+1) begin
            if (aim_detected_all[k]) begin
                
                // 1. 바운딩 박스 체크
                if ( ((y_pixel == box_y_min_all[k] || y_pixel == box_y_max_all[k]) && (x_pixel >= box_x_min_all[k] && x_pixel <= box_x_max_all[k])) ||
                     ((x_pixel == box_x_min_all[k] || x_pixel == box_x_max_all[k]) && (y_pixel >= box_y_min_all[k] && y_pixel <= box_y_max_all[k])) ) begin
                    on_box = 1;
                end

                // 2. 개별 에임(십자선) 체크
                // 가로선
                if ( (y_pixel >= aim_y_all[k] - AIM_THK) && (y_pixel <= aim_y_all[k] + AIM_THK) && 
                     (x_pixel >= aim_x_all[k] - AIM_LEN) && (x_pixel <= aim_x_all[k] + AIM_LEN) )
                     on_aim = 1;
                // 세로선
                if ( (x_pixel >= aim_x_all[k] - AIM_THK) && (x_pixel <= aim_x_all[k] + AIM_THK) && 
                     (y_pixel >= aim_y_all[k] - AIM_LEN) && (y_pixel <= aim_y_all[k] + AIM_LEN) )
                     on_aim = 1;
            end
        end
    end

    // --- 텍스트 그리기 함수 (기존 코드 유지) ---
    // (이전 파일의 draw_digit 함수가 있다면 그대로 두세요)
    logic is_text_pixel;
    assign is_text_pixel = 0; 

    // --- 최종 픽셀 믹싱 ---
    always_comb begin
        logic [11:0] pixel_color;

        // 우선순위: 텍스트 > 에임 > 박스 > 카메라
        if (is_text_pixel) begin
            pixel_color = TEXT_COLOR;
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