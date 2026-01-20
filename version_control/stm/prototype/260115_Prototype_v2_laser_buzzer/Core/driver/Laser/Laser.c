/*
 * Laser.c
 *
 *  Created on: Jan 12, 2026
 *      Author: kccistc
 */

#include "Laser.h"

void Laser_Init(hlaser *laser, GPIO_TypeDef *GPIOx, uint32_t pinNum)
{
	//하두웨어 연결정보 저장
	laser->GPIOx = GPIOx;
	laser->pinNum = pinNum;

	//초기상태 설정(시작은 항상 꺼진상태로)
	HAL_GPIO_WritePin(LASER_PORT, laser->pinNum, GPIO_PIN_RESET);
}

void Laser_on(hlaser *laser)
{
	HAL_GPIO_WritePin(laser->GPIOx, laser->pinNum, GPIO_PIN_SET);
}

void Laser_off(hlaser *laser)
{
	HAL_GPIO_WritePin(laser->GPIOx, laser->pinNum, GPIO_PIN_RESET);
}

/*
구조체를 정의할때 포인터를 사용하는이유
hlaser laser -> 구조체 전체를 통째로 복사해서 함수에 넘겨준다(메모리 낭비, 속도저하)
hlaser *laser -> 구조체가 있는 '집 주소'만 알려줌(32bit 주소값 하나만 넘기면 끝남)

hlaser->구조체를 선언(모듈이름)
laser -> 실제 가져다쓰는 인스턴스 이름

-> 화살표는 구조체의 주소(포인터)를 다룰떄 씀
즉, hlaser *laser는 구조체 변수의 주소값을 알려주는데 주소값을 들고있을때
그 주소로 찾아가서 화살표 안에있는 것을 꺼내라
 */
