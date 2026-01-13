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
<<<<<<< HEAD
    output logic       target_off,

    // keyboard
    input logic ps2_clk_keyboard,
    input logic ps2_data_keyboard
=======
    output logic       target_locked_led,  // LED[14]
    output logic       target_off,         // LED[15]

    inout logic PS2Clk,
    inout logic PS2Data,

    output logic debug_ps2clk,
    output logic debug_ps2data,
    input  logic sclk,
    input  logic mosi,
    output logic miso,
    input  logic cs,

    output logic [3:0] fnd_com,
    output logic [7:0] fnd_seg
>>>>>>> 09cd8b49095b117816de4e7bdcde7f1b29e98cce
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

    // keyboard
    logic [ 7:0]      w_keyboard_data;

    // manual_multi 16개 타겟 배열 신호
    logic [15:0][9:0] aim_x_all;
    logic [15:0][9:0] aim_y_all;
    logic [15:0]      aim_detected_all;
    logic [15:0][11:0] x_min_all, x_max_all, y_min_all, y_max_all;
    logic target_off_manual;

    // auto_single
    logic [9:0] aim_x, aim_y;
    logic aim_detected;
    logic [11:0] box_x_min, box_x_max, box_y_min, box_y_max;

<<<<<<< HEAD
    logic [3:0] r_port_auto;
    logic [3:0] g_port_auto;
    logic [3:0] b_port_auto;
=======
    // 락온 제어 정보 (Controller <-> Pixel Mixer)
    logic        is_locked;
    logic [ 3:0] locked_idx;
    logic        center_hit;

    /////predict module port
    logic [15:0] x_real_in;
    logic [15:0] y_real_in;
    logic [15:0] final_enemy_xdata;
    logic [15:0] final_enemy_ydata;


    ///spi module
    logic [ 7:0] mortor_xdata;
    logic [ 6:0] mortor_ydata;
    logic        mosi_valid;
>>>>>>> 09cd8b49095b117816de4e7bdcde7f1b29e98cce

    logic [3:0] r_port_manual;
    logic [3:0] g_port_manual;
    logic [3:0] b_port_manual;
    logic       target_off_auto;

    // led debug
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

<<<<<<< HEAD
        .r_port(r_port_manual),
        .g_port(g_port_manual),
        .b_port(b_port_manual)
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
=======
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
        .target_x_coor(x_real_in),
        .target_y_coor(y_real_in)
    );


    enemy_predict_2s U_Predict (
        .clk(clk),
        .reset(reset),
        .x_meas_now(x_real_in),
        .y_meas_now(y_real_in),
        .red_detect_in(aim_detected_led),
        .x_predict_out(final_enemy_xdata),
        .y_predict_out(final_enemy_ydata),
        .vir_red_detect(vir_red_detect)
    );


    slave_top U_SPI_Slave (
        .clk(clk),
        .reset(reset),
        /////// spi protocol port
        .sclk(sclk),
        .mosi(mosi),
        .miso(miso),
        .cs(cs),
        ////// miso data
        .enemy_xdata(final_enemy_xdata),
        .enemy_ydata(final_enemy_ydata),
        .miso_etc({vir_red_detect, raser_shoot, 11'b000_0000_0000}),
        ////// mosi data
        .mortor_xdata(mortor_xdata),
        .mortor_ydata(mortor_ydata),
        .mosi_valid(mosi_valid),  //valid when mosi_valid is 1.
        .mosi_etc(17'b0_0000_0000_0000_0000)
>>>>>>> 09cd8b49095b117816de4e7bdcde7f1b29e98cce

        .x_min_all(x_min_all),
        .x_max_all(x_max_all),
        .y_min_all(y_min_all),
        .y_max_all(y_max_all),

        .target_off(target_off_manual)
    );

    pixel_mixer_auto U_PIXEL_MIXER_AUTO (
        .img_bg      (img_cam),
        .aim_x       (aim_x),
        .aim_y       (aim_y),
        .aim_detected(aim_detected),
        .x_pixel     (x_pixel),
        .y_pixel     (y_pixel),

        .box_x_min(box_x_min),
        .box_x_max(box_x_max),
        .box_y_min(box_y_min),
        .box_y_max(box_y_max),
        .r_port   (r_port_auto),
        .g_port   (g_port_auto),
        .b_port   (b_port_auto)
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
        .aim_detected(aim_detected),

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
        .target_off_auto  (target_off_auto),
        .target_off_manual(target_off_manual),
        .r_port_auto      (r_port_auto),
        .g_port_auto      (g_port_auto),
        .b_port_auto      (b_port_auto),
        .r_port_manual    (r_port_manual),
        .g_port_manual    (g_port_manual),
        .b_port_manual    (b_port_manual),

        .target_off(target_off),
        .r_port    (r_port),
        .g_port    (g_port),
        .b_port    (b_port)
    );

    fnd_controller U_FND (
        .clk(clk),
        .reset(reset),
        .cnt_data({6'b0,mortor_xdata}),
        .fnd_com(fnd_com),
        .fnd_seg(fnd_seg)
    );

endmodule
