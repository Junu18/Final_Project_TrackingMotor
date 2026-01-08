`timescale 1ns / 1ps

module slave_top (
    input  logic clk,
    input  logic reset,

    input  logic sclk,
    input  logic mosi,
    output logic miso,
    input  logic cs
);

    logic        req;
    logic [15:0] xdata_tx;
    logic [15:0] ydata_tx;

    logic [9:0] xdata = 10'b1111000010;
    logic [8:0] ydata = 9'b110011001;

    spi_packer u_packer (
        .clk      (clk),
        .reset    (reset),
        .req      (req),
        .xdata    (xdata),
        .ydata    (ydata),
        .xdata_tx (xdata_tx),
        .ydata_tx (ydata_tx)
    );

    spi_slave u_spi (
        .clk      (clk),
        .reset    (reset),
        .sclk     (sclk),
        .mosi     (mosi),
        .cs       (cs),
        .miso     (miso),
        .xdata_tx (xdata_tx),
        .ydata_tx (ydata_tx),
        .req      (req)
    );

endmodule