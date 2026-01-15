/*
 * Laser.h
 *
 *  Created on: Jan 12, 2026
 *      Author: kccistc
 */

#ifndef DRIVER_LASER_LASER_H_
#define DRIVER_LASER_LASER_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define LASER_PORT GPIOA
#define LASER_PIN GPIO_PIN_12

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint32_t pinNum;
} hlaser;

void Laser_Init	(hlaser *laser, GPIO_TypeDef *GPIOx, uint32_t pinNum);
void Laser_on	(hlaser *laser);
void Laser_off	(hlaser *laser);

#endif /* DRIVER_LASER_LASER_H_ */
