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
char *user_data = "The application is running\r\n";

uint8_t data_buffer[100];
uint32_t count = 0;
uint8_t rx_data;

void SystemClockConfig(void);
void UART1_Init(void);
void Error_handler(void);
void capitalize_string(uint8_t *str, uint32_t length);
uint8_t convert_to_capital(uint8_t character);

int main(void) {
	HAL_Init();
	SystemClockConfig();
	UART1_Init();

	if (HAL_UART_Transmit(&huart1, (uint8_t*) user_data, strlen(user_data), HAL_MAX_DELAY) != HAL_OK) {
		Error_handler();
	}

	while(1){
		HAL_UART_Receive_IT(&huart1, &rx_data, 1);
	}

	return 0;
}

void SystemClockConfig(void) {

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

void capitalize_string(uint8_t *str, uint32_t length) {
	for (int i = 0; i < length; i++) {
		if ((*(str + i) >= 'a') && (*(str + i) <= 'z')) {
			*(str + i) += 'A' - 'a';
		}
	}
}

uint8_t convert_to_capital(uint8_t character) {
	if (character >= 'a' && character <= 'z')
		return (character + 'A' - 'a');
	else
		return character;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (rx_data == '\r'){
		data_buffer[count++] = '\r';
		HAL_UART_Transmit(huart, data_buffer, count, HAL_MAX_DELAY);
		count = 0;
	}
	else{
		data_buffer[count++] = convert_to_capital(rx_data);
	}

}
