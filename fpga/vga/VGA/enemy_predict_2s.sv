`timescale 1ns / 1ps

module enemy_predict_2s (
    input  logic        clk,
    input  logic        reset,
    input  logic        v_sync,

    input  logic [15:0] x_real_in,
    input  logic [15:0] y_real_in,
    input  logic        red_detect_in,

    output logic [15:0] x_predict_out,
    output logic [15:0] y_predict_out,
    output logic        red_detect_spi_out
);

    localparam int X_MAX         = 639;
    localparam int Y_MAX         = 479;

    localparam int CLK_HZ        = 100_000_000;
    localparam int SAMPLE_MS     = 200;
    localparam int SAMPLE_CYCLES = (CLK_HZ/1000) * SAMPLE_MS;

    localparam int GIVEUP_TICKS  = 10;

    localparam int VMAX          = 16;
    localparam int V_EMA_SHIFT   = 3;
    localparam int DECAY_SHIFT   = 4;
    localparam int POS_EMA_SHIFT = 0;

    localparam int CENTER_X      = 320;
    localparam int CENTER_Y      = 240;

    localparam int CNT_W  = $clog2(SAMPLE_CYCLES);
    localparam int MISS_W = $clog2(GIVEUP_TICKS + 1);

    function automatic logic signed [15:0] clamp_s16(
        input logic signed [15:0] v,
        input int                 lo,
        input int                 hi
    );
        int vi;
        begin
            vi = v;
            if (vi < lo)      clamp_s16 = shortint'(lo);
            else if (vi > hi) clamp_s16 = shortint'(hi);
            else              clamp_s16 = v;
        end
    endfunction

    function automatic logic [15:0] clamp_u16_range(
        input int v,
        input int lo,
        input int hi
    );
        begin
            if (v < lo)      clamp_u16_range = lo[15:0];
            else if (v > hi) clamp_u16_range = hi[15:0];
            else             clamp_u16_range = v[15:0];
        end
    endfunction

    function automatic logic [15:0] ema_pos(
        input logic [15:0] cur,
        input logic [15:0] meas
    );
        logic signed [16:0] diff;
        logic signed [16:0] upd;
        begin
            if (POS_EMA_SHIFT <= 0) ema_pos = meas;
            else begin
                diff = $signed({1'b0, meas}) - $signed({1'b0, cur});
                upd  = $signed({1'b0, cur}) + (diff >>> POS_EMA_SHIFT);
                ema_pos = upd[15:0];
            end
        end
    endfunction

    logic [CNT_W-1:0] tick_cnt;
    logic sample_stb;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            tick_cnt   <= '0;
            sample_stb <= 1'b0;
        end else begin
            if (tick_cnt == SAMPLE_CYCLES-1) begin
                tick_cnt   <= '0;
                sample_stb <= 1'b1;
            end else begin
                tick_cnt   <= tick_cnt + 1'b1;
                sample_stb <= 1'b0;
            end
        end
    end

    logic [15:0] x_est_r, y_est_r;

    logic [15:0] x_prev_meas_r, y_prev_meas_r;
    logic        prev_detect_r;

    logic signed [15:0] vx_r, vy_r;

    logic [MISS_W-1:0] miss_cnt_r;
    logic              track_r;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            x_est_r       <= CENTER_X[15:0];
            y_est_r       <= CENTER_Y[15:0];

            x_prev_meas_r <= 16'd0;
            y_prev_meas_r <= 16'd0;
            prev_detect_r <= 1'b0;

            vx_r          <= '0;
            vy_r          <= '0;

            miss_cnt_r    <= '0;
            track_r       <= 1'b0;
        end else begin
            if (sample_stb) begin
                if (red_detect_in) begin
                    x_est_r <= ema_pos(x_est_r, x_real_in);
                    y_est_r <= ema_pos(y_est_r, y_real_in);

                    if (prev_detect_r) begin
                        logic signed [15:0] vx_meas, vy_meas;
                        logic signed [15:0] vx_tgt,  vy_tgt;

                        vx_meas = $signed(x_real_in) - $signed(x_prev_meas_r);
                        vy_meas = $signed(y_real_in) - $signed(y_prev_meas_r);

                        vx_tgt  = clamp_s16(vx_meas, -VMAX, VMAX);
                        vy_tgt  = clamp_s16(vy_meas, -VMAX, VMAX);

                        vx_r <= vx_r + ((vx_tgt - vx_r) >>> V_EMA_SHIFT);
                        vy_r <= vy_r + ((vy_tgt - vy_r) >>> V_EMA_SHIFT);
                    end else begin
                        vx_r <= '0;
                        vy_r <= '0;
                    end

                    x_prev_meas_r <= x_real_in;
                    y_prev_meas_r <= y_real_in;
                    prev_detect_r <= 1'b1;

                    miss_cnt_r    <= '0;
                    track_r       <= 1'b1;
                end else begin
                    prev_detect_r <= 1'b0;

                    if (track_r) begin
                        if (miss_cnt_r < GIVEUP_TICKS[MISS_W-1:0]) begin
                            logic signed [15:0] vx_c, vy_c;
                            int x_next, y_next;
                            logic [MISS_W-1:0] miss_next;

                            vx_c = clamp_s16(vx_r, -VMAX, VMAX);
                            vy_c = clamp_s16(vy_r, -VMAX, VMAX);

                            x_next = $signed({1'b0, x_est_r}) + vx_c;
                            y_next = $signed({1'b0, y_est_r}) + vy_c;

                            x_est_r <= clamp_u16_range(x_next, 0, X_MAX);
                            y_est_r <= clamp_u16_range(y_next, 0, Y_MAX);

                            vx_r <= vx_r - (vx_r >>> DECAY_SHIFT);
                            vy_r <= vy_r - (vy_r >>> DECAY_SHIFT);

                            miss_next  = miss_cnt_r + 1'b1;
                            miss_cnt_r <= miss_next;

                            if (miss_next >= GIVEUP_TICKS[MISS_W-1:0]) begin
                                track_r    <= 1'b0;
                                vx_r       <= '0;
                                vy_r       <= '0;
                                x_est_r    <= CENTER_X[15:0];
                                y_est_r    <= CENTER_Y[15:0];
                                miss_cnt_r <= GIVEUP_TICKS[MISS_W-1:0];
                            end
                        end else begin
                            track_r <= 1'b0;
                            vx_r    <= '0;
                            vy_r    <= '0;
                            x_est_r <= CENTER_X[15:0];
                            y_est_r <= CENTER_Y[15:0];
                        end
                    end else begin
                        x_est_r <= CENTER_X[15:0];
                        y_est_r <= CENTER_Y[15:0];
                    end
                end
            end
        end
    end

    always_comb begin
        red_detect_spi_out = track_r;
        if (track_r) begin
            x_predict_out = x_est_r;
            y_predict_out = y_est_r;
        end else begin
            x_predict_out = CENTER_X[15:0];
            y_predict_out = CENTER_Y[15:0];
        end
    end

endmodule
