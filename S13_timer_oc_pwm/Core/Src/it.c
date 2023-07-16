/*
 * it.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */
#include "it.h"

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;

void SysTick_Handler(void){
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void USART1_IRQHandler(void){
	HAL_UART_IRQHandler(&huart1);
}

void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim2);
}

void EXTI4_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI0_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
