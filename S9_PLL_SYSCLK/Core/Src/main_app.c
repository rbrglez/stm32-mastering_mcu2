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

#define CLK_CONFIG_MSG_SIZE 380
#define HEADER_BUFFER_SIZE 400
#define APP_BUFFER_MSG_SIZE 300

RCC_OscInitTypeDef osc_init;
RCC_ClkInitTypeDef clk_init;

RCC_PLLInitTypeDef PLL_config = {0};

UART_HandleTypeDef huart1;

char app_buffer_msg[APP_BUFFER_MSG_SIZE];
char clk_config_msg[CLK_CONFIG_MSG_SIZE];
char header_buffer[HEADER_BUFFER_SIZE];
char buffer[32];

void SystemClockConfig(void);
void UART1_Init(void);
void Error_handler(void);
void APP_Gen_Clock_Config_String(char *msg, uint32_t msg_size);
void APP_Gen_Header_String(char *header_buffer, char *header_msg, uint32_t header_buffer_size);

void PllConfig(RCC_PLLInitTypeDef *PLL_init);

int main(void) {
	HAL_Init();
	UART1_Init();
	// clear screen
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "\x1B[2J");
	if(HAL_UART_Transmit(&huart1, (uint8_t*)buffer, 32, HAL_MAX_DELAY) != HAL_OK){
		Error_handler();
	}

	// application start
	APP_Gen_Header_String(app_buffer_msg, "The application is starting!", APP_BUFFER_MSG_SIZE);
	if (HAL_UART_Transmit(&huart1, (uint8_t*) app_buffer_msg, strlen(app_buffer_msg), HAL_MAX_DELAY) != HAL_OK) {
		Error_handler();
	}

	// Via Uart print current configured clock frequencies
	APP_Gen_Header_String(header_buffer, "Clock Configuration Before initialization", HEADER_BUFFER_SIZE);
	for(int i = 0; header_buffer[i] != '\0'; i++){
		if(HAL_UART_Transmit(&huart1, (uint8_t*) (header_buffer + i), 1, HAL_MAX_DELAY) != HAL_OK){
			Error_handler();
		}
	}

	APP_Gen_Clock_Config_String(clk_config_msg, CLK_CONFIG_MSG_SIZE);
	HAL_UART_Transmit(&huart1, (uint8_t*) clk_config_msg, strlen(clk_config_msg), HAL_MAX_DELAY);

	// Configure clocks
	SystemClockConfig();

	// Reconfigure UART1, because clock which runs uart has been reconfigured
	UART1_Init();

	// Via Uart print new configured clock frequencies
	APP_Gen_Header_String(app_buffer_msg, "Clock Configuration After initialization", APP_BUFFER_MSG_SIZE);
	if (HAL_UART_Transmit(&huart1, (uint8_t*) app_buffer_msg, strlen(app_buffer_msg), HAL_MAX_DELAY) != HAL_OK) {
		Error_handler();
	}

	APP_Gen_Clock_Config_String(clk_config_msg, CLK_CONFIG_MSG_SIZE);
	HAL_UART_Transmit(&huart1, (uint8_t*) clk_config_msg, strlen(clk_config_msg), HAL_MAX_DELAY);

	PLL_config.PLLState = RCC_PLL_ON;
	PLL_config.PLLSource = RCC_PLLSOURCE_HSE;
	PLL_config.PLLM = LL_RCC_PLLM_DIV_4;
	PLL_config.PLLN = 42;
	PLL_config.PLLP = RCC_PLLP_DIV4;
	PLL_config.PLLQ = RCC_PLLQ_DIV4;
	PLL_config.PLLR = RCC_PLLR_DIV4;
	PllConfig(&PLL_config);
	UART1_Init();

	APP_Gen_Header_String(app_buffer_msg, "Reconfigured Clock", APP_BUFFER_MSG_SIZE);
	if (HAL_UART_Transmit(&huart1, (uint8_t*) app_buffer_msg, strlen(app_buffer_msg), HAL_MAX_DELAY) != HAL_OK) {
		Error_handler();
	}
	APP_Gen_Clock_Config_String(clk_config_msg, CLK_CONFIG_MSG_SIZE);
	HAL_UART_Transmit(&huart1, (uint8_t*) clk_config_msg, strlen(clk_config_msg), HAL_MAX_DELAY);

	while (1) {

	}

	return 0;
}

void APP_Gen_Header_String(char *header_buffer, char *header_msg, uint32_t header_buffer_size){
	//char symbol_buffer[100];
	// clear array
	memset(header_buffer, 0, header_buffer_size);

	sprintf(
			header_buffer,
			"**********************************************************\r\n" \
			"** %s \n\r" \
			"**********************************************************\r\n",
			header_msg
	);
}

