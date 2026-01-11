`timescale 1ns / 1ps

module ps2_packet (
    input        clk,
    input        reset,
    input        rx_done,
    input  [7:0] valid_data,
    output [7:0] packet1,
    output [7:0] packet2,
    output [7:0] packet3,
    output       packet_done
);

    localparam PKT_IDLE = 0;
    localparam PKT_X = 1;
    localparam PKT_Y = 2;
    localparam PKT_DONE = 3;

    reg [7:0] packet1_reg, packet1_next;
    reg [7:0] packet2_reg, packet2_next;
    reg [7:0] packet3_reg, packet3_next;

    reg [1:0] pkt_state_reg, pkt_state_next;
    reg packet_done_reg, packet_done_next;

    assign packet1 = packet1_reg;
    assign packet2 = packet2_reg;
    assign packet3 = packet3_reg;
    assign packet_done = packet_done_reg;


    always @(posedge clk, posedge reset) begin
        if (reset) begin
            pkt_state_reg <= PKT_IDLE;
            packet1_reg <= 8'h00;
            packet2_reg <= 8'h00;
            packet3_reg <= 8'h00;
            packet_done_reg <= 0;
        end else begin
            pkt_state_reg <= pkt_state_next;
            packet1_reg <= packet1_next;
            packet2_reg <= packet2_next;
            packet3_reg <= packet3_next;
            packet_done_reg <= packet_done_next;
        end
    end

    always @(*) begin
        pkt_state_next = pkt_state_reg;
        packet1_next = packet1_reg;
        packet2_next = packet2_reg;
        packet3_next = packet3_reg;
        packet_done_next = 1'b0;
        case (pkt_state_reg)
            PKT_IDLE: begin
                if (rx_done) begin
                    if (valid_data[3] == 1) begin
                        packet1_next   = valid_data;
                        pkt_state_next = PKT_X;
                    end
                end
            end
            PKT_X: begin
                if (rx_done) begin
                    packet2_next   = valid_data;
                    pkt_state_next = PKT_Y;
                end
            end
            PKT_Y: begin
                if (rx_done) begin
                    packet3_next = valid_data;
                    packet_done_next = 1;
                    pkt_state_next = PKT_DONE;
                end
            end
            PKT_DONE: begin
                packet_done_next = 1;
                pkt_state_next   = PKT_IDLE;
            end
        endcase
    end

endmodule
