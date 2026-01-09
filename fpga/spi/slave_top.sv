`timescale 1ns / 1ps

module slave_top (
    input  logic        clk,
    input  logic        reset,
    ///////
    input  logic        sclk,
    input  logic        mosi,
    output logic        miso,
    input  logic        cs,
    //////
    input  logic [ 9:0] xdata,
    input  logic [ 8:0] ydata,
    input  logic [12:0] etc
);

    logic        req;
    logic [31:0] data_frame;

    spi_packer u_packer (
        .clk       (clk),
        .reset     (reset),
        .req       (req),
        .xdata     (xdata),
        .ydata     (ydata),
        .etc       (etc),
        .data_frame(data_frame)
    );

    spi_slave u_spi (
        .clk     (clk),
        .reset   (reset),
        .sclk    (sclk),
        .mosi    (mosi),
        .cs      (cs),
        .miso    (miso),
        .data_frame(data_frame),
        .req     (req)
    );

endmodule
