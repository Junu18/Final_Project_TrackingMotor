`timescale 1ns / 1ps

module ps2_keyboard (
    input            clk,
    input            reset,
    input            rx_done,
    input      [7:0] valid_data_keyboard,
    output reg [7:0] keyboard_data
);

    reg is_released;

    always @(posedge clk, posedge reset) begin
        if (reset) begin
            is_released   <= 1'b0;
            keyboard_data <= 8'b0;
        end else if (rx_done) begin
            if (valid_data_keyboard == 8'hF0) begin
                is_released <= 1'b1;
            end else begin
                case(valid_data_keyboard) 
                8'h1D: keyboard_data[0] <= is_released ? 0 : 1; // W 
                8'h1C: keyboard_data[1] <= is_released ? 0 : 1; // A
                8'h1B: keyboard_data[2] <= is_released ? 0 : 1; // S 
                8'h23: keyboard_data[3] <= is_released ? 0 : 1; // D 
                8'h2D: keyboard_data[4] <= is_released ? 0 : 1; // R
                8'h05: keyboard_data[5] <= is_released ? 0 : 1; // F1
                8'h06: keyboard_data[6] <= is_released ? 0 : 1; // F2
                8'h04: keyboard_data[6] <= is_released ? 0 : 1; // F3
                endcase
            end
        end
    end


endmodule
