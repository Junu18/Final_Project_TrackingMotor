`timescale 1ns / 1ps

module Font_Rom (
    input  logic [6:0] char_code, // ASCII 코드
    input  logic [3:0] row_addr,  // 글자의 몇 번째 줄인지 (0~15)
    output logic [7:0] row_data   // 그 줄의 픽셀 데이터 (1=글자, 0=X)
);

    always_comb begin
        case (char_code)
            // --- 알파벳  ---
            7'h41: case(row_addr) // A
                0: row_data = 8'h18; 1: row_data = 8'h24; 2: row_data = 8'h42; 3: row_data = 8'h42;
                4: row_data = 8'h7E; 5: row_data = 8'h42; 6: row_data = 8'h42; 7: row_data = 8'h42;
                default: row_data = 0; endcase
            7'h43: case(row_addr) // C
                0: row_data = 8'h3C; 1: row_data = 8'h42; 2: row_data = 8'h40; 3: row_data = 8'h40;
                4: row_data = 8'h40; 5: row_data = 8'h40; 6: row_data = 8'h42; 7: row_data = 8'h3C;
                default: row_data = 0; endcase
            7'h45: case(row_addr) // E
                0: row_data = 8'h7E; 1: row_data = 8'h40; 2: row_data = 8'h40; 3: row_data = 8'h7C;
                4: row_data = 8'h40; 5: row_data = 8'h40; 6: row_data = 8'h40; 7: row_data = 8'h7E;
                default: row_data = 0; endcase
            7'h47: case(row_addr) // G
                0: row_data = 8'h3C; 1: row_data = 8'h42; 2: row_data = 8'h40; 3: row_data = 8'h40;
                4: row_data = 8'h4E; 5: row_data = 8'h42; 6: row_data = 8'h42; 7: row_data = 8'h3C;
                default: row_data = 0; endcase
            7'h48: case(row_addr) // H
                0: row_data = 8'h42; 1: row_data = 8'h42; 2: row_data = 8'h42; 3: row_data = 8'h7E;
                4: row_data = 8'h42; 5: row_data = 8'h42; 6: row_data = 8'h42; 7: row_data = 8'h42;
                default: row_data = 0; endcase
            7'h49: case(row_addr) // I
                0: row_data = 8'h3C; 1: row_data = 8'h18; 2: row_data = 8'h18; 3: row_data = 8'h18;
                4: row_data = 8'h18; 5: row_data = 8'h18; 6: row_data = 8'h18; 7: row_data = 8'h3C;
                default: row_data = 0; endcase
            7'h4E: case(row_addr) // N
                0: row_data = 8'h42; 1: row_data = 8'h62; 2: row_data = 8'h52; 3: row_data = 8'h4A;
                4: row_data = 8'h46; 5: row_data = 8'h42; 6: row_data = 8'h42; 7: row_data = 8'h42;
                default: row_data = 0; endcase
            7'h53: case(row_addr) // S
                0: row_data = 8'h3C; 1: row_data = 8'h42; 2: row_data = 8'h40; 3: row_data = 8'h3C;
                4: row_data = 8'h02; 5: row_data = 8'h42; 6: row_data = 8'h42; 7: row_data = 8'h3C;
                default: row_data = 0; endcase
            7'h54: case(row_addr) // T
                0: row_data = 8'h7E; 1: row_data = 8'h18; 2: row_data = 8'h18; 3: row_data = 8'h18;
                4: row_data = 8'h18; 5: row_data = 8'h18; 6: row_data = 8'h18; 7: row_data = 8'h18;
                default: row_data = 0; endcase
            7'h57: case(row_addr) // W
                0: row_data = 8'h92; 1: row_data = 8'h92; 2: row_data = 8'h92; 3: row_data = 8'h92;
                4: row_data = 8'h92; 5: row_data = 8'h92; 6: row_data = 8'h92; 7: row_data = 8'h54;
                default: row_data = 0; endcase
            
            // --- 공백 (Space) ---
            default: row_data = 8'h00; 
        endcase
    end
endmodule
