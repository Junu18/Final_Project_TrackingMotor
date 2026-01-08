`timescale 1ns / 1ps

module spi_packer (
    input  logic       clk,
    input  logic       reset,

    input  logic       req,
    input  logic [9:0] xdata,
    input  logic [8:0] ydata,

    output logic [15:0] xdata_tx,
    output logic [15:0] ydata_tx
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
                x_reg <= {1'b1, 5'b0, xdata};
                y_reg <= {1'b0, 6'b0, ydata};
            end
        end
    end

endmodule