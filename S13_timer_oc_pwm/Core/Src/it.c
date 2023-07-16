/*
 * it.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */
#include "it.h"

extern TIM_HandleTypeDef htim1;

void SysTick_Handler(void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM1_UP_TIM16_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim1);
}
