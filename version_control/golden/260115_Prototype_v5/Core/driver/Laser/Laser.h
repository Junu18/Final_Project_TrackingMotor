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

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint32_t pinNum;
} Laser_t;

void Laser_Init	(Laser_t *pLaser, GPIO_TypeDef *GPIOx, uint32_t pinNum);
void Laser_On	(Laser_t *pLaser);
void Laser_Off	(Laser_t *pLaser);

#endif /* DRIVER_LASER_LASER_H_ */
