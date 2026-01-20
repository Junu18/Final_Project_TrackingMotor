`timescale 1ns / 1ps

module ps2_top (
    input        clk,
    input        reset,
    input        btnR,
    inout        PS2Clk,
    inout        PS2Data,
    input        ps2_clk_keyboard,
    input        ps2_data_keyboard,
    output [9:0] x_pos,        // 현재 화면 X 좌표 (0~639)
    output [9:0] y_pos,        // 현재 화면 Y 좌표 (0~479)
    output       click_l,        // 왼쪽 버튼 상태
    output       click_r,        // 오른쪽 버튼 상태
    output       click_m,         // 가운데 버튼 상태
    output [7:0] keyboard_data
);

    wire [7:0] w_valid_data_mouse;
    wire [7:0] w_valid_data_keyboard;
    wire [7:0] w_packet1;
    wire [7:0] w_packet2;
    wire [7:0] w_packet3;
    wire w_packet_done;
    wire w_rx_done;

    button_debounce U_BTN_DEBOUNCE (
        .clk  (clk),
        .reset(reset),
        .i_btn(btnR),
        .o_btn(btnR_debounce)
    );

    ps2_tx U_PS2_TX (
        .clk     (clk),
        .reset   (reset),
        .tx_start(btnR_debounce),
        .ps2clk  (PS2Clk),
        .ps2data (PS2Data)
    );

    ps2_rx_mouse U_PS2_RX_MOUSE (
        .clk        (clk),
        .reset      (reset),
        .ps2clk     (PS2Clk),
        .ps2data    (PS2Data),
        .rx_done    (w_rx_done),
        .valid_data (w_valid_data_mouse)
    );

    ps2_packet U_PS2_PACKET_MOUSE (
        .clk        (clk),
        .reset      (reset),
        .rx_done    (w_rx_done),
        .valid_data (w_valid_data_mouse),
        .packet1    (w_packet1),
        .packet2    (w_packet2),
        .packet3    (w_packet3),
        .packet_done(packet_done)
    );

    ps2_xy U_PS2_XY_MOUSE (
        .clk        (clk),
        .reset      (reset),
        .packet_done(packet_done),
        .packet1    (w_packet1),
        .packet2    (w_packet2),
        .packet3    (w_packet3),
        .x_pos      (x_pos),
        .y_pos      (y_pos),
        .click_l      (click_l),
        .click_r      (click_r),
        .click_m      (click_m)
    );

    ps2_rx_keyboard U_PS2_RX_KEYBOARD (
        .clk        (clk),
        .reset      (reset),
        .ps2clk     (ps2_clk_keyboard),
        .ps2data    (ps2_data_keyboard),
        .rx_done    (w_rx_done),
        .valid_data (w_valid_data_keyboard)
    );

    ps2_keyboard U_PS2_KEYBOARD (
        .clk        (clk),
        .reset      (reset),
        .rx_done    (w_rx_done),
        .valid_data_keyboard (w_valid_data_keyboard),
        .keyboard_data(keyboard_data)
    );

endmodule
