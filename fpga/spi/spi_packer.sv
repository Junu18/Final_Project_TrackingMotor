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

    logic [15:0] x_reg;
    logic [15:0] y_reg;

    assign xdata_tx = x_reg;
    assign ydata_tx = y_reg;

    always_ff @(posedge clk, posedge reset) begin
        if (reset) begin
            x_reg <= '0;
            y_reg <= '0;
        end else begin
            if (req) begin
                data_frame <= {xdata, ydata, etc};
            end
        end
    end

endmodule
