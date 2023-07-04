/*
 * it.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#include "main_app.h"

extern UART_HandleTypeDef huart1;

void SysTick_Handler(void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


void USART1_IRQHandler(void){
	HAL_UART_IRQHandler(&huart1);
}
