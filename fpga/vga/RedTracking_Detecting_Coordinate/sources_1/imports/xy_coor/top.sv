`timescale 1ns / 1ps

module top (
    // global side
    input  logic       clk,
    input  logic       reset,
    // OV7670 side
    output logic       xclk,
    input  logic       pclk,
    input  logic       href,
    input  logic       vsync,
    input  logic [7:0] data,
    // VGA side
    output logic       h_sync,
    output logic       v_sync,
    output logic [3:0] r_port,
    output logic [3:0] g_port,
    output logic [3:0] b_port,
    // SCCB side
    output logic       SIO_C,
    output logic       SIO_D,

    output logic aim_detected_led,
    output logic raser_shoot_led,

    output logic target_off,
    // SPI side
    input  logic mosi,
    output logic miso,
    input  logic sclk,
    input  logic cs
);


    // global side
    logic        sys_clk;

    // VGA side
    logic        DE;
    logic [ 9:0] x_pixel;
    logic [ 9:0] y_pixel;

    // OV7670 side
    logic        wclk;
    logic        we;
    logic [16:0] wAddr;  // [$clog2(320*240)-1:0] 
    logic [15:0] wData;
    logic        rclk;
    logic        oe;
    logic [16:0] rAddr;  // [$clog2(320*240)-1:0] 
    logic [15:0] rData;  //imgData of IMG_MEM_READER

    // img side
    logic [ 1:0] bg_sel;
    logic [11:0] img_cam, img_bg;

    logic [9:0] aim_x, aim_y;
    logic aim_detected;

    logic [11:0] box_x_min, box_x_max, box_y_min, box_y_max;

    // led debug
    assign aim_detected_led = aim_detected;
    assign raser_shoot_led = raser_shoot;

    assign xclk = sys_clk;

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
        .clk    (sys_clk),
        .reset  (reset),
        .h_sync (h_sync),
        .v_sync (v_sync),
        .DE     (DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel)
    );

    image_reader U_IMG_ROM_READER (
        .DE     (DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .addr   (rAddr),
        .imgData(rData),
        .r_port (img_cam[11:8]),
        .g_port (img_cam[7:4]),
        .b_port (img_cam[3:0])
    );

    pixel_mixer U_PIXEL_MIXER (
        .img_bg      (img_cam),
        .aim_x       (aim_x),
        .aim_y       (aim_y),
        .aim_detected(aim_detected),
        .x_pixel     (x_pixel),
        .y_pixel     (y_pixel),
        .box_x_min   (box_x_min),
        .box_x_max   (box_x_max),
        .box_y_min   (box_y_min),
        .box_y_max   (box_y_max),
        .r_port      (r_port),
        .g_port      (g_port),
        .b_port      (b_port)
    );

    frame_buffer U_FRAME_BUFFER (
        // write side
        .wclk (pclk),
        .we   (we),
        .wAddr(wAddr),  // [$clog2(320*240)-1:0] 
        .wData(wData),
        // read side 
        .rclk (sys_clk),
        .oe   (1'b1),
        .rAddr(rAddr),  // [$clog2(320*240)-1:0] 
        .rData(rData)
    );

    OV7670_controller U_OV7670_MEM_CONTROLLER (
        .pclk (pclk),
        .reset(reset),
        // OV7670 side
        .href (href),
        .vsync(vsync),
        .data (data),
        // memory side
        .we   (we),
        .wAddr(wAddr),  // 320 * 240
        .wData(wData)
    );

    red_tracker U_RED_TRACKER (
        .clk         (sys_clk),
        .reset       (reset),
        .v_sync      (v_sync),
        .DE          (DE),
        .x_pixel     (x_pixel),
        .y_pixel     (y_pixel),
        .data        (rData),
        .aim_x       (aim_x),
        .aim_y       (aim_y),
        .aim_detected(aim_detected),
        .raser_shoot (raser_shoot),
        .x_min_out   (box_x_min),
        .x_max_out   (box_x_max),
        .y_min_out   (box_y_min),
        .y_max_out   (box_y_max),
        .target_off  (target_off)
    );

    slave_top U_SPI_Slave (
        .clk  (clk),
        .reset(reset),
        .sclk (sclk),
        .mosi (mosi),
        .miso (miso),
        .cs   (cs),
        .xdata(aim_x),
        .ydata(aim_y[8:0]),
        .etc  (13'b0_0000_0000_0000)
    );

endmodule
