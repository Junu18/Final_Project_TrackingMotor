`timescale 1ns / 1ps

module top (
    input  logic       clk,
    input  logic       reset,
    output logic       xclk,
    input  logic       pclk,
    input  logic       href,
    input  logic       vsync,
    input  logic [7:0] data,
    output logic       h_sync,
    output logic       v_sync,
    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,
    output logic       SIO_C,
    output logic       SIO_D,
    output logic       aim_detected_led,
    output logic       target_off
);

    logic                     sys_clk;
    logic                     DE;
    logic [              9:0] x_pixel;
    logic [              9:0] y_pixel;
    logic                     wclk;
    logic                     we;
    logic [             16:0] wAddr; 
    logic [             15:0] wData; 
    logic                     rclk;
    logic                     oe;
    logic [             16:0] rAddr; 
    logic [             15:0] rData; 
    logic [11:0] img_cam;
    
    // 16개 타겟 배열 신호
    logic [15:0][9:0] aim_x_all;
    logic [15:0][9:0] aim_y_all;
    logic [15:0]      aim_detected_all;
    
    // 16개 박스 배열 신호
    logic [15:0][11:0] x_min_all, x_max_all, y_min_all, y_max_all;

    // led debug
    assign aim_detected_led = (aim_detected_all != 0);
    assign xclk = sys_clk;

    // (기존 모듈들 유지)
    sccb U_SCCB (.clk(clk), .reset(reset), .SIO_D(SIO_D), .SIO_C(SIO_C));
    pixel_clk_gen P_CLK_GEN (.clk(clk), .reset(reset), .pclk(sys_clk));
    VGA_Syncher U_VGA_SYNCHER (.clk(sys_clk), .reset(reset), .h_sync(h_sync), .v_sync(v_sync), .DE(DE), .x_pixel(x_pixel), .y_pixel(y_pixel));
    image_reader U_IMG_ROM_READER (.DE(DE), .x_pixel(x_pixel), .y_pixel(y_pixel), .addr(rAddr), .imgData(rData), .r_port(img_cam[11:8]), .g_port(img_cam[7:4]), .b_port(img_cam[3:0]));
    frame_buffer U_FRAME_BUFFER (.wclk(pclk), .we(we), .wAddr(wAddr), .wData(wData), .rclk(sys_clk), .oe(1'b1), .rAddr(rAddr), .rData(rData));
    OV7670_controller U_OV7670_MEM_CONTROLLER (.pclk(pclk), .reset(reset), .href(href), .vsync(vsync), .data(data), .we(we), .wAddr(wAddr), .wData(wData));

    // [수정] Pixel Mixer 연결
    pixel_mixer U_PIXEL_MIXER (
        .img_bg        (img_cam),
        
        .aim_x_all       (aim_x_all),
        .aim_y_all       (aim_y_all),
        .aim_detected_all(aim_detected_all),
        
        .box_x_min_all (x_min_all), .box_x_max_all (x_max_all),
        .box_y_min_all (y_min_all), .box_y_max_all (y_max_all),
        
        .target_off      (target_off),
        .x_pixel         (x_pixel),
        .y_pixel         (y_pixel),
        .r_port(r_port), .g_port(g_port), .b_port(b_port)
    );

    // [수정] Red Tracker 연결
    red_tracker U_RED_TRACKER (
        .clk         (sys_clk),
        .reset       (reset),
        .v_sync      (v_sync),
        .DE          (DE),
        .x_pixel     (x_pixel),
        .y_pixel     (y_pixel),
        .data        (rData),
        
        .aim_x_all       (aim_x_all),
        .aim_y_all       (aim_y_all),
        .aim_detected_all(aim_detected_all),
        
        .x_min_all(x_min_all), .x_max_all(x_max_all),
        .y_min_all(y_min_all), .y_max_all(y_max_all),
        
        .target_off      (target_off)
    );

endmodule