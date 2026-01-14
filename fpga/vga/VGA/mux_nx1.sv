`timescale 1ns / 1ps

module mux_nx1 (
    input logic       clk,
    input logic       reset,
    input logic [7:0] keyboard_data,
    // auto
    input logic       target_off_auto,
    input logic [3:0] r_port_auto,
    input logic [3:0] g_port_auto,
    input logic [3:0] b_port_auto,
    // miso auto
    input logic [9:0] x_coor_auto,
    input logic [9:0] y_coor_auto,
    input logic       red_detect_auto,
    input logic       shoot_auto,
    // manual
    input logic       target_off_manual,
    input logic [3:0] r_port_manual,
    input logic [3:0] g_port_manual,
    input logic [3:0] b_port_manual,
    // miso manual
    input logic [9:0] x_coor_manual,
    input logic [9:0] y_coor_manual,
    input logic       red_detect_manual,
    input logic       shoot_manual,

    output logic       target_off,
    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,
    output logic [9:0] x_coor,
    output logic [9:0] y_coor,
    output logic       red_detect,
    output logic       shoot
);

    logic toggle;
    logic f1_prev;

    wire  f1_pressed = keyboard_data[5];

    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            toggle  <= 0;
            f1_prev <= 0;
        end else begin
            f1_prev <= f1_pressed;
            if (f1_pressed == 1'b1 && f1_prev == 1'b0) begin
                toggle <= ~toggle;
            end
        end
    end

    always_comb begin
        target_off = target_off_auto;
        r_port     = r_port_auto;
        g_port     = g_port_auto;
        b_port     = b_port_auto;
        x_coor     = x_coor_auto;
        y_coor     = y_coor_auto;
        red_detect = red_detect_auto;
        shoot      = shoot_auto;
        case (toggle)
            1'b0: begin  // Auto Mode
                target_off = target_off_auto;
                r_port     = r_port_auto;
                g_port     = g_port_auto;
                b_port     = b_port_auto;
                x_coor     = x_coor_auto;
                y_coor     = y_coor_auto;
                red_detect = red_detect_auto;
                shoot      = shoot_auto;
            end
            1'b1: begin  // Manual Mode
                target_off = target_off_manual;
                r_port     = r_port_manual;
                g_port     = g_port_manual;
                b_port     = b_port_manual;
                x_coor     = x_coor_manual;
                y_coor     = y_coor_manual;
                red_detect = red_detect_manual;
                shoot      = shoot_manual;
            end
        endcase
    end

endmodule

