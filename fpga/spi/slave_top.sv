`timescale 1ns / 1ps

module slave_top (
    input logic clk,
    input logic reset,

    input  logic sclk,
    input  logic mosi,
    output logic miso,
    input  logic cs,

    input logic [9:0] xdata,
    input logic [8:0] ydata
);

    logic        req;
    logic [15:0] xdata_tx;
    logic [15:0] ydata_tx;

    spi_packer u_packer (
        .clk     (clk),
        .reset   (reset),
        .req     (req),
        .xdata   (xdata),
        .ydata   (ydata),
        .xdata_tx(xdata_tx),
        .ydata_tx(ydata_tx)
    );

    spi_slave u_spi (
        .clk     (clk),
        .reset   (reset),
        .sclk    (sclk),
        .mosi    (mosi),
        .cs      (cs),
        .miso    (miso),
        .xdata_tx(xdata_tx),
        .ydata_tx(ydata_tx),
        .req     (req)
    );

endmodule
