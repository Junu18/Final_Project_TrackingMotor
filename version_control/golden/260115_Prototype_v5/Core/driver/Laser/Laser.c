/*
 * Laser.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kccistc
 */

#include "Laser.h"

void Laser_Init(Laser_t *pLaser, GPIO_TypeDef *GPIOx, uint32_t pinNum)
{
	pLaser->GPIOx = GPIOx;
	pLaser->pinNum = pinNum;
}

void Laser_On(Laser_t *pLaser)
{
	HAL_GPIO_WritePin(pLaser->GPIOx, pLaser->pinNum, GPIO_PIN_SET);
}

void Laser_Off(Laser_t *pLaser)
{
	HAL_GPIO_WritePin(pLaser->GPIOx, pLaser->pinNum, GPIO_PIN_RESET);
}
