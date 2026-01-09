/*
 * Listener_Tracking.c
 *
 *  Created on: Jan 7, 2026
 *      Author: kccistc
 */

#include "Listener_Tracking.h"

extern osPoolId poolTrackingData;

void Listener_Tracking_Init()
{

}

void Listener_Tracking_Excute()
{
	static int current_angle_x = 0;
	static int current_angle_y = 0;

	tracking_t *pData = (tracking_t *) osPoolAlloc(poolTrackingData);
	//메모리할당 osPoolAllco는 주소의 데이터만큼 메모리를 정확히 할다함

	if(pData != NULL)
	{
		pData-> x_angle = current_angle_x;
		current_angle_x += 20;
		if(current_angle_x > 180)
		{
			current_angle_x = 0;
		}

		pData-> y_angle = current_angle_y;
		current_angle_y += 20;
		if(current_angle_y > 180)
		{
			current_angle_y = 0;
		}
		osMessagePut(trackingDataMsgBox, (uint32_t)pData, osWaitForever);
	}
	osDelay(500);
}


