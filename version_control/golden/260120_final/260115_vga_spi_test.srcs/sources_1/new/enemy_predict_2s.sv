`timescale 1ns / 1ps

module enemy_predict_2s (
    input logic clk,
    input logic reset,

    input logic [9:0] x_meas_now,
    input logic [9:0] y_meas_now,
    input logic       red_detect_in,

    output logic [9:0] x_predict_out,
    output logic [9:0] y_predict_out,
    output logic       vir_red_detect,
    output logic       vir_signal
);

    localparam int X_MAX = 639;
    localparam int Y_MAX = 479;
    localparam int CENTER_X = 320;
    localparam int CENTER_Y = 240;

    // 클럭 주파수(기존 코드 상수 100_000_000 기반)
    localparam int CLK_HZ = 100_000_000;

    localparam int SAMPLE_PERIOD = (100_000_000/1000) * 5; // 0.02s sampling period 

    localparam int GIVEUP_CNT = 100;

    localparam int VMAX = 16;  // limit 16pixels / 0.2s
    localparam int V_EMA_SHIFT = 3;  // ema weight
    localparam int DECAY_SHIFT   = 3; // velocity decay weight (15/16 / 0.2s or 50% / 2s)

    // 10us 펄스 폭(클럭 기반 카운트)
    localparam int PULSE_US = 10;
    localparam int PULSE_CYCLES = (CLK_HZ / 1_000_000) * PULSE_US;

    localparam int CNT_WIDTH = $clog2(SAMPLE_PERIOD);
    localparam int MISS_WIDTH = $clog2(GIVEUP_CNT + 1);
    localparam int PULSE_W = $clog2(PULSE_CYCLES + 1);

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

    logic [9:0] x_out, y_out;  //final output
    logic [9:0] x_meas_prev, y_meas_prev;  //previous measurement
    logic signed [9:0] vx, vy;

    logic prev_detect_valid;  //is the previous measurement valid?
    logic [MISS_WIDTH-1:0] miss_cnt;
    logic virtual_track;

    // 추가: 펄스 생성용 카운터 (0이 아니면 vir_fail=1)
    logic [PULSE_W-1:0] pulse_cnt;

    // 비교/대입에 쓰는 로컬 폭 정리
    logic [MISS_WIDTH-1:0] GIVEUP_CNT_L;
    assign GIVEUP_CNT_L = GIVEUP_CNT[MISS_WIDTH-1:0];

    // 추가: "가상추적이 2초동안 재탐지 실패로 종료되는 순간"에만 펄스 시작
    // 조건: sampling_time 타이밍에, virtual_track 상태이고, red_detect_in=0이며,
    //       이번 사이클에서 miss_cnt가 (GIVEUP_CNT-1)이라면 다음 miss_next에서 종료가 발생함
    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            pulse_cnt <= '0;
        end else begin
            if (sampling_time && virtual_track && !red_detect_in &&
                (miss_cnt == (GIVEUP_CNT_L - 1'b1))) begin
                pulse_cnt <= PULSE_CYCLES[PULSE_W-1:0]; // 약 10us 동안 1 유지
            end else if (pulse_cnt != '0) begin
                pulse_cnt <= pulse_cnt - 1'b1;
            end
        end
    end

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            x_out             <= CENTER_X[9:0];
            y_out             <= CENTER_Y[9:0];

            x_meas_prev       <= 10'd320;
            y_meas_prev       <= 10'd240;
            prev_detect_valid <= 1'b0;
            vx                <= '0;
            vy                <= '0;

            miss_cnt          <= '0;
            virtual_track     <= 1'b0;
            vir_signal        <= 1'b0;
        end else begin
            if (sampling_time) begin  //0.2s period
                if (red_detect_in) begin
                    x_out      <= x_meas_now;
                    y_out      <= y_meas_now;
                    vir_signal <= 1'b0;

                    if (prev_detect_valid) begin
                        logic signed [9:0] vx_meas, vy_meas;
                        logic signed [9:0] vx_adjusted, vy_adjusted;

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
                        vir_signal        <= 1'b1;
                        if (miss_cnt < GIVEUP_CNT_L) begin //before virtual tracking done
                            logic signed [9:0] vx_c, vy_c;
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

                            if (x_next < 0) x_out <= 10'd0;
                            else if (x_next > X_MAX) x_out <= X_MAX[9:0];
                            else x_out <= x_next[9:0];

                            if (y_next < 0) y_out <= 10'd0;
                            else if (y_next > Y_MAX) y_out <= Y_MAX[9:0];
                            else y_out <= y_next[9:0];

                            vx <= vx - (vx >>> DECAY_SHIFT);
                            vy <= vy - (vy >>> DECAY_SHIFT);

                            miss_next = miss_cnt + 1'b1;
                            miss_cnt <= miss_next;

                            if (miss_next >= GIVEUP_CNT_L) begin
                                vir_signal    <= 1'b0;
                                virtual_track <= 1'b0;
                                vx            <= '0;
                                vy            <= '0;
                                x_out         <= CENTER_X[9:0];
                                y_out         <= CENTER_Y[9:0];
                                miss_cnt      <= GIVEUP_CNT_L;
                            end
                        end else begin  //virtual tracking done
                            vir_signal    <= 1'b0;
                            virtual_track <= 1'b0;
                            vx            <= '0;
                            vy            <= '0;
                            x_out         <= CENTER_X[9:0];
                            y_out         <= CENTER_Y[9:0];
                        end
                    end else begin
                        x_out <= CENTER_X[9:0];
                        y_out <= CENTER_Y[9:0];
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
            x_predict_out = CENTER_X[9:0];
            y_predict_out = CENTER_Y[9:0];
        end
    end

endmodule
