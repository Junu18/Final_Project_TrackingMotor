// `timescale 1ns / 1ps

// module red_tracker (
//     input  logic        clk,
//     input  logic        reset,
//     input  logic        v_sync,
//     input  logic        DE,
//     input  logic [ 9:0] x_pixel,
//     input  logic [ 9:0] y_pixel,
//     input  logic [15:0] data,
    
//     // --- [출력] 16개의 타겟 좌표 (배열) ---
//     output logic [15:0][9:0] aim_x_all, 
//     output logic [15:0][9:0] aim_y_all,
//     output logic [15:0]      aim_detected_all,
    
//     // --- [출력] 16개의 바운딩 박스 좌표 (배열) ---
//     output logic [15:0][11:0] x_min_all,
//     output logic [15:0][11:0] x_max_all,
//     output logic [15:0][11:0] y_min_all,
//     output logic [15:0][11:0] y_max_all,
    
//     output logic        target_off
// );

//     localparam SEC_3 = 75_000_000;
//     logic [$clog2(SEC_3) - 1 : 0] target_cnt;

//     // RGB 추출
//     logic [4:0] r_val;
//     logic [5:0] g_val;
//     logic [4:0] b_val;
//     assign r_val = data[15:11];
//     assign g_val = data[10:5];
//     assign b_val = data[4:0];

//     // 빨간색 판별 조건
//     logic is_red;
//     assign is_red = (r_val > 5'd20) && (g_val < 6'd15) && (b_val < 5'd15);

//     // --- 16개 구역 내부 변수 ---
//     logic [11:0] x_min[15:0], x_max[15:0], y_min[15:0], y_max[15:0];
//     logic [16:0] p_count[15:0];

//     logic vsync_d, vsync_start;
//     always_ff @(posedge clk) vsync_d <= v_sync;
//     assign vsync_start = v_sync && !vsync_d;

//     // 구역 인덱스 계산 (0~15)
//     logic [3:0] region_idx;
//     logic [1:0] col, row;

//     always_comb begin
//         if      (x_pixel < 160) col = 0;
//         else if (x_pixel < 320) col = 1;
//         else if (x_pixel < 480) col = 2;
//         else                    col = 3;

//         if      (y_pixel < 120) row = 0;
//         else if (y_pixel < 240) row = 1;
//         else if (y_pixel < 360) row = 2;
//         else                    row = 3;
        
//         region_idx = {row, col};
//     end

//     integer i;
    
//     always_ff @(posedge clk or posedge reset) begin
//         if (reset) begin
//             aim_x_all <= 0; aim_y_all <= 0; aim_detected_all <= 0;
//             target_cnt <= 0; target_off <= 0;
            
//             for(i=0; i<16; i=i+1) begin
//                 x_min_all[i] <= 0; x_max_all[i] <= 0;
//                 y_min_all[i] <= 0; y_max_all[i] <= 0;
                
//                 x_min[i] <= 12'd4095; x_max[i] <= 0;
//                 y_min[i] <= 12'd4095; y_max[i] <= 0;
//                 p_count[i] <= 0;
//             end

//         end else begin
//             if (vsync_start) begin
//                 // --- 프레임 종료: 좌표 업데이트 ---
//                 for(i=0; i<16; i=i+1) begin
//                     if (p_count[i] > 30) begin // 노이즈 필터
//                         aim_detected_all[i] <= 1'b1;
//                         aim_x_all[i] <= (x_min[i] + x_max[i]) >> 1;
//                         aim_y_all[i] <= (y_min[i] + y_max[i]) >> 1;
                        
//                         x_min_all[i] <= x_min[i];
//                         x_max_all[i] <= x_max[i];
//                         y_min_all[i] <= y_min[i];
//                         y_max_all[i] <= y_max[i];
//                     end else begin
//                         aim_detected_all[i] <= 1'b0;
//                         x_min_all[i] <= 0; x_max_all[i] <= 0;
//                         y_min_all[i] <= 0; y_max_all[i] <= 0;
//                     end
//                 end

//                 // --- Target Off 타이머 ---
//                 if (aim_detected_all != 0) begin
//                     target_cnt <= 0;
//                     target_off <= 0;
//                 end else begin
//                     if (target_cnt < SEC_3) target_cnt <= target_cnt + 1;
//                     else target_off <= 1'b1;
//                 end

//                 // --- 변수 리셋 ---
//                 for(i=0; i<16; i=i+1) begin
//                     x_min[i] <= 12'd4095; x_max[i] <= 0;
//                     y_min[i] <= 12'd4095; y_max[i] <= 0;
//                     p_count[i] <= 0;
//                 end

