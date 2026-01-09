/*
 * Common.h
 */
#ifndef AP_COMMON_COMMON_H_
#define AP_COMMON_COMMON_H_

#include "main.h"
#include "cmsis_os.h"

/* extern을 사용하여 main.c에 있는 변수를 참조한다고 알림 */
extern osMessageQId g_queue_fpga_data;

#define TEST_INTERVAL_MS 1000

#endif /* AP_COMMON_COMMON_H_ */

