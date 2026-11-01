/*
 * WS2811_12.c
 *
 *  Created on: Nov 6, 2025
 *      Author: Mohsen
 */

#include "WS2811_12.h"



uint16_t pwmData[(24*MAX_LED)+50];
int datasentflag=0;
uint8_t LED_Data[MAX_LED][4];



void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(WS_TIMER_H, WS_TIMER_CHANNEL);
	datasentflag=1;
}


void Set_LED (int LEDnum, int Red, int Green, int Blue)
{
	LED_Data[LEDnum][0] = LEDnum;
	LED_Data[LEDnum][1] = Green;
	LED_Data[LEDnum][2] = Red;
	LED_Data[LEDnum][3] = Blue;
}



void WS2812_Send (void)
{
	uint32_t indx=0;
	uint32_t color;

	for (int i = 0; i < 10; i++)
	{
		color = ((uint32_t)LED_Data[i][1] << 16) | ((uint32_t)LED_Data[i][2] << 8) | LED_Data[i][3];

		for (int bit = 23; bit >= 0; bit--)
		{
			if (color & (1 << bit))
			{
				pwmData[indx] = WS_LOGIC_ONE_CCR;  // logic 1
			}
			else
			{
				pwmData[indx] = WS_LOGIC_ZERO_CCR;  // logic 0
			}
			indx++;
		}
	}

	for (int i = 0; i < 50; i++)
	{
		pwmData[indx] = 0;
		indx++;
	}

	HAL_TIM_PWM_Start_DMA(WS_TIMER_H, WS_TIMER_CHANNEL, (uint32_t *)pwmData, indx);
	while (!datasentflag);
	datasentflag = 0;
}

