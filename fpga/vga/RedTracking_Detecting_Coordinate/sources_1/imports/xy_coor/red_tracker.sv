`timescale 1ns / 1ps

module red_tracker (
    input  logic        clk,
    input  logic        reset,
    input  logic        v_sync,
    input  logic        DE,
    input  logic [ 9:0] x_pixel,
    input  logic [ 9:0] y_pixel,
    input  logic [15:0] data,
    output logic [ 9:0] aim_x,
    output logic [ 9:0] aim_y,
    output logic        aim_detected,
    output logic        raser_shoot,
    output logic [11:0] x_min_out,
    output logic [11:0] x_max_out,
    output logic [11:0] y_min_out,
    output logic [11:0] y_max_out,
    output logic        target_off
);

    localparam SEC_3 = 75_000_000;
    logic [$clog2(SEC_3) - 1 : 0] target_cnt;

    logic [4:0] r_val;
    logic [5:0] g_val;
    logic [4:0] b_val;

    assign r_val = data[15:11];
    assign g_val = data[10:5];
    assign b_val = data[4:0];

    logic is_red;
    // assign is_red = (r_val > 5'd20) && (g_val < 6'd15) && (b_val < 5'd15); // 15
    assign is_red = (r_val > 5'd18) &&    
        (g_val < 6'd15) &&         
        (b_val < 5'd15) && 
        (r_val > g_val) &&         
        (r_val > b_val);   

    logic [11:0] x_min, x_max;
    logic [11:0] y_min, y_max;
    logic [16:0] pixel_count;

    // Raser Shoot Signal
    logic is_taken_aimX;
    assign is_taken_aimX = (aim_x >= 256+32 && aim_x <= 383-32) ? 1'b1 : 1'b0;
    logic is_taken_aimY;
    assign is_taken_aimY = (aim_y >= 192+16 && aim_y <= 287-16) ? 1'b1 : 1'b0;
    assign raser_shoot = (aim_detected & is_taken_aimX & is_taken_aimY);

    logic vsync_d;
    logic vsync_start;

    logic [9:0] center_x, center_y;
    logic [9:0] diff_x, diff_y;

    always_ff @(posedge clk) begin
        vsync_d <= v_sync;
    end

    assign vsync_start = v_sync && !vsync_d;

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            aim_x <= 0;
            aim_y <= 0;
            aim_detected <= 0;
            x_min <= 12'd4095;
            x_max <= 0;
            y_min <= 12'd4095;
            y_max <= 0;
            pixel_count <= 0;
            x_min_out <= 0;
            x_max_out <= 0;
            y_min_out <= 0;
            y_max_out <= 0;
        end else begin
            if (vsync_start) begin
                if (pixel_count > 50) begin // 임계치 이상으로 Red를 발견했을 때, X/Y 좌표 측정
                    aim_detected <= 1'b1;

                    center_x = (x_min + x_max) >> 1;
                    center_y = (y_min + y_max) >> 1;

                    diff_x = (center_x > aim_x) ? (center_x - aim_x) : (aim_x - center_x);
                    diff_y = (center_y > aim_y) ? (center_y - aim_y) : (aim_y - center_y);

                    if (diff_x > 10) aim_x <= center_x;
                    if (diff_y > 10) aim_y <= center_y;

                    x_min_out <= x_min;
                    x_max_out <= x_max;
                    y_min_out <= y_min;
                    y_max_out <= y_max;
                end else begin
                    aim_detected <= 1'b0;
                end

                x_min <= 12'd4095;
                x_max <= 0;
                y_min <= 12'd4095;
                y_max <= 0;
                pixel_count <= 0;

            end else begin
                if (DE && is_red && (x_pixel > 10) && (x_pixel < 630)) begin
                    pixel_count <= pixel_count + 1;

                    if ({2'b00, x_pixel} < x_min) x_min <= {2'b00, x_pixel};
                    if ({2'b00, x_pixel} > x_max) x_max <= {2'b00, x_pixel};

                    if ({2'b00, y_pixel} < y_min) y_min <= {2'b00, y_pixel};
                    if ({2'b00, y_pixel} > y_max) y_max <= {2'b00, y_pixel};
                end
            end
        end
    end

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            target_cnt <= 0;
            target_off  <= 0;
        end else begin
            if (aim_detected) begin
                target_cnt <= 0;
                target_off  <= 0;
            end else begin
                if (target_cnt < SEC_3) begin
                    target_cnt <= target_cnt + 1;
                end else begin
                    target_off <= 1'b1;
                end
            end
        end
    end

endmodule
