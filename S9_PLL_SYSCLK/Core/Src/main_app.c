/*
 * main_app.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#include "stm32wbxx_hal.h"
#include "main_app.h"
#include "string.h"
#include  "stdio.h"

RCC_OscInitTypeDef osc_init;
RCC_ClkInitTypeDef clk_init;

UART_HandleTypeDef huart1;

char *user_data = "The application is starting! \r\n";
char msg[100];

void SystemClockConfig(void);
void UART1_Init(void);
void Error_handler(void);
void UART_Tx_Clk_Freqs(UART_HandleTypeDef *huart);

int main(void) {
	HAL_Init();
	UART1_Init();
	// Via Uart print current configured clock frequencies
	UART_Tx_Clk_Freqs(&huart1);
	// Configure clocks
	SystemClockConfig();
	// Reconfigure UART1, because clock which runs uart has been reconfigured
	UART1_Init();
	// Via Uart print new configured clock frequencies
	UART_Tx_Clk_Freqs(&huart1);

	if (HAL_UART_Transmit(&huart1, (uint8_t*) user_data, strlen(user_data), HAL_MAX_DELAY) != HAL_OK) {
		Error_handler();
	}

	while (1) {

	}

	return 0;
}

void UART_Tx_Clk_Freqs(UART_HandleTypeDef *huart){
	memset(msg, 0, sizeof(msg));
	sprintf(msg, "SYSCLK : %ld\r\n", HAL_RCC_GetSysClockFreq());
	HAL_UART_Transmit(huart, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	memset(msg, 0, sizeof(msg));
	sprintf(msg, "HCLK   : %ld\r\n", HAL_RCC_GetHCLKFreq());
	HAL_UART_Transmit(huart, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	memset(msg, 0, sizeof(msg));
	sprintf(msg, "PCLK1  : %ld\r\n", HAL_RCC_GetPCLK1Freq());
	HAL_UART_Transmit(huart, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

	memset(msg, 0, sizeof(msg));
	sprintf(msg, "PCLK2  : %ld\r\n", HAL_RCC_GetPCLK2Freq());
	HAL_UART_Transmit(huart, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
}

void SystemClockConfig(void) {
	// Initialize osciliator
	memset(&osc_init, 0, sizeof(osc_init)); // set whole structure to 0
	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;
	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		Error_handler();
	}

	clk_init.ClockType = (
			RCC_CLOCKTYPE_SYSCLK |
			RCC_CLOCKTYPE_HCLK |
			RCC_CLOCKTYPE_PCLK1 |
			RCC_CLOCKTYPE_PCLK2 |
			RCC_CLOCKTYPE_HCLK2 |
			RCC_CLOCKTYPE_HCLK4);
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV2;
	clk_init.APB1CLKDivider = RCC_HCLK_DIV8;
	clk_init.APB2CLKDivider = RCC_HCLK_DIV8;
	if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_1) != HAL_OK){
		Error_handler();
	}

	// Because uC is configured to use HSE clock, HSI Clock can be disabled to lower consumption.
	__HAL_RCC_HSI_DISABLE();

	//Systick by default uses HSI clock.
	// Now that HSI clock is disabled, systick should be reconfigured

	// diveded by 1000, because systick should trigger every 1ms in order for HAL APIs to work properly.
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);


}

void UART1_Init(void) {
	huart1.Instance = USART1;

	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;

	if (HAL_UART_Init(&huart1) != HAL_OK) {
		// There is a problem
		Error_handler();
	}
}

void Error_handler(void) {

}
