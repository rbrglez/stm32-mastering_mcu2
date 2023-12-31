/*
 * msp.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

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

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef gpio_uart;

	// USART1 peripheral low-level initializations

	// 1.) enable the clock for USART1 peripheral
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// 2.) Pin muxing configurations
	// Uart Tx and Uart Rx

	/**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
	gpio_uart.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOB, &gpio_uart);

	// 3.) Enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_NVIC_SetPriority(USART1_IRQn, UART1_PRIORITY, DEFAULT_SUB_PRIORITY);

}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim){
	GPIO_InitTypeDef gpio_init;

	// 1.) Enable peripheral clocks
	__HAL_RCC_TIM2_CLK_ENABLE(); // Clock for TIM2
	__HAL_RCC_GPIOA_CLK_ENABLE(); // Clock for TIM2 CH1

	// 2.) Configure a GPIO to behave as timer 2 channel 1
	gpio_init.Pin = GPIO_PIN_0;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	gpio_init.Alternate = GPIO_AF1_TIM2;

	HAL_GPIO_Init(GPIOA, &gpio_init);


	// 3.) Enable IRQ of TIM2
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	// 4.) Setup the priority of IRQ
	HAL_NVIC_SetPriority(TIM2_IRQn, TIM2_PRIORITY, DEFAULT_SUB_PRIORITY);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim){

	// 1.) Enable the clock for the TIM16 peripheral
	__HAL_RCC_TIM16_CLK_ENABLE();

	// 2.) Enable IRQ of TIM16
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

	// 3.) Setup the priority of IRQ
	HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, TIM16_PRIORITY, DEFAULT_SUB_PRIORITY);
}
