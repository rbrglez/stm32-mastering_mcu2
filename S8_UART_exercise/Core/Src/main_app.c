/*
 * main_app.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */


#include "stm32wbxx_hal.h"
#include "main_app.h"
#include "string.h"

UART_HandleTypeDef huart1;
char *user_data = "The application is running\n";

void SystemClockConfig(void);
void UART1_Init(void);
void Error_handler(void);

int main(void)
{
	HAL_Init();
	SystemClockConfig();
	UART1_Init();

	if(HAL_UART_Transmit(&huart1, (uint8_t *)user_data, strlen(user_data), HAL_MAX_DELAY) != HAL_OK)
	{
		Error_handler();
	}

	while(1){

	}

	return 0;
}


void SystemClockConfig(void)
{

}

void UART1_Init(void)
{
	huart1.Instance = USART1;

	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		// There is a problem
		Error_handler();
	}
}

void Error_handler(void)
{

}