void APP_Gen_Clock_Config_String(char *msg, uint32_t msg_size){
	// clear array
	memset(msg, 0, msg_size);
	// Generate string
	sprintf(
	    	msg,
	        "*********************\r\n" \
	        "** SYSTEM CLOCK\r\n" 		\
	        "*********************\r\n" \
	        "SYSCLK: %ldHz\r\n"			\
	        "*********************\r\n" \
	        "** HIGH-SPEED CLOCKS\r\n" 	\
	        "*********************\r\n" \
	        "HCLK:   %ldHz\r\n" 		\
	        "HCLK2:  %ldHz\r\n" 		\
	        "HCLK4:  %ldHz\r\n" 		\
	        "*********************\r\n" \
	        "** PERIPHERAL CLOCKS\r\n" 	\
	        "*********************\r\n" \
	        "PLCLK1: %ldHz\r\n"			\
	        "PLCLK2: %ldHz\r\n"			\
			"\r\n",
			HAL_RCC_GetSysClockFreq(),
			HAL_RCC_GetHCLKFreq(),
			HAL_RCC_GetHCLK2Freq(),
			HAL_RCC_GetHCLK4Freq(),
			HAL_RCC_GetPCLK1Freq(),
			HAL_RCC_GetPCLK2Freq()
	);
}

void PllConfig(RCC_PLLInitTypeDef *PLL_init){
	RCC_OscInitTypeDef curr_osc_init = {0};
	RCC_ClkInitTypeDef curr_clk_init = {0};

	// deinitialize clock configurations
	HAL_RCC_DeInit();

	// Initialize oscillator
	curr_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	curr_osc_init.HSEState = RCC_HSE_ON;
	// PLL configuration
	curr_osc_init.PLL = *PLL_init;

	if (HAL_RCC_OscConfig(&curr_osc_init) != HAL_OK){
		Error_handler();
	}

	curr_clk_init.ClockType = (
			RCC_CLOCKTYPE_SYSCLK |
			RCC_CLOCKTYPE_HCLK |
			RCC_CLOCKTYPE_PCLK1 |
			RCC_CLOCKTYPE_PCLK2 |
			RCC_CLOCKTYPE_HCLK2 |
			RCC_CLOCKTYPE_HCLK4);
	curr_clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	curr_clk_init.AHBCLKDivider = RCC_SYSCLK_DIV2;
	curr_clk_init.AHBCLK4Divider = RCC_SYSCLK_DIV4; // set divider, so that flash latency used is FLASH_LATENCY_0
	curr_clk_init.APB1CLKDivider = RCC_HCLK_DIV8;
	curr_clk_init.APB2CLKDivider = RCC_HCLK_DIV8;
	if (HAL_RCC_ClockConfig(&curr_clk_init, FLASH_LATENCY_0) != HAL_OK){
		Error_handler();
	}

	// divided by 1000, because systick should trigger every 1ms in order for HAL APIs to work properly.
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void SystemClockConfig(void) {
	// Initialize oscillator
	memset(&osc_init, 0, sizeof(osc_init)); // set whole structure to 0
	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;
	// PLL configuration
	osc_init.PLL.PLLState = RCC_PLL_ON; // activate PLL
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc_init.PLL.PLLM = RCC_PLLM_DIV4;
	osc_init.PLL.PLLN = 25;
	osc_init.PLL.PLLR = RCC_PLLR_DIV4;
	// Don't care for these settings but set them anyway
	osc_init.PLL.PLLP = RCC_PLLP_DIV4;
	osc_init.PLL.PLLQ = RCC_PLLQ_DIV4;

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		Error_handler();
	}

	memset(&clk_init, 0, sizeof(clk_init)); // set whole structure to 0
	clk_init.ClockType = (
			RCC_CLOCKTYPE_SYSCLK |
			RCC_CLOCKTYPE_HCLK |
			RCC_CLOCKTYPE_PCLK1 |
			RCC_CLOCKTYPE_PCLK2 |
			RCC_CLOCKTYPE_HCLK2 |
			RCC_CLOCKTYPE_HCLK4);
	clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_init.AHBCLKDivider = RCC_SYSCLK_DIV2;
	clk_init.AHBCLK4Divider = RCC_SYSCLK_DIV4; // set divider, so that flash latency used is FLASH_LATENCY_0
	clk_init.APB1CLKDivider = RCC_HCLK_DIV8;
	clk_init.APB2CLKDivider = RCC_HCLK_DIV8;
	if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_0) != HAL_OK){
		Error_handler();
	}

	// Because uC is configured to use HSE clock, HSI Clock can be disabled to lower consumption.
	__HAL_RCC_HSI_DISABLE();

	//Systick by default uses HSI clock.
	// Now that HSI clock is disabled, systick should be reconfigured

	// divided by 1000, because systick should trigger every 1ms in order for HAL APIs to work properly.
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

	while(1){
		// If any error wait here
	}
}
