/*
 * it.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */
#include "it.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart1;

void SysTick_Handler(void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim2);
}

void TIM1_TRG_COM_TIM17_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim17);
}

void USART1_IRQHandler(void){
	HAL_UART_IRQHandler(&huart1);
}

void TIM1_CC_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim1);
}
