`timescale 1ns / 1ps

module top (
    input  logic       clk,
    input  logic       sys_reset,
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
    output logic       target_off,

    // keyboard
    input logic ps2_clk_keyboard,
    input logic ps2_data_keyboard,

    //spi
    input  logic sclk,
    input  logic mosi,
    output logic miso,
    input  logic cs,

    //fnd
    output logic [ 3:0] fnd_com,
    output logic [ 7:0] fnd_seg
);

    //test
    logic sys_clk;
    logic locked;
    logic reset;

    // logic             sys_clk;
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

    // keyboard
    logic [ 7:0]      w_keyboard_data;

    // manual_multi 16개 타겟 배열 신호
    logic [15:0][9:0] aim_x_all;
    logic [15:0][9:0] aim_y_all;
    logic [15:0]      aim_detected_all;
    logic             aim_detect_manual;
    logic [15:0][11:0] x_min_all, x_max_all, y_min_all, y_max_all;
    logic       target_off_manual;
    logic [9:0] x_coor_manual;
    logic [9:0] y_coor_manual;
    logic       shoot_manual;

    // auto_single
    logic [9:0] aim_x, aim_y;
    logic aim_detect_auto;
    logic shoot_auto;
    logic [11:0] box_x_min, box_x_max, box_y_min, box_y_max;

    logic [3:0] r_port_auto;
    logic [3:0] g_port_auto;
    logic [3:0] b_port_auto;

    logic [3:0] r_port_manual;
    logic [3:0] g_port_manual;
    logic [3:0] b_port_manual;
    logic       target_off_auto;

    // miso signal
    logic [9:0] x_coor;
    logic [9:0] y_coor;
    logic       red_detect;
    logic       shoot;

    logic [ 7:0] mortor_xdata;
    logic [ 6:0] mortor_ydata;
    logic [16:0] mosi_etc;
    logic        mosi_valid;

    // led debug
    assign xclk = sys_clk;

    sccb U_SCCB (
        .clk  (clk),
        .reset(reset),
        .SIO_D(SIO_D),
        .SIO_C(SIO_C)
    );

    // pixel_clk_gen P_CLK_GEN (
    //     .clk  (clk),
    //     .reset(reset),
    //     .pclk (sys_clk_1)
    // );

    clk_wiz_0 u_clk_wiz (
        .clk_in1 (clk),
        .reset   (sys_reset),
        .clk_out1(sys_clk),
        .locked  (locked)
    );

    assign reset = sys_reset || ~locked; 

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

    // Pixel Mixer 연결
    pixel_mixer_manual U_PIXEL_MIXER_MANUAL (
        .img_bg(img_cam),

        .aim_x_all       (aim_x_all),
        .aim_y_all       (aim_y_all),
        .aim_detected_all(aim_detected_all),

        .box_x_min_all(x_min_all),
        .box_x_max_all(x_max_all),
        .box_y_min_all(y_min_all),
        .box_y_max_all(y_max_all),
        .x_pixel      (x_pixel),
        .y_pixel      (y_pixel),
        .keyboard_data(w_keyboard_data),

        .r_port(r_port_manual),
        .g_port(g_port_manual),
        .b_port(b_port_manual),

        .x_coor(x_coor_manual),
        .y_coor(y_coor_manual),
        .shoot (shoot_manual)
    );

    // Red Tracker 연결
    red_tracker_manual U_RED_TRACKER_MANUAL (
        .clk    (sys_clk),
        .reset  (reset),
        .v_sync (v_sync),
        .DE     (DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .data   (rData),

        .aim_x_all       (aim_x_all),
        .aim_y_all       (aim_y_all),
        .aim_detected_all(aim_detected_all),

        .x_min_all(x_min_all),
        .x_max_all(x_max_all),
        .y_min_all(y_min_all),
        .y_max_all(y_max_all),

        .aim_detect(aim_detect_manual),
        .target_off(target_off_manual)
    );

    pixel_mixer_auto U_PIXEL_MIXER_AUTO (
        .img_bg      (img_cam),
        .aim_x       (aim_x),
        .aim_y       (aim_y),
        .aim_detected(aim_detect_auto),
        .x_pixel     (x_pixel),
        .y_pixel     (y_pixel),

        .box_x_min(box_x_min),
        .box_x_max(box_x_max),
        .box_y_min(box_y_min),
        .box_y_max(box_y_max),
        .r_port   (r_port_auto),
        .g_port   (g_port_auto),
        .b_port   (b_port_auto),
        .shoot    (shoot_auto)
    );

    red_tracker_auto U_RED_TRACKER_AUTO (
        .clk    (sys_clk),
        .reset  (reset),
        .v_sync (v_sync),
        .DE     (DE),
        .x_pixel(x_pixel),
        .y_pixel(y_pixel),
        .data   (rData),

        .aim_x       (aim_x),
        .aim_y       (aim_y),
        .aim_detected(aim_detect_auto),

        .x_min_out(box_x_min),
        .x_max_out(box_x_max),
        .y_min_out(box_y_min),
        .y_max_out(box_y_max),

        .target_off(target_off_auto)
    );

    ps2_keyboard_only_top U_ps2_keyboard_only_top (
        .clk              (clk),
        .reset            (reset),
        .ps2_clk_keyboard (ps2_clk_keyboard),
        .ps2_data_keyboard(ps2_data_keyboard),
        .keyboard_data    (w_keyboard_data)
    );

    mux_nx1 U_mux_nx1 (
        .clk              (sys_clk),
        .reset            (reset),
        .keyboard_data    (w_keyboard_data),
        // auto
        .target_off_auto  (target_off_auto),
        .r_port_auto      (r_port_auto),
        .g_port_auto      (g_port_auto),
        .b_port_auto      (b_port_auto),
        // miso auto
        .x_coor_auto      (aim_x),
        .y_coor_auto      (aim_y),
        .red_detect_auto  (aim_detect_auto),
        .shoot_auto       (shoot_auto),
        // manual
        .target_off_manual(target_off_manual),
        .r_port_manual    (r_port_manual),
        .g_port_manual    (g_port_manual),
        .b_port_manual    (b_port_manual),
        // miso manual
        .x_coor_manual    (x_coor_manual),
        .y_coor_manual    (y_coor_manual),
        .red_detect_manual(aim_detect_manual),
        .shoot_manual     (shoot_manual),
        // output
        .target_off       (target_off),
        .r_port           (r_port),
        .g_port           (g_port),
        .b_port           (b_port),
        // miso output
        .x_coor           (x_coor),
        .y_coor           (y_coor),
        .red_detect       (red_detect),
        .shoot            (shoot)
    );


    slave_top U_SPI (
        .clk(sys_clk),
        .reset(reset),
        .sclk(sclk),
        .mosi(mosi),
        .miso(miso),
        .cs(cs),
        .enemy_xdata(x_coor),
        .enemy_ydata(y_coor[8:0]),
        .miso_etc({red_detect,shoot,target_off,10'b00_0000_0000}),
        .mortor_xdata(mortor_xdata),
        .mortor_ydata(mortor_ydata),
        .mosi_etc(mosi_etc),
        .mosi_valid(mosi_valid)
    );


    fnd_controller U_FND(
        .clk(clk),
        .reset(reset),
        .cnt_data({4'b0000,x_coor}), // 14BIT
        .fnd_com(fnd_com),
        .fnd_seg(fnd_seg)
    );

endmodule
