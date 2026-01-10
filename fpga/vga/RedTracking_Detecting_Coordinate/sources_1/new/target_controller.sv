`timescale 1ns / 1ps

module target_controller (
    input  logic        clk,
    input  logic        reset,
    
    // Mouse Inputs
    input  logic [ 9:0] mouse_x,
    input  logic [ 9:0] mouse_y,
    input  logic        click_l,
    input  logic        click_r,

    // From Red Tracker (16 Targets)
    input  logic [15:0][9:0] aim_x_all,
    input  logic [15:0][9:0] aim_y_all,
    input  logic [15:0]      aim_detected_all,
    input  logic [15:0][11:0] x_min_all, x_max_all,
    input  logic [15:0][11:0] y_min_all, y_max_all,

    // Outputs to Pixel Mixer & LED
    output logic        is_locked,       // 락온 상태
    output logic [ 3:0] locked_idx,      // 추적 중인 타겟 인덱스
    output logic        center_hit,      // 중앙 정렬 여부
    output logic        aim_detected_led,// LED[0] 제어
    output logic        target_locked_led// LED[14] 제어
);

    // --- 내부 변수 ---
    logic [9:0] last_target_x;
    logic [9:0] last_target_y;

    // 마우스 엣지 디텍션
    logic click_l_d, click_r_d;
    logic click_l_rise, click_r_rise;

    always_ff @(posedge clk) begin
        click_l_d <= click_l;
        click_r_d <= click_r;
    end
    assign click_l_rise = click_l && !click_l_d; 
    assign click_r_rise = click_r && !click_r_d;

    // --- Main State Machine ---
    integer i;
    integer dist_01;
    integer min_dist;
    integer dx, dy;
    integer best_idx;

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            is_locked <= 0;
            locked_idx <= 0;
            last_target_x <= 320;
            last_target_y <= 240;
        end else begin
            // 1. 락온 해제 (우클릭)
            if (click_r_rise) begin
                is_locked <= 0;
            end 
            // 2. 락온 시도 (좌클릭)
            else if (click_l_rise && !is_locked) begin
                for (i=0; i<16; i=i+1) begin
                    if (aim_detected_all[i]) begin
                        // 마우스가 박스 안에 있는지 확인
                        if (mouse_x >= x_min_all[i] && mouse_x <= x_max_all[i] &&
                            mouse_y >= y_min_all[i] && mouse_y <= y_max_all[i]) begin
                            
                            is_locked <= 1;
                            locked_idx <= i[3:0];
                            last_target_x <= aim_x_all[i];
                            last_target_y <= aim_y_all[i];
                        end
                    end
                end
            end
            // 3. 락온 유지 및 핸드오버 (자동 추적)
            else if (is_locked) begin
                min_dist = 100000;
                best_idx = -1;

                // 기억된 위치와 가장 가까운 타겟 찾기
                for (i=0; i<16; i=i+1) begin
                    if (aim_detected_all[i]) begin
                        dx = (aim_x_all[i] > last_target_x) ? (aim_x_all[i] - last_target_x) : (last_target_x - aim_x_all[i]);
                        dy = (aim_y_all[i] > last_target_y) ? (aim_y_all[i] - last_target_y) : (last_target_y - aim_y_all[i]);
                        dist_01 = dx + dy;

                        if (dist_01 < min_dist) begin
                            min_dist = dist_01;
                            best_idx = i;
                        end
                    end
                end

                // 타겟 갱신 (150픽셀 이내일 경우)
                if (best_idx != -1 && min_dist < 150) begin
                    locked_idx <= best_idx[3:0];
                    last_target_x <= aim_x_all[best_idx];
                    last_target_y <= aim_y_all[best_idx];
                end 
            end
        end
    end

    // --- Center Hit Logic ---
    always_comb begin
        if (is_locked && aim_detected_all[locked_idx]) begin
            if (aim_x_all[locked_idx] >= 288 && aim_x_all[locked_idx] <= 351 &&
                aim_y_all[locked_idx] >= 208 && aim_y_all[locked_idx] <= 271)
                center_hit = 1;
            else
                center_hit = 0;
        end else begin
            center_hit = 0;
        end
    end

    // --- LED Output ---
    assign aim_detected_led  = (aim_detected_all != 0);
    assign target_locked_led = center_hit;

endmodule