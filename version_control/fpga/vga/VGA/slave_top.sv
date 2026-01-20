`timescale 1ns / 1ps

module slave_top (
    input  logic        clk,
    input  logic        reset,

    input  logic        sclk,
    input  logic        mosi,
    output logic        miso,
    input  logic        cs,

    input  logic [ 9:0] enemy_xdata,
    input  logic [ 8:0] enemy_ydata,
    input  logic [12:0] miso_etc,

    output logic [ 7:0] mortor_xdata,
    output logic [ 6:0] mortor_ydata,
    output logic [16:0] mosi_etc,
    output logic        mosi_valid
);

    logic        req;
    logic [31:0] miso_data_frame;
    logic [31:0] mosi_data_frame;

    assign mortor_xdata = mosi_data_frame[31:24];
    assign mortor_ydata = mosi_data_frame[23:17];
    assign mosi_etc     = mosi_data_frame[16:0];

    logic sclk_sync0, sclk_sync1;
    logic cs_sync0,   cs_sync1;
    logic mosi_sync0, mosi_sync1;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            sclk_sync0 <= 1'b0;
            sclk_sync1 <= 1'b0;
            cs_sync0   <= 1'b1;
            cs_sync1   <= 1'b1;
            mosi_sync0 <= 1'b0;
            mosi_sync1 <= 1'b0;
        end else begin
            sclk_sync0 <= sclk;
            sclk_sync1 <= sclk_sync0;
            cs_sync0   <= cs;
            cs_sync1   <= cs_sync0;
            mosi_sync0 <= mosi;
            mosi_sync1 <= mosi_sync0;
        end
    end

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
        .sclk      (sclk_sync1),
        .mosi      (mosi_sync1),
        .cs        (cs_sync1),
        .miso      (miso),
        .data_frame(32'h55fa_507c),
        .req       (req),
        .rx_data   (mosi_data_frame),
        .rx_valid  (mosi_valid)
    );

endmodule
