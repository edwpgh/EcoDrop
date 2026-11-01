/*
 * WS2811_12.h
 *
 *  Created on: Nov 6, 2025
 *      Author: Mohsen
 */


#ifndef INC_WS2811_12_H_
#define INC_WS2811_12_H_




//	Initialization (For CubeMX)
//	MCU Clock should be on 72MHz clock
//	timer should be on mode of PWM Generation
//	PWM Frequency should be 800Khz (on 72MHz, Period should be 89)
//	Add DMA Channel on Timer PWM Channel (Half, Memory Inc)


#include "tim.h"


#define WS_TIMER_H			&htim1
#define WS_TIMER_CHANNEL	TIM_CHANNEL_4
#define MAX_LED				20

// Sum of these values should be Period Of PWM
#define WS_LOGIC_ONE_CCR		40
#define WS_LOGIC_ZERO_CCR		20


void Set_LED (int LEDnum, int Red, int Green, int Blue);
void WS2812_Send (void);


extern uint16_t pwmData[(24*MAX_LED)+50];
extern int datasentflag;
extern uint8_t LED_Data[MAX_LED][4];





#endif /* INC_WS2811_12_H_ */








