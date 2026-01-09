/*
 * Model_Mode.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef AP_MODEL_MODEL_MODE_H_
#define AP_MODEL_MODEL_MODE_H_

#include <stdint.h>
#include "cmsis_os.h"

typedef enum {
	STOPWATCH_MODE, DISTANCE_MODE, TEMP_HUMID_MODE
} modeState_t;
typedef enum {
	EVENT_MODE = 1
} modeEvent_t;

extern osMessageQId modeEventMsgBox;

void Model_ModeInit();
void Model_SetMode(modeState_t mode);
modeState_t Model_GetMode();

#endif /* AP_MODEL_MODEL_MODE_H_ */
