/*
 * ServoMotor.h
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#ifndef DRIVER_SERVOMOTOR_SERVOMOTOR_H_
#define DRIVER_SERVOMOTOR_SERVOMOTOR_H_

#include "stm32f4xx_hal.h"
#include "tim.h"
#include <stdint.h>

//x,y축 각각 제어하려면 구조체를 두 개로 만들어야하나?
// 만들필요없음 하나의 모듈로 인스턴스 2개를 선언하듯이 하나의 구조체를 두 개의 변수로 선언해서 사용함
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
