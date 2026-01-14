`timescale 1ns / 1ps

module spi_slave (
    input  logic        clk,
    input  logic        reset,

    input  logic        sclk,
    input  logic        mosi,
    input  logic        cs,
    output logic        miso,

    input  logic [31:0] data_frame,
    output logic        req,

    output logic [31:0] rx_data,
    output logic        rx_valid
);

    logic sclk_sync0, sclk_sync1, sclk_sync2;
    logic cs_sync0,   cs_sync1,   cs_sync2;
    logic mosi_sync0, mosi_sync1, mosi_sync2;

    logic sclk_rise, sclk_fall;
    logic cs_fall, cs_rise;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            sclk_sync0 <= 1'b0;
            sclk_sync1 <= 1'b0;
            sclk_sync2 <= 1'b0;

            cs_sync0   <= 1'b1;
            cs_sync1   <= 1'b1;
            cs_sync2   <= 1'b1;

            mosi_sync0 <= 1'b0;
            mosi_sync1 <= 1'b0;
            mosi_sync2 <= 1'b0;
        end else begin
            sclk_sync0 <= sclk;
            sclk_sync1 <= sclk_sync0;
            sclk_sync2 <= sclk_sync1;

            cs_sync0   <= cs;
            cs_sync1   <= cs_sync0;
            cs_sync2   <= cs_sync1;

            mosi_sync0 <= mosi;
            mosi_sync1 <= mosi_sync0;
            mosi_sync2 <= mosi_sync1;
        end
    end

    assign sclk_rise =  sclk_sync1 & ~sclk_sync2;
    assign sclk_fall = ~sclk_sync1 &  sclk_sync2;

    assign cs_fall   = ~cs_sync1 &  cs_sync2;
    assign cs_rise   =  cs_sync1 & ~cs_sync2;

    typedef enum logic [1:0] {ST_IDLE, ST_PREP, ST_SHIFT, ST_HOLD} state_t;
    state_t state, state_next;

    logic [31:0] tx_shift, tx_shift_next;
    logic [31:0] rx_shift, rx_shift_next;
    logic [5:0]  bit_cnt,  bit_cnt_next;

    logic [1:0]  prep_cnt, prep_cnt_next;

    logic miso_reg, miso_next;
    logic req_reg, req_next;

    logic [31:0] rx_data_reg, rx_data_next;
    logic        rx_valid_reg, rx_valid_next;

    assign miso     = cs_sync1 ? 1'b0 : miso_reg;
    assign req      = req_reg;
    assign rx_data  = rx_data_reg;
    assign rx_valid = rx_valid_reg;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            state        <= ST_IDLE;
            tx_shift     <= 32'b01_0011_1111_0_1110_1111_0000000000000;
            rx_shift     <= '0;
            bit_cnt      <= '0;
            prep_cnt     <= '0;
            miso_reg     <= 1'b0;
            req_reg      <= 1'b0;
            rx_data_reg  <= '0;
            rx_valid_reg <= 1'b0;
        end else begin
            state        <= state_next;
            tx_shift     <= tx_shift_next;
            rx_shift     <= rx_shift_next;
            bit_cnt      <= bit_cnt_next;
            prep_cnt     <= prep_cnt_next;
            miso_reg     <= miso_next;
            req_reg      <= req_next;
            rx_data_reg  <= rx_data_next;
            rx_valid_reg <= rx_valid_next;
        end
    end

    always_comb begin
        state_next     = state;
        tx_shift_next  = tx_shift;
        rx_shift_next  = rx_shift;
        bit_cnt_next   = bit_cnt;
        prep_cnt_next  = prep_cnt;
        miso_next      = miso_reg;
        req_next       = 1'b0;
        rx_data_next   = rx_data_reg;
        rx_valid_next  = 1'b0;

        if (cs_rise) begin
            state_next     = ST_IDLE;
            tx_shift_next  = '0;
            rx_shift_next  = '0;
            bit_cnt_next   = '0;
            prep_cnt_next  = '0;
            miso_next      = 1'b0;
            req_next       = 1'b0;
            rx_valid_next  = 1'b0;
        end else begin
            case (state)
                ST_IDLE: begin
                    if (cs_fall) begin
                        prep_cnt_next = 2'd2;
                        req_next      = 1'b1;
                        state_next    = ST_PREP;
                    end
                end

                ST_PREP: begin
                    req_next = 1'b1;
                    if (prep_cnt == 2'd0) begin
                        tx_shift_next = data_frame;
                        rx_shift_next = '0;
                        bit_cnt_next  = '0;
                        miso_next     = data_frame[31];
                        state_next    = ST_SHIFT;
                    end else begin
                        prep_cnt_next = prep_cnt - 1'b1;
                    end
                end

                ST_SHIFT: begin
                    if (sclk_fall) begin
                        rx_shift_next = {rx_shift[30:0], mosi_sync1};
                        if (bit_cnt == 6'd31) begin
                            rx_data_next  = {rx_shift[30:0], mosi_sync1};
                            rx_valid_next = 1'b1;
                            state_next    = ST_HOLD;
                        end else begin
                            bit_cnt_next  = bit_cnt + 1'b1;
                        end
                    end

                    if (sclk_fall) begin
                        tx_shift_next = {tx_shift[30:0], 1'b0};
                        miso_next     = tx_shift[30];
                    end
                end

                ST_HOLD: begin
                    state_next = ST_HOLD;
                end
            endcase
        end
    end

endmodule
