`timescale 1ns / 1ps

module ps2_xy (
    input            clk,
    input            reset,
    input            packet_done,
    input      [7:0] packet1,      // [YV, XV, YS, XS, 1, M, R, L]
    input      [7:0] packet2,      // X Magnitude
    input      [7:0] packet3,      // Y Magnitude
    output     [9:0] mouse_x_pixel,
    output     [9:0] mouse_y_pixel,
    output           click_l,
    output           click_r,
    output           click_m
);

    reg [9:0] x_reg, y_reg;
    reg click_l_reg, click_r_reg, click_m_reg;
    reg signed [9:0] dx, dy;

    // 핵심: 음수 결과값을 안전하게 저장하기 위해 11비트 부호 있는 변수 선언
    reg signed [10:0] next_x, next_y; 

    assign mouse_x_pixel = x_reg;
    assign mouse_y_pixel = y_reg;
    assign {click_m, click_r, click_l} = {click_m_reg, click_r_reg, click_l_reg};

    // 1. 변화량 결정 (오버플로 및 부호 확장)
    always @(*) begin
        if (packet1[6]) dx = packet1[4] ? -10'd256 : 10'd255; // X 오버플로 처리
        else            dx = {{2{packet1[4]}}, packet2};     // X 부호 확장

        if (packet1[7]) dy = packet1[5] ? -10'd256 : 10'd255; // Y 오버플로 처리
        else            dy = {{2{packet1[5]}}, packet3};     // Y 부호 확장
    end

    // 2. 좌표 누적 및 경계 처리 (Teleportation 방지 로직)
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            x_reg <= 10'd320;
            y_reg <= 10'd240;
            {click_m_reg, click_r_reg, click_l_reg} <= 3'b000;
        end else if (packet_done) begin
            click_l_reg <= packet1[0]; // Left Click
            click_r_reg <= packet1[1]; // Right Click
            click_m_reg <= packet1[2]; // Middle Click

            // [수정 포인트] $signed 캐스팅을 통해 11비트 영역에서 계산 수행
            next_x = $signed({1'b0, x_reg}) + dx;
            next_y = $signed({1'b0, y_reg}) - dy; // 마우스 위쪽 이동 시 좌표 감소 (VGA 기준)

            // X축 경계 제한 (0 미만 감지 가능)
            if (next_x < 0) 
                x_reg <= 10'd0;
            else if (next_x > 10'd639) 
                x_reg <= 10'd639;
            else 
                x_reg <= next_x[9:0];

            // Y축 경계 제한 (0 미만 감지 가능)
            if (next_y < 0) 
                y_reg <= 10'd0;
            else if (next_y > 10'd479) 
                y_reg <= 10'd479;
            else 
                y_reg <= next_y[9:0];
        end
    end
endmodule