`timescale 1ns / 1ps

module ps2_keyboard_only_top (
    input        clk,
    input        reset,
    input        ps2_clk_keyboard,
    input        ps2_data_keyboard,
    output [7:0] keyboard_data
);

    wire [7:0] w_valid_data_keyboard;
    wire w_rx_done_keyboard;


    ps2_rx_keyboard U_PS2_RX_KEYBOARD (
        .clk        (clk),
        .reset      (reset),
        .ps2clk     (ps2_clk_keyboard),
        .ps2data    (ps2_data_keyboard),
        .rx_done    (w_rx_done_keyboard),
        .valid_data (w_valid_data_keyboard)
    );

    ps2_keyboard U_PS2_KEYBOARD (
        .clk        (clk),
        .reset      (reset),
        .rx_done    (w_rx_done_keyboard),
        .valid_data_keyboard (w_valid_data_keyboard),
        .keyboard_data(keyboard_data)
    );

endmodule
