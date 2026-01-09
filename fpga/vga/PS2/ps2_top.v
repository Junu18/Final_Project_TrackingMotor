`timescale 1ns / 1ps
module ps2_top (
    input        clk,
    input        reset,
    input        btnR,
    inout        PS2Clk,
    inout        PS2Data,
    output [7:0] valid_data,
    output       led_ps2clk,
    output       led_ps2data,
    output [2:0] led_state
);

    ps2_rx U_PS2_RX (
        .clk        (clk),
        .reset      (reset),
        .ps2clk     (PS2Clk),
        .ps2data    (PS2Data),
        .led_state  (led_state),
        .valid_data (valid_data),
        .led_ps2clk (led_ps2clk),
        .led_ps2data(led_ps2data)
    );

    ps2_tx U_PS2_TX (
        .clk     (clk),
        .reset   (reset),
        .tick    (tick),
        .tx_start(btnR_debounce),
        //    .tx_data      (tx_data),
        .ps2clk  (PS2Clk),
        .ps2data (PS2Data)
    );

    tick_gen U_TICK_GEN (
        .clk (clk),
        .rst (rst),
        .tick(tick)
    );

    button_debounce U_BTN_DEBOUNCE (
        .clk  (clk),
        .reset(reset),
        .i_btn(btnR),
        .o_btn(btnR_debounce)
    );


endmodule
