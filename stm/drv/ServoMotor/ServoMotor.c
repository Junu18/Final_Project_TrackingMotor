/*
 * ServoMotor.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "ServoMotor.h"

void Servo_Init(Servo_t *servo, TIM_HandleTypeDef *htim, uint32_t channel) {
	servo->htim = htim;
	servo->channel = channel;

	servo->min_angle = 0.0f;
	servo->max_angle = 180.0f;
	servo->min_pulse = 600;
	servo->max_pulse = 2400;

	Servo_SetAngle(servo, 90.0f);
}

void Servo_SetAngle(Servo_t *servo, float angle) {
	if (angle < servo->min_angle)
		angle = servo->min_angle;
	if (angle > servo->max_angle)
		angle = servo->max_angle;
	servo->cur_angle = angle;

	uint32_t pulse = (uint32_t) ((angle - servo->min_angle)
			* (servo->max_pulse - servo->min_pulse)
			/ (servo->max_angle - servo->min_angle) + servo->min_pulse);
	servo->cur_pulse = pulse;

	__HAL_TIM_SET_COMPARE(servo->htim, servo->channel, pulse);
}

void Servo_Start(Servo_t *servo) {
	HAL_TIM_PWM_Start(servo->htim, servo->channel);
}

void Servo_AddAngle(Servo_t *servo, float step) {
    float target_angle = servo->cur_angle + step;
    servo->cur_angle = target_angle;
    Servo_SetAngle(servo, target_angle);
}
