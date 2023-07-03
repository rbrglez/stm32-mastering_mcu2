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

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef gpio_uart;

	// USART1 peripheral low-level initializations

	// 1.) enable the clock for UASRT1 peripheral
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
