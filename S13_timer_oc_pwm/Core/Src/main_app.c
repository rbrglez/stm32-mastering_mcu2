/*
 * main_app.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#include "main_app.h"
#include "stdio.h"
#include "string.h"

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;

void SystemClockConfig(uint8_t clk_freq);
void UART1_Init(void);
void TIM2_Init(void);
void GPIO_Init(void);

// Application
void APP_UART_TxCurrentConfig(void);
void APP_ModifyConfigMsg(char *msg[]);

// Callbacks
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);

char usr_msg[100];

char* config_msg[] = {
		"***********************\r\n",
		"LED_0 (A3)\r\n",
		"LED_1 (A2)\r\n",
		"LED_2 (D1)\r\n",
		"LED_3 (D0)\r\n"
};

uint8_t send_config = 0;

uint8_t selected_PWM_Ch = 2;
uint16_t pwm_ch_setting[4] = {75, 50, 25, 10};

int main(void) {
	HAL_Init();
	SystemClockConfig(SYS_CLOCK_FREQ_50_MHZ);
	UART1_Init();
	TIM2_Init();
	GPIO_Init();

	if(HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_3) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_4) != HAL_OK){
		Error_handler();
	}

	sprintf(usr_msg, "The Application has started!\r\n");
	if(HAL_UART_Transmit_IT(&huart1, (uint8_t*)usr_msg, strlen(usr_msg)) != HAL_OK){
		Error_handler();
	}

	// Add Your code here
	while (1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_Delay(1000);
	}

	return 0;
}

void APP_ModifyConfigMsg(char *msg[]){
	char insert_msg[] = "> ";

	uint8_t modified_line_length = strlen(msg[1]) + strlen(insert_msg) + 1;
	char modified_line[modified_line_length];

    // Construct the modified second line with ">"
    snprintf(modified_line, modified_line_length, "> %s", msg[1]);

    msg[1] = modified_line;


//	sprintf(usr_msg, "lineLength = %d\r\n", lineLength);
//	HAL_UART_Transmit_IT(&huart1, (uint8_t*)modified_line, modified_line_length);
	APP_UART_TxCurrentConfig();
}
void APP_UART_TxCurrentConfig(void){
	static uint8_t curr_line = 0;
	char* line = config_msg[curr_line];

	curr_line++;

	if(curr_line >= sizeof(config_msg)/sizeof(config_msg[0])){
		curr_line = 0;
		send_config = 0;
	}

	if(HAL_UART_Transmit_IT(&huart1, (uint8_t*)line, strlen(line)) != HAL_OK){
		Error_handler();
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(send_config == 1){
		APP_UART_TxCurrentConfig();
	}
}

void GPIO_Init(void){

	GPIO_InitTypeDef led_init = {0};
	GPIO_InitTypeDef btn_init = {0};

	/* Initialize LEDs */

	// GPIOB Clock enable
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/** LEDs GPIO Configuration
    PB5 ------> Blue LED
    PB0 ------> Green LED
    PB1 ------> Red LED
    */
	led_init.Pin = GPIO_PIN_5 | GPIO_PIN_0 | GPIO_PIN_1;
	led_init.Mode = GPIO_MODE_OUTPUT_PP;
	led_init.Pull = GPIO_NOPULL;
	led_init.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOB, &led_init);

	/* Initialize Buttons */

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/** Buttons GPIO Configuration
    PC4 ------> SW1
    PD0 ------> SW2
    PD1 ------> SW3
    */
	btn_init.Pin = GPIO_PIN_4;
	btn_init.Mode = GPIO_MODE_IT_FALLING;
	btn_init.Pull = GPIO_PULLUP;
	btn_init.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &btn_init);

	btn_init.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	HAL_GPIO_Init(GPIOD, &btn_init);

	// Enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_SetPriority(EXTI4_IRQn, BTN_PRIORITY, DEFAULT_SUB_PRIORITY);

	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, BTN_PRIORITY, DEFAULT_SUB_PRIORITY);

	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_SetPriority(EXTI1_IRQn, BTN_PRIORITY, DEFAULT_SUB_PRIORITY);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	switch (GPIO_Pin){
		case GPIO_PIN_4:
			// SW1
			// TODO: Send Current configuration via UART

			send_config = 1; // Enable sending of Configurations
			APP_UART_TxCurrentConfig();

			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // toggle green LED
			break;

		case GPIO_PIN_0:
			// SW2

			APP_ModifyConfigMsg(config_msg);

			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1); // toggle red LED
			break;

		case GPIO_PIN_1:
			// SW3
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5); // toggle blue LED
			break;

		default:
			Error_handler();
			break;
	}
}

