`timescale 1ns / 1ps

module ps2_xy (
    input            clk,          // 시스템 클럭 (100MHz)
    input            reset,        // 리셋 신호
    input            packet_done,  // ps2_packet 모듈에서 3바이트 수집 완료 신호
    input      [7:0] packet1,      // Status Byte: [YV, XV, YS, XS, 1, M, R, L]
    input      [7:0] packet2,      // X Movement Magnitude
    input      [7:0] packet3,      // Y Movement Magnitude
    output     [9:0] x_pos,        // 현재 화면 X 좌표 (0~639)
    output     [9:0] y_pos,        // 현재 화면 Y 좌표 (0~479)
    output           click_l,        // 왼쪽 버튼 상태
    output           click_r,        // 오른쪽 버튼 상태
    output           click_m         // 가운데 버튼 상태
);

    // 내부 레지스터 선언
    reg [9:0] x_reg, y_reg;
    reg click_l_reg, click_r_reg, click_m_reg;
    
    // 변화량 계산을 위한 부호 있는 10비트 변수
    reg signed [9:0] dx, dy;

    // 출력 연결
    assign x_pos = x_reg;
    assign y_pos = y_reg;
    assign click_l = click_l_reg;
    assign click_r = click_r_reg;
    assign click_m = click_m_reg;

    // 1. 오버플로 및 부호 비트를 고려한 실제 변화량(dx, dy) 결정
    always @(*) begin
        // X축 변화량 처리
        if (packet1[6]) begin // X Overflow 발생 시
            dx = packet1[4] ? -10'd256 : 10'd255; // 음수면 최솟값, 양수면 최댓값 고정
        end else begin
            dx = {{2{packet1[4]}}, packet2}; // XS 비트를 이용한 10비트 부호 확장
        end

        // Y축 변화량 처리
        if (packet1[7]) begin // Y Overflow 발생 시
            dy = packet1[5] ? -10'd256 : 10'd255;
        end else begin
            dy = {{2{packet1[5]}}, packet3}; // YS 비트를 이용한 10비트 부호 확장
        end
    end

    // 2. 좌표 누적 및 경계 처리 (Clamping)
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            x_reg <= 10'd320; // 화면 중앙에서 시작
            y_reg <= 10'd240;
            click_l_reg <= 1'b0;
            click_r_reg <= 1'b0;
            click_m_reg <= 1'b0;
        end else if (packet_done) begin
            // 버튼 상태 업데이트
            click_l_reg <= packet1[0];
            click_r_reg <= packet1[1];
            click_m_reg <= packet1[2];

            // X 좌표 계산 및 화면 경계 제한 (0 ~ 639)
            if (x_reg + dx < 0) 
                x_reg <= 10'd0;
            else if (x_reg + dx > 10'd639) 
                x_reg <= 10'd639;
            else 
                x_reg <= x_reg + dx;

            // Y 좌표 계산 및 화면 경계 제한 (0 ~ 479)
            // PS/2는 위쪽 이동이 (+)이나 VGA 좌표계는 아래쪽이 (+)이므로 dy를 뺍니다.
            if (y_reg - dy < 0) 
                y_reg <= 10'd0;
            else if (y_reg - dy > 10'd479) 
                y_reg <= 10'd479;
            else 
                y_reg <= y_reg - dy;
        end
    end

endmodule