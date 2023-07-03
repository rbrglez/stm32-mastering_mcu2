/*
 * it.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#include "stm32wbxx_hal.h"

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}
