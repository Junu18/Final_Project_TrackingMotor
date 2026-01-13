/*
 * Common.h
 */
#ifndef AP_COMMON_COMMON_H_
#define AP_COMMON_COMMON_H_

#include "main.h"
#include "cmsis_os.h"

/* ========================================
 * FreeRTOS Queues (main.c에서 정의)
 * ======================================== */
extern osMessageQId g_queue_fpga_data;   // SPI Callback → Listener
extern osMessageQId g_queue_xy_coord;    // Listener → Controller (추후 사용)
extern osMessageQId g_queue_angle;       // Controller → Presenter (추후 사용)

/* ========================================
 * Task Handles (main.c에서 정의)
 * ======================================== */
extern osThreadId Listener_TaskHandle;
extern osThreadId Controller_TaskHandle;
extern osThreadId Presenter_TaskHandle;

/* ========================================
 * 설정 상수
 * ======================================== */
#define TEST_INTERVAL_MS 1000

#endif /* AP_COMMON_COMMON_H_ */

