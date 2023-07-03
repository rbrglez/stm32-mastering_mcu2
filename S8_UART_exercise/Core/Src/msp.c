/*
 * msp.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#include "stm32wbxx_hal.h"
#include "main_app.h"


void HAL_MspInit(void)
{
	// Function called by HAL_Init()
	// Initialize low-level processor specific inits

	// 1.) Set up the priority grouping of the arm cortex mx processor
	// 	NOTE: Redundant, because this is default setting(This is done in HAL_Init())
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	// 2.) Enable the required system exceptions of the arm cortex mx processor
	// 	NOTE: Enabled usage fault, memory fault and bus fault exceptions.
	SCB -> SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
	SCB -> SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
	SCB -> SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;


	// 3.) Configure the priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, MEM_MNG_PRIORITY, DEFAULT_SUB_PRIORITY);
	HAL_NVIC_SetPriority(BusFault_IRQn, BUS_FLT_PRIORITY, DEFAULT_SUB_PRIORITY);
	HAL_NVIC_SetPriority(UsageFault_IRQn, BUS_FLT_PRIORITY, DEFAULT_SUB_PRIORITY);
}
