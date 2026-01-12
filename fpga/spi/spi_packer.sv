`timescale 1ns / 1ps

module spi_packer (
    input logic clk,
    input logic reset,
    /////////////////
    input logic [ 9:0] xdata,
    input logic [ 8:0] ydata,
    input logic [12:0] etc,
    /////////////////
    input logic        req,
    //////////////////
    output logic [31:0] data_frame
);

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            data_frame <= '0;
        end else begin
            if (req) begin
                data_frame <= {xdata, ydata, etc};
            end else begin
                data_frame <= data_frame;
            end
        end
    end

endmodule
