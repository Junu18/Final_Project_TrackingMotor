`timescale 1ns / 1ps

module enemy_predict_2s (
    input logic clk,
    input logic reset,

    input logic [15:0] x_meas_now,
    input logic [15:0] y_meas_now,
    input logic        red_detect_in,

    output logic [15:0] x_predict_out,
    output logic [15:0] y_predict_out,
    output logic        vir_red_detect
);

    localparam int X_MAX = 639;
    localparam int Y_MAX = 479;
    localparam int CENTER_X = 319;
    localparam int CENTER_Y = 239;

    localparam int SAMPLE_PERIOD = (100_000_000/1000) * 200; // 0.2s sampling period 

    localparam int GIVEUP_CNT = 10;

    localparam int VMAX = 16;  // limit 16pixels / 0.2s
    localparam int V_EMA_SHIFT = 3;  // ema weight
    localparam int DECAY_SHIFT   = 4; // velocity decay weight (15/16 / 0.2s or 50% / 2s)

    localparam int CNT_WIDTH = $clog2(SAMPLE_PERIOD);
    localparam int MISS_WIDTH = $clog2(GIVEUP_CNT + 1);

    logic [CNT_WIDTH-1:0] tick_cnt;
    logic sampling_time;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            tick_cnt      <= '0;
            sampling_time <= 1'b0;
        end else begin
            if (tick_cnt == SAMPLE_PERIOD - 1) begin
                tick_cnt      <= '0;
                sampling_time <= 1'b1;
            end else begin
                tick_cnt      <= tick_cnt + 1'b1;
                sampling_time <= 1'b0;
            end
        end
    end

    logic [15:0] x_out, y_out;  //final output
    logic [15:0] x_meas_prev, y_meas_prev;  //previous measurement
    logic signed [15:0] vx, vy;

    logic prev_detect_valid;  //is the previous measurement valid?
    logic [MISS_WIDTH-1:0] miss_cnt;
    logic virtual_track;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            x_out             <= CENTER_X[15:0];
            y_out             <= CENTER_Y[15:0];

            x_meas_prev       <= 16'd0;
            y_meas_prev       <= 16'd0;
            prev_detect_valid <= 1'b0;
            vx                <= '0;
            vy                <= '0;

            miss_cnt          <= '0;
            virtual_track     <= 1'b0;
        end else begin
            if (sampling_time) begin  //0.2s period
                if (red_detect_in) begin
                    x_out <= x_meas_now;
                    y_out <= y_meas_now;

                    if (prev_detect_valid) begin
                        logic signed [15:0] vx_meas, vy_meas;
                        logic signed [15:0] vx_adjusted, vy_adjusted;

                        vx_meas = $signed(x_meas_now) -
                            $signed(x_meas_prev);  // V= x1-x2
                        vy_meas = $signed(y_meas_now) - $signed(y_meas_prev);

                        if (vx_meas > $signed(VMAX))        
                            vx_adjusted = $signed(VMAX);
                        else if (vx_meas < -$signed(VMAX))
                            vx_adjusted = -$signed(VMAX);
                        else vx_adjusted = vx_meas;

                        if (vy_meas > $signed(VMAX))
                            vy_adjusted = $signed(VMAX);
                        else if (vy_meas < -$signed(VMAX))
                            vy_adjusted = -$signed(VMAX);
                        else vy_adjusted = vy_meas;

                        vx <= vx + ((vx_adjusted - vx) >>> V_EMA_SHIFT);
                        vy <= vy + ((vy_adjusted - vy) >>> V_EMA_SHIFT);
                    end else begin
                        vx <= '0;
                        vy <= '0;
                    end

                    x_meas_prev       <= x_meas_now;
                    y_meas_prev       <= y_meas_now;
                    prev_detect_valid <= 1'b1;

                    miss_cnt          <= '0;
                    virtual_track     <= 1'b1;

                end else begin  //can't detect red
                    prev_detect_valid <= 1'b0;

                    if (virtual_track) begin
                        if (miss_cnt < GIVEUP_CNT[MISS_WIDTH-1:0]) begin //before virtual tracking done
                            logic signed [15:0] vx_c, vy_c;
                            int x_next, y_next;
                            logic [MISS_WIDTH-1:0] miss_next;

                            if (vx > $signed(VMAX)) vx_c = $signed(VMAX);
                            else if (vx < -$signed(VMAX)) vx_c = -$signed(VMAX);
                            else vx_c = vx;

                            if (vy > $signed(VMAX)) vy_c = $signed(VMAX);
                            else if (vy < -$signed(VMAX)) vy_c = -$signed(VMAX);
                            else vy_c = vy;

                            x_next = $signed({1'b0, x_out}) + vx_c;
                            y_next = $signed({1'b0, y_out}) + vy_c;

                            if (x_next < 0) x_out <= 16'd0;
                            else if (x_next > X_MAX) x_out <= X_MAX[15:0];
                            else x_out <= x_next[15:0];

                            if (y_next < 0) y_out <= 16'd0;
                            else if (y_next > Y_MAX) y_out <= Y_MAX[15:0];
                            else y_out <= y_next[15:0];

                            vx <= vx - (vx >>> DECAY_SHIFT);
                            vy <= vy - (vy >>> DECAY_SHIFT);

                            miss_next = miss_cnt + 1'b1;
                            miss_cnt <= miss_next;

                            if (miss_next >= GIVEUP_CNT[MISS_WIDTH-1:0]) begin
                                virtual_track <= 1'b0;
                                vx            <= '0;
                                vy            <= '0;
                                x_out         <= CENTER_X[15:0];
                                y_out         <= CENTER_Y[15:0];
                                miss_cnt      <= GIVEUP_CNT[MISS_WIDTH-1:0];
                            end
                        end else begin //virtual tracking done
                            virtual_track <= 1'b0;
                            vx            <= '0;
                            vy            <= '0;
                            x_out         <= CENTER_X[15:0];
                            y_out         <= CENTER_Y[15:0];
                        end
                    end else begin
                        x_out <= CENTER_X[15:0];
                        y_out <= CENTER_Y[15:0];
                    end
                end
            end
        end
    end

    always_comb begin
        vir_red_detect = virtual_track;
        if (virtual_track) begin
            x_predict_out = x_out;
            y_predict_out = y_out;
        end else begin
            x_predict_out = CENTER_X[15:0];
            y_predict_out = CENTER_Y[15:0];
        end
    end

endmodule
