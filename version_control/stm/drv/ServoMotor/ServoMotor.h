/*
 * ServoMotor.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef DRIVER_SERVOMOTOR_SERVOMOTOR_H_
#define DRIVER_SERVOMOTOR_SERVOMOTOR_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct {
	TIM_HandleTypeDef*	htim;
	uint32_t 			channel;

	uint32_t 			min_pulse;
	uint32_t 			max_pulse;
	float 				min_angle;
	float 				max_angle;

	uint32_t 			cur_pulse;
	float 				cur_angle;
} Servo_t;

void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *htim, uint32_t channel);
void Servo_SetAngle(Servo_t *servo, float angle);
void Servo_Start(Servo_t *servo);
void Servo_AddAngle(Servo_t *servo, float step);

#endif /* DRIVER_SERVOMOTOR_SERVOMOTOR_H_ */