//             end else begin
//                 // --- 픽셀 스캔 ---
//                 if (DE && is_red) begin
//                     p_count[region_idx] <= p_count[region_idx] + 1;
                    
//                     if ({2'b00, x_pixel} < x_min[region_idx]) x_min[region_idx] <= {2'b00, x_pixel};
//                     if ({2'b00, x_pixel} > x_max[region_idx]) x_max[region_idx] <= {2'b00, x_pixel};
                    
//                     if ({2'b00, y_pixel} < y_min[region_idx]) y_min[region_idx] <= {2'b00, y_pixel};
//                     if ({2'b00, y_pixel} > y_max[region_idx]) y_max[region_idx] <= {2'b00, y_pixel};
//                 end
//             end
//         end
//     end
// endmodule


`timescale 1ns / 1ps

module red_tracker (
    input  logic        clk,
    input  logic        reset,
    input  logic        v_sync,
    input  logic        DE,
    input  logic [ 9:0] x_pixel,
    input  logic [ 9:0] y_pixel,
    input  logic [15:0] data,
    
    // 최종 출력 (병합된 결과)
    output logic [15:0][9:0] aim_x_all, 
    output logic [15:0][9:0] aim_y_all,
    output logic [15:0]      aim_detected_all,
    
    output logic [15:0][11:0] x_min_all,
    output logic [15:0][11:0] x_max_all,
    output logic [15:0][11:0] y_min_all,
    output logic [15:0][11:0] y_max_all,
    
    output logic        target_off
);

    localparam SEC_3 = 75_000_000;
    logic [$clog2(SEC_3) - 1 : 0] target_cnt;

    // RGB 추출 & 판별
    logic [4:0] r_val;
    logic [5:0] g_val;
    logic [4:0] b_val;
    assign r_val = data[15:11];
    assign g_val = data[10:5];
    assign b_val = data[4:0];

    logic is_red;
    assign is_red = (r_val > 5'd20) && (g_val < 6'd15) && (b_val < 5'd15);

    // --- 내부 Raw 데이터 (병합 전) ---
    logic [11:0] r_x_min[15:0], r_x_max[15:0], r_y_min[15:0], r_y_max[15:0];
    logic [16:0] p_count[15:0];
    logic [15:0] r_detected;

    logic vsync_d, vsync_start;
    always_ff @(posedge clk) vsync_d <= v_sync;
    assign vsync_start = v_sync && !vsync_d;

    // 구역 인덱스 계산
    logic [3:0] region_idx;
    logic [1:0] col, row;
    always_comb begin
        if      (x_pixel < 160) col = 0;
        else if (x_pixel < 320) col = 1;
        else if (x_pixel < 480) col = 2;
        else                    col = 3;

        if      (y_pixel < 120) row = 0;
        else if (y_pixel < 240) row = 1;
        else if (y_pixel < 360) row = 2;
        else                    row = 3;
        
        region_idx = {row, col};
    end

    integer i, r, c;
    integer curr, next;
    
    // --- 병합 로직용 임시 변수 (Blocking 할당용) ---
    // SystemVerilog에서는 배열 전체를 logic 변수로 선언하고 comb 로직에서 처리하는 것이 안전함
    logic [11:0] t_x_min[15:0], t_x_max[15:0];
    logic [11:0] t_y_min[15:0], t_y_max[15:0];
    logic [15:0] t_detected;

    // 3. 타이머 제어
    always_ff @(posedge clk or posedge reset) begin
        if(reset) begin
            target_cnt <= 0; target_off <= 0;
        end else begin
            if (aim_detected_all != 0) begin
                target_cnt <= 0; target_off <= 0;
            end else begin
                if (target_cnt < SEC_3) target_cnt <= target_cnt + 1;
                else target_off <= 1'b1;
            end
        end
    end

    // --- 메인 파이프라인 ---
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            // 초기화
            for(i=0; i<16; i=i+1) begin
                aim_x_all[i] <= 0; aim_y_all[i] <= 0; aim_detected_all[i] <= 0;
                x_min_all[i] <= 0; x_max_all[i] <= 0; y_min_all[i] <= 0; y_max_all[i] <= 0;
                
                r_x_min[i] <= 12'd4095; r_x_max[i] <= 0;
                r_y_min[i] <= 12'd4095; r_y_max[i] <= 0;
                p_count[i] <= 0; r_detected[i] <= 0;
            end
        end else begin
            if (vsync_start) begin
                // 1. Raw 데이터 확정 (노이즈 필터링)
                for(i=0; i<16; i=i+1) begin
                    if (p_count[i] > 30) begin
                        t_detected[i] = 1'b1;
                        t_x_min[i] = r_x_min[i]; t_x_max[i] = r_x_max[i];
                        t_y_min[i] = r_y_min[i]; t_y_max[i] = r_y_max[i];
                    end else begin
                        t_detected[i] = 1'b0;
                        t_x_min[i] = 0; t_x_max[i] = 0; t_y_min[i] = 0; t_y_max[i] = 0;
                    end
                end

                // 2. 가로 병합 (Horizontal Merge) : 좌 -> 우
                // 예: [0]과 [1]이 붙어있으면 [0]으로 합치고 [1]은 제거
                for (r=0; r<4; r=r+1) begin
                    for (c=0; c<3; c=c+1) begin // 0,1,2 까지만 (오른쪽과 비교)
                        curr = r*4 + c;
                        next = curr + 1;
                        
                        // 둘 다 감지되었고, X좌표 경계가 맞닿아 있으면 (Gap < 10 pixels)
                        if (t_detected[curr] && t_detected[next]) begin
                            if (t_x_min[next] <= t_x_max[curr] + 10) begin
                                // curr에 병합
                                if (t_x_max[next] > t_x_max[curr]) t_x_max[curr] = t_x_max[next];
                                if (t_y_min[next] < t_y_min[curr]) t_y_min[curr] = t_y_min[next];
                                if (t_y_max[next] > t_y_max[curr]) t_y_max[curr] = t_y_max[next];
                                
                                // next 삭제
                                t_detected[next] = 0; 
                            end
                        end
                    end
                end

                // 3. 세로 병합 (Vertical Merge) : 상 -> 하
                for (c=0; c<4; c=c+1) begin
                    for (r=0; r<3; r=r+1) begin // 0,1,2 까지만 (아래쪽과 비교)
                        curr = r*4 + c;
                        next = curr + 4;
                        
                        if (t_detected[curr] && t_detected[next]) begin
                            if (t_y_min[next] <= t_y_max[curr] + 10) begin
                                // curr에 병합
                                if (t_y_max[next] > t_y_max[curr]) t_y_max[curr] = t_y_max[next];
                                if (t_x_min[next] < t_x_min[curr]) t_x_min[curr] = t_x_min[next];
                                if (t_x_max[next] > t_x_max[curr]) t_x_max[curr] = t_x_max[next];
                                
                                // next 삭제
                                t_detected[next] = 0;
                            end
                        end
                    end
                end

                // 4. 최종 결과 업데이트
                for(i=0; i<16; i=i+1) begin
                    aim_detected_all[i] <= t_detected[i];
                    if (t_detected[i]) begin
                        aim_x_all[i] <= (t_x_min[i] + t_x_max[i]) >> 1;
                        aim_y_all[i] <= (t_y_min[i] + t_y_max[i]) >> 1;
                        
                        x_min_all[i] <= t_x_min[i];
                        x_max_all[i] <= t_x_max[i];
                        y_min_all[i] <= t_y_min[i];
                        y_max_all[i] <= t_y_max[i];
                    end else begin
                        aim_x_all[i] <= 0; aim_y_all[i] <= 0;
                        x_min_all[i] <= 0; x_max_all[i] <= 0;
                        y_min_all[i] <= 0; y_max_all[i] <= 0;
                    end
                end

                // 5. 다음 프레임 리셋
                for(i=0; i<16; i=i+1) begin
                    r_x_min[i] <= 12'd4095; r_x_max[i] <= 0;
                    r_y_min[i] <= 12'd4095; r_y_max[i] <= 0;
                    p_count[i] <= 0;
                end

            end else begin
                // --- 픽셀 스캔 (기존과 동일) ---
                if (DE && is_red) begin
                    p_count[region_idx] <= p_count[region_idx] + 1;
                    if ({2'b00, x_pixel} < r_x_min[region_idx]) r_x_min[region_idx] <= {2'b00, x_pixel};
                    if ({2'b00, x_pixel} > r_x_max[region_idx]) r_x_max[region_idx] <= {2'b00, x_pixel};
                    if ({2'b00, y_pixel} < r_y_min[region_idx]) r_y_min[region_idx] <= {2'b00, y_pixel};
                    if ({2'b00, y_pixel} > r_y_max[region_idx]) r_y_max[region_idx] <= {2'b00, y_pixel};
                end
            end
        end
    end

endmodule