void TIM2_Init(void){
	htim2.Instance = TIM2;

	htim2.Init.Prescaler = 50 - 1; // Timer clock is 1 MHz
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1000 - 1;

	if(HAL_TIM_OC_Init(&htim2) != HAL_OK){
		Error_handler();
	}

	TIM_OC_InitTypeDef oc_ch_config = {0};

	oc_ch_config.OCMode = TIM_OCMODE_PWM1;
	oc_ch_config.OCPolarity = TIM_OCPOLARITY_HIGH;
	oc_ch_config.OCFastMode = TIM_OCFAST_ENABLE;

	oc_ch_config.Pulse = 250;
	if(HAL_TIM_OC_ConfigChannel(&htim2, &oc_ch_config, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	oc_ch_config.Pulse = 450;
	if(HAL_TIM_OC_ConfigChannel(&htim2, &oc_ch_config, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	oc_ch_config.Pulse = 750;
	if(HAL_TIM_OC_ConfigChannel(&htim2, &oc_ch_config, TIM_CHANNEL_3) != HAL_OK){
		Error_handler();
	}

	oc_ch_config.Pulse = 900;
	if(HAL_TIM_OC_ConfigChannel(&htim2, &oc_ch_config, TIM_CHANNEL_4) != HAL_OK){
		Error_handler();
	}

}

void SystemClockConfig(uint8_t clk_freq){

	RCC_OscInitTypeDef osc_init = {0};
	RCC_ClkInitTypeDef clk_init = {0};
	uint32_t FLatency;

	// Initialize oscillator
	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_init.HSEState = RCC_HSE_ON;
	// PLL configuration
	osc_init.PLL.PLLState = RCC_PLL_ON; // activate PLL
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clk_freq){
		case SYS_CLOCK_FREQ_50_MHZ:
		{
			osc_init.PLL.PLLM = RCC_PLLM_DIV4;
			osc_init.PLL.PLLN = 25;
			osc_init.PLL.PLLR = RCC_PLLR_DIV4;
			// Don't care for these settings but set them anyway
			osc_init.PLL.PLLP = RCC_PLLP_DIV4;
			osc_init.PLL.PLLQ = RCC_PLLQ_DIV4;

			clk_init.ClockType = (
					RCC_CLOCKTYPE_SYSCLK |
					RCC_CLOCKTYPE_HCLK |
					RCC_CLOCKTYPE_PCLK1 |
					RCC_CLOCKTYPE_PCLK2 |
					RCC_CLOCKTYPE_HCLK2 |
					RCC_CLOCKTYPE_HCLK4);
			clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			clk_init.AHBCLK2Divider = RCC_SYSCLK_DIV2;
			clk_init.AHBCLK4Divider = RCC_SYSCLK_DIV4; // set divider, so that flash latency used is FLASH_LATENCY_0
			clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
			clk_init.APB2CLKDivider = RCC_HCLK_DIV1;

			FLatency = FLASH_ACR_LATENCY_0WS;

			break;
		}

		case SYS_CLOCK_FREQ_64_MHZ:
		{
			osc_init.PLL.PLLM = RCC_PLLM_DIV4;
			osc_init.PLL.PLLN = 32;
			osc_init.PLL.PLLR = RCC_PLLR_DIV4;
			// Don't care for these settings but set them anyway
			osc_init.PLL.PLLP = RCC_PLLP_DIV4;
			osc_init.PLL.PLLQ = RCC_PLLQ_DIV4;

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

			FLatency = FLASH_ACR_LATENCY_0WS;

			break;
		}

		case SYS_CLOCK_FREQ_25_MHZ:
		{
			osc_init.PLL.PLLM = RCC_PLLM_DIV4;
			osc_init.PLL.PLLN = 25;
			osc_init.PLL.PLLR = RCC_PLLR_DIV8;
			// Don't care for these settings but set them anyway
			osc_init.PLL.PLLP = RCC_PLLP_DIV4;
			osc_init.PLL.PLLQ = RCC_PLLQ_DIV4;

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

			FLatency = FLASH_ACR_LATENCY_0WS;

			break;
		}

		default:
			return;
	}

	if (HAL_RCC_OscConfig(&osc_init) != HAL_OK){
		Error_handler();
	}

	if (HAL_RCC_ClockConfig(&clk_init, FLatency) != HAL_OK){
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
