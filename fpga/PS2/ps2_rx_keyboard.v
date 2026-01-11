`timescale 1ns / 1ps

module ps2_rx_keyboard (
    input  clk,
    input  reset,
    inout  ps2clk,
    inout  ps2data,
    output rx_done,
    output [7:0] valid_data
);

    reg led_checkaa;
    reg led_check00;
    reg led_checkdata;

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
    assign ps2data_falling = ~(ps2data_sync1) & ps2data_sync2;

    // parity 
    reg parity_error_reg, parity_error_next;
    assign led_parity = parity_error_reg;


    // rx fsm
    localparam RX_IDLE = 3, RX_DATA = 2, RX_PARITY = 1, RX_STOP = 0;

    reg [2:0] state_reg, state_next;
    reg [3:0] tick_cnt_reg, tick_cnt_next;
    reg [2:0] bit_cnt_reg, bit_cnt_next;
    reg [3:0] parity_cnt_reg, parity_cnt_next;
    reg [7:0] rx_data_reg, rx_data_next;
    reg [7:0] rx_buffer_reg, rx_buffer_next;
    reg rx_done_reg, rx_done_next;

    assign rx_done = rx_done_reg;
    assign valid_data  = rx_buffer_reg;
    assign led_state   = state_reg;

    assign led_ps2clk  = ps2clk;
    assign led_ps2data = ps2data;


    always @(posedge clk, posedge reset) begin
        if (reset) begin
            state_reg        <= RX_IDLE;
            tick_cnt_reg     <= 0;
            bit_cnt_reg      <= 0;
            rx_data_reg      <= 0;
            rx_done_reg      <= 1'b0;
            rx_buffer_reg    <= 1'b0;
            parity_cnt_reg   <= 0;
            parity_error_reg <= 1'b0;
        end else begin
            state_reg        <= state_next;
            tick_cnt_reg     <= tick_cnt_next;
            bit_cnt_reg      <= bit_cnt_next;
            rx_data_reg      <= rx_data_next;
            rx_done_reg      <= rx_done_next;
            rx_buffer_reg    <= rx_buffer_next;
            parity_cnt_reg   <= parity_cnt_next;
            parity_error_reg <= parity_error_next;
        end
    end

    always @(*) begin
        state_next        = state_reg;
        tick_cnt_next     = tick_cnt_reg;
        bit_cnt_next      = bit_cnt_reg;
        rx_data_next      = rx_data_reg;
        rx_done_next      = rx_done_reg;
        rx_buffer_next    = rx_buffer_reg;
        parity_cnt_next   = parity_cnt_reg;
        parity_error_next = parity_error_reg;
        case (state_reg)
            RX_IDLE: begin
                rx_done_next = 1'b0;
                if (ps2clk_falling && ~ps2data_sync2) begin
                    bit_cnt_next    = 0;
                    tick_cnt_next   = 0;
                    parity_cnt_next = 0;
                    state_next      = RX_DATA;
                end
            end
            RX_DATA: begin
                if (ps2clk_falling) begin
                    if (ps2data_sync2 == 1) begin
                        parity_cnt_next = parity_cnt_reg + 1;
                    end
                    rx_data_next = {ps2data_sync2, rx_data_reg[7:1]};
                    if (bit_cnt_reg == 7) begin
                        state_next = RX_PARITY;
                    end else begin
                        bit_cnt_next = bit_cnt_reg + 1;
                    end
                end
            end
            RX_PARITY: begin
                if (ps2clk_falling) begin
                    if ((parity_cnt_reg[0] ^ ps2data_sync2) == 1'b1) begin
                        state_next = RX_STOP;
                        parity_error_next = 0;
                    end else begin  // 
                        state_next = RX_IDLE;
                        parity_error_next = 1;
                    end
                end
            end
            RX_STOP: begin
                if (ps2clk_falling && ps2data_sync2) begin
                    rx_done_next = 1'b1;
                    rx_buffer_next = rx_data_reg;
                    state_next = RX_IDLE;
                end
            end
        endcase
    end
endmodule


