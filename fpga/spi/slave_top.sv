`timescale 1ns / 1ps

module slave_top (
    /////// global
    input  logic        clk,
    input  logic        reset,
    /////// spi protocol port
    input  logic        sclk,
    input  logic        mosi,
    output logic        miso,
    input  logic        cs,
    ////// miso data
    input  logic [ 9:0] enemy_xdata,
    input  logic [ 8:0] enemy_ydata,
    input  logic [12:0] miso_etc,
    ////// mosi data
    //valid when mosi_valid is 1.
    output logic [ 7:0] mortor_xdata,
    output logic [ 6:0] mortor_ydata,
    output logic [16:0] mosi_etc,
    output logic        mosi_valid 
);

    logic        req;
    logic [31:0] miso_data_frame;
    logic [31:0] mosi_data_frame;


    spi_packer u_packer (
        .clk       (clk),
        .reset     (reset),
        .req       (req),
        .xdata     (enemy_xdata),
        .ydata     (enemy_ydata),
        .etc       (miso_etc),
        .data_frame(miso_data_frame)
    );

    spi_slave u_spi (
        .clk       (clk),
        .reset     (reset),
        .sclk      (sclk),
        .mosi      (mosi),
        .cs        (cs),
        .miso      (miso),
        .data_frame(miso_data_frame),
        .req       (req),
        .rx_data   (mosi_data_frame),
        .rx_valid  (mosi_valid)
    );

endmodule