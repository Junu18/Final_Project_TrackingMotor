/*
 * model.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Model_Mode.h"

modeState_t modeState;

osMessageQId modeEventMsgBox;
osMessageQDef(modeEventQueue, 4, uint16_t);

void Model_ModeInit() {
	modeState = STOPWATCH_MODE;
	modeEventMsgBox = osMessageCreate(osMessageQ(modeEventQueue), NULL);
}

void Model_SetMode(modeState_t mode) {
	modeState = mode;
}

modeState_t Model_GetMode() {
	return modeState;
}
