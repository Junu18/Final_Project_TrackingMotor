module ps2_bypass (
    input clk,
    input reset,
    input  PS2Clk,   // 키보드로부터 오는 클럭 (C17)
    input  PS2Data,  // 키보드로부터 오는 데이터 (B17)
    output ps2clk_keyboard,  // 마스터 보드로 보낼 Pmod 핀
    output ps2data_keyboard  // 마스터 보드로 보낼 Pmod 핀
);
    // 단순히 입력 받은 신호를 그대로 출력으로 쏴줍니다.
    assign ps2clk_keyboard  = PS2Clk;
    assign ps2data_keyboard = PS2Data;
endmodule