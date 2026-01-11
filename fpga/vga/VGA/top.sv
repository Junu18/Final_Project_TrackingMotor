`timescale 1ns / 1ps

module top (
    input  logic       clk,
    input  logic       reset,
    output logic       xclk,
    input  logic       pclk,
    input  logic       href,
    input  logic       vsync,
    input  logic [7:0] data,
    input  logic       btnR,
    output logic       h_sync,
    output logic       v_sync,
    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,
    output logic       SIO_C,
    output logic       SIO_D,
    output logic       aim_detected_led,   // LED[0]
    output logic       target_locked_led,  // LED[14]
    output logic       target_off,         // LED[15]

    inout logic PS2Clk,
    inout logic PS2Data,

    output logic debug_ps2clk,
    output logic debug_ps2data
);

    logic             sys_clk;
    logic             DE;
    logic [ 9:0]      x_pixel;
    logic [ 9:0]      y_pixel;
    logic             wclk;
    logic             we;
    logic [16:0]      wAddr;
    logic [15:0]      wData;
    logic             rclk;
    logic             oe;
    logic [16:0]      rAddr;
    logic [15:0]      rData;
    logic [11:0]      img_cam;

    // 16개 타겟 정보
    logic [15:0][9:0] aim_x_all;
    logic [15:0][9:0] aim_y_all;
    logic [15:0]      aim_detected_all;
    logic [15:0][11:0] x_min_all, x_max_all, y_min_all, y_max_all;

    // 마우스 정보
    logic [9:0] mouse_x, mouse_y;
    logic click_l, click_r, click_m;

    // 락온 제어 정보 (Controller <-> Pixel Mixer)
    logic       is_locked;
    logic [3:0] locked_idx;
    logic       center_hit;

    assign xclk = sys_clk;

    // --- 1. System & Camera Modules ---
    sccb U_SCCB (
        .clk  (clk),
        .reset(reset),
        .SIO_D(SIO_D),
        .SIO_C(SIO_C)
    );
    pixel_clk_gen P_CLK_GEN (
        .clk  (clk),
        .reset(reset),
        .pclk (sys_clk)
    );

    VGA_Syncher U_VGA_SYNCHER (
        .clk(sys_clk),
        .reset(reset),
        .h_sync(h_sync),
        .v_sync(v_sync),
        .DE(DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel)
    );

    image_reader U_IMG_ROM_READER (
        .DE(DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .addr(rAddr),
        .imgData(rData),
        .r_port(img_cam[11:8]),
        .g_port(img_cam[7:4]),
        .b_port(img_cam[3:0])
    );

    frame_buffer U_FRAME_BUFFER (
        .wclk(pclk),
        .we(we),
        .wAddr(wAddr),
        .wData(wData),
        .rclk(sys_clk),
        .oe(1'b1),
        .rAddr(rAddr),
        .rData(rData)
    );

    OV7670_controller U_OV7670_MEM_CONTROLLER (
        .pclk(pclk),
        .reset(reset),
        .href(href),
        .vsync(vsync),
        .data(data),
        .we(we),
        .wAddr(wAddr),
        .wData(wData)
    );

    // --- 2. Mouse Input Module ---
    ps2_top U_PS2_TOP (
        .clk(clk),
        .reset(reset),
        .btnR(btnR),
        .PS2Clk(PS2Clk),
        .PS2Data(PS2Data),
        .mouse_x_pixel(mouse_x),
        .mouse_y_pixel(mouse_y),
        .click_l(click_l),
        .click_r(click_r),
        .click_m(click_m),
        .debug_ps2clk(debug_ps2clk),
        .debug_ps2data(debug_ps2data)
    );

    // --- 3. Vision Processing (Red Tracker) ---
    red_tracker U_RED_TRACKER (
        .clk(sys_clk),
        .reset(reset),
        .v_sync(v_sync),
        .DE(DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .data(rData),

        .aim_x_all(aim_x_all),
        .aim_y_all(aim_y_all),
        .aim_detected_all(aim_detected_all),
        .x_min_all(x_min_all),
        .x_max_all(x_max_all),
        .y_min_all(y_min_all),
        .y_max_all(y_max_all),
        .target_off(target_off)
    );

    // --- 4. Target Control Logic (FSM & Handover) ---
    target_controller U_TARGET_CONTROLLER (
        .clk  (sys_clk),
        .reset(reset),

        // Mouse Input
        .mouse_x(mouse_x),
        .mouse_y(mouse_y),
        .click_l(click_l),
        .click_r(click_r),

        // Tracker Input
        .aim_x_all(aim_x_all),
        .aim_y_all(aim_y_all),
        .aim_detected_all(aim_detected_all),
        .x_min_all(x_min_all),
        .x_max_all(x_max_all),
        .y_min_all(y_min_all),
        .y_max_all(y_max_all),

        // Outputs
        .is_locked(is_locked),
        .locked_idx(locked_idx),
        .center_hit(center_hit),
        .aim_detected_led(aim_detected_led),
        .target_locked_led(target_locked_led)
    );

    // --- 5. UI Display (Pixel Mixer) ---
    pixel_mixer U_PIXEL_MIXER (
        .img_bg(img_cam),

        .aim_x_all(aim_x_all),
        .aim_y_all(aim_y_all),
        .aim_detected_all(aim_detected_all),
        .box_x_min_all(x_min_all),
        .box_x_max_all(x_max_all),
        .box_y_min_all(y_min_all),
        .box_y_max_all(y_max_all),

        .mouse_x_pixel(mouse_x),
        .mouse_y_pixel(mouse_y),
        .click_l(click_l),
        .click_r(click_r),
        .click_m(click_m),

        // Controller에서 온 신호들
        .is_locked (is_locked),
        .locked_idx(locked_idx),
        .center_hit(center_hit),

        .target_off(target_off),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .r_port(r_port),
        .g_port(g_port),
        .b_port(b_port),

        // stm에 보낼 최종 좌표
        .target_x_coor(),
        .target_y_coor()
    );

endmodule
