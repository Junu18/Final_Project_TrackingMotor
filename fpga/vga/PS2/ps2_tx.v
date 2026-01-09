`timescale 1ns / 1ps
module ps2_tx (
    input      clk,
    input      reset,
    input      tick,
    input      tx_start,
    //    input      [7:0] tx_data,
    inout      ps2clk,
    inout      ps2data
);


    // syncronizer, edge detector
    reg ps2clk_sync0, ps2clk_sync1, ps2clk_sync2;
    wire ps2clk_rising, ps2clk_falling;
    reg ps2data_sync0, ps2data_sync1, ps2data_sync2;
    wire ps2data_rising, ps2data_falling;

    always @(posedge clk, posedge reset) begin
        if (reset) begin
            ps2clk_sync0  <= 1'b1;
            ps2clk_sync1  <= 1'b1;
            ps2clk_sync2  <= 1'b1;
            ps2data_sync0 <= 1'b1;
            ps2data_sync1 <= 1'b1;
            ps2data_sync2 <= 1'b1;
        end else begin
            ps2clk_sync0  <= ps2clk;
            ps2clk_sync1  <= ps2clk_sync0;
            ps2clk_sync2  <= ps2clk_sync1;
            ps2data_sync0 <= ps2data;
            ps2data_sync1 <= ps2data_sync0;
            ps2data_sync2 <= ps2data_sync1;
        end
    end
    assign ps2clk_rising = ps2clk_sync1 & ~(ps2clk_sync2);
    assign ps2clk_falling = ~(ps2clk_sync1) & ps2clk_sync2;
    assign ps2data_rising = ps2data_sync1 & ~(ps2data_sync2);
    assign ps2datas_falling = ~(ps2data_sync1) & ps2data_sync2;

    // sda 3state buffer
    reg  ps2clk_en;
    reg  ps2clk_wr;
    wire ps2clk_rd;
    assign ps2clk_rd = ps2clk;
    assign ps2clk = (ps2clk_en) ? ps2clk_wr : 1'bz;

    reg  ps2data_en;
    reg  ps2data_wr;
    wire ps2data_rd;
    assign ps2data_rd = ps2data;
    assign ps2data = (ps2data_en) ? ps2data_wr : 1'bz;

    // state
    localparam TX_IDLE = 0;
    localparam TX_CLK_DOWN = 1;
    localparam TX_DATA_DOWN = 2;
    localparam TX_CLK_UP = 3;
    localparam TX_F4 = 4;
    localparam TX_PARITY = 5;
    localparam TX_STOP = 6;
    localparam RX_FA = 7;

    reg [2:0] tx_state_reg, tx_state_next;
    reg tx_busy_reg, tx_busy_next;
    reg tx_reg, tx_next;
    reg [7:0] data_buf_reg, data_buf_next;
    reg [3:0] tick_cnt_reg, tick_cnt_next;
    reg [2:0] bit_cnt_reg, bit_cnt_next;

    always @(posedge clk, posedge reset) begin
        if (reset) begin
            tx_state_reg <= TX_IDLE;
            tick_cnt_reg <= 4'b0000;
            bit_cnt_reg <= 3'b000;
            tx_busy_reg <= 1'b0;
            tx_reg <= 1'b1;
            data_buf_reg <= 8'h00;
        end else begin
            tx_state_reg <= tx_state_next;
            tx_busy_reg <= tx_busy_next;
            tx_reg <= tx_next;
            data_buf_reg <= data_buf_next;
            tick_cnt_reg <= tick_cnt_next;
            bit_cnt_reg <= bit_cnt_next;
        end
    end

    always @(*) begin
        tx_state_next = tx_state_reg;
        tx_busy_next  = tx_busy_reg;
        tx_next       = tx_reg;
        data_buf_next = data_buf_reg;
        tick_cnt_next = tick_cnt_reg;
        bit_cnt_next  = bit_cnt_reg;
        case (tx_state_reg)
            TX_IDLE: begin
                tx_next      = 1'b1;
                tx_busy_next = 1'b0;
                ps2clk_en    = 1'b0;
                ps2data_en   = 1'b0;
                if (tx_start) begin
                    tick_cnt_next = 0;
                    data_buf_next = 8'hF4;  // tx_data
                    tx_state_next = TX_CLK_DOWN;
                end
            end
            TX_CLK_DOWN: begin
                tx_next      = 1'b0;
                tx_busy_next = 1'b1;
                ps2clk_en    = 1'b1;
                ps2data_en   = 1'b1;
                ps2clk_wr    = 0;
                ps2data_wr   = 1;
                if (tick) begin
                    if (tick_cnt_reg == 4) begin
                        tick_cnt_next = 0;
                        bit_cnt_next  = 0;
                        tx_state_next = TX_DATA_DOWN;
                    end else begin
                        tick_cnt_next = tick_cnt_reg + 1;
                    end
                end
            end
            TX_DATA_DOWN: begin
                tx_next      = 1'b0;
                tx_busy_next = 1'b1;
                ps2clk_en    = 1'b1;
                ps2data_en   = 1'b1;
                ps2clk_wr    = 1'b0;
                ps2data_wr   = 1'b0;
                if (tick) begin
                    tick_cnt_next = 0;
                    bit_cnt_next  = 0;
                    tx_state_next = TX_CLK_UP;
                end
            end
            TX_CLK_UP: begin
                tx_next      = 1'b0;
                tx_busy_next = 1'b1;
                ps2clk_en    = 1'b1;
                ps2data_en   = 1'b1;
                ps2clk_wr    = 1'b1;
                ps2data_wr   = 1'b0;
                if (tick) begin
                    tick_cnt_next = 0;
                    bit_cnt_next  = 0;
                    tx_state_next = TX_F4;
                    data_buf_next = 8'hF4;
                end
            end
            TX_F4: begin
                ps2clk_en  = 1'b0;
                ps2data_en = 1'b1;
                ps2data_wr = data_buf_reg[0];
                if (ps2clk_falling) begin
                    if (bit_cnt_reg == 7) begin
                        tx_state_next = TX_PARITY;
                    end else begin
                        bit_cnt_next  = bit_cnt_reg + 1;
                        data_buf_next = data_buf_reg >> 1;
                    end
                end
            end
            TX_PARITY: begin
                ps2clk_en  = 1'b0;
                ps2data_en = 1'b1;
                ps2data_wr = ~(^8'hF4);
                if (ps2clk_falling) begin
                    tx_state_next = TX_STOP;
                end
            end
            TX_STOP: begin
                tx_next = 1'b1;
                if (ps2clk_falling) begin
                    tx_busy_next  = 1'b0;
                    tx_state_next = TX_IDLE;
                end
            end
        endcase
    end

endmodule


module tick_gen #(
    parameter TICK_COUNT = 3000 - 1  // 30us 
) (
    input  clk,
    input  rst,
    output tick
);

    reg [$clog2(TICK_COUNT)-1 : 0] counter_reg;
    reg tick_reg;

    assign tick = tick_reg;

    always @(posedge clk, posedge rst) begin
        if (rst) begin
            counter_reg <= 0;
            tick_reg <= 1'b0;
        end else begin
            if (counter_reg == TICK_COUNT) begin
                counter_reg <= 0;
                tick_reg <= 1'b1;
            end else begin
                counter_reg <= counter_reg + 1;
                tick_reg <= 1'b0;
            end
        end
    end
endmodule


module button_debounce #(
    parameter F_BTN   = 1_000_000,
    parameter NUM_DEB = 16
) (
    input  clk,
    input  reset,
    input  i_btn,
    output o_btn
);

    localparam IDLE = 2'b00, DEBOUNCE = 2'b01, PULSE = 2'b10, STOP = 2'b11;
    reg [1:0] state_reg, state_next;
    reg [$clog2(NUM_DEB)-1:0] deb_cnt_reg, deb_cnt_next;
    wire w_tick;

    btn_tick_gen #(
        .FREQ(F_BTN)
    ) U_TICK (
        .clk(clk),
        .reset(reset),
        .o_tick(w_tick)
    );

    assign o_btn = (state_reg == PULSE);

    always @(posedge clk, posedge reset) begin
        if (reset) begin
            state_reg   <= 0;
            deb_cnt_reg <= 0;
        end else begin
            state_reg   <= state_next;
            deb_cnt_reg <= deb_cnt_next;
        end
    end

    always @(*) begin
        state_next   = state_reg;
        deb_cnt_next = deb_cnt_reg;
        case (state_reg)
            IDLE: begin
                deb_cnt_next = 0;
                if (i_btn) begin
                    state_next = DEBOUNCE;
                end
            end
            DEBOUNCE: begin
                if (w_tick) begin
                    if (i_btn) begin
                        if (deb_cnt_reg == NUM_DEB - 1) begin
                            state_next = PULSE;
                        end else begin
                            deb_cnt_next = deb_cnt_reg + 1;
                        end
                    end else begin
                        state_next = IDLE;
                    end
                end
            end
            PULSE: begin
                state_next = STOP;
            end
            STOP: begin
                if (!i_btn) begin
                    state_next = IDLE;
                end
            end
        endcase
    end

endmodule

module btn_tick_gen #(
    parameter FREQ = 1_000_000
) (
    input  clk,
    input  reset,
    output o_tick
);

    localparam F_COUNT = 100_000_000 / FREQ;

    reg [$clog2(F_COUNT)-1:0] r_cnt;
    reg r_tick;
    assign o_tick = r_tick;

    always @(posedge clk, posedge reset) begin
        if (reset) begin
            r_cnt  <= 0;
            r_tick <= 1'b0;
        end else begin
            if (r_cnt == F_COUNT - 1) begin
                r_cnt  <= 0;
                r_tick <= 1'b1;
            end else begin
                r_cnt  <= r_cnt + 1;
                r_tick <= 1'b0;
            end
        end
    end

endmodule
