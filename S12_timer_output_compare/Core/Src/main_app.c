/*
 * main_app.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

/*
 ************************************************************************************************
 ** Library Includes
 ************************************************************************************************
 */
#include "main_app.h"
#include "string.h"
#include "stdio.h"
/*
 ************************************************************************************************
 ** Variables
 ************************************************************************************************
 */
RCC_OscInitTypeDef osc_init = {0};
RCC_ClkInitTypeDef clk_init = {0};

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim17;

uint32_t diff_input_capture = 0;
uint32_t period_meas_ns = 0;
uint32_t freq_meas_Hz = 0;

uint32_t freq_tim2_Hz = 0;
uint32_t period_tim2_ns = 0;

uint8_t IC_capture_Callback_Done = 0;
uint8_t send_msg = 0;

char usr_msg[100];

/*
 ************************************************************************************************
 ** Function Prototypes
 ************************************************************************************************
 */
void SystemClockConfig(uint8_t clk_freq);
void UART1_Init(void);
void TIM1_Init(void);
void TIM2_Init(void);
void TIM17_Init(void);
void GPIO_Init(void);
void LSE_Configuration(void);

// Callbacks
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/*
 ************************************************************************************************
 ** MAIN
 ************************************************************************************************
 */
int main(void) {
	HAL_Init();
	SystemClockConfig(SYS_CLOCK_FREQ_50_MHZ);
	UART1_Init();
	TIM1_Init();
	TIM2_Init();
	TIM17_Init();
	GPIO_Init();
	LSE_Configuration();

	// Start timer 2 in Input Capture mode
	if(HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	// Start timer17 in base mode
	if(HAL_TIM_Base_Start_IT(&htim17) != HAL_OK){
		Error_handler();
	}

	// Start timer1 in Output Compare mode
	if(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	while (1) {
		// when input_capture_callback is done,
		// calculate frequency and period of timer signal and
		// calculate frequency and period of measured signal
		if (IC_capture_Callback_Done == 1){
			freq_tim2_Hz = 25000000;
			period_tim2_ns = (uint32_t)1e9 / freq_tim2_Hz;

			period_meas_ns = diff_input_capture * period_tim2_ns;
			freq_meas_Hz = (uint32_t)1e9 / period_meas_ns;

			IC_capture_Callback_Done = 0;
		}

		if(send_msg == 1){
			send_msg = 0;
			sprintf(usr_msg, "Measured Frequency: %luHz\n\r", freq_meas_Hz);
			HAL_UART_Transmit_IT(&huart1, (uint8_t*)usr_msg, strlen(usr_msg));
		}
	}

	return 0;
}

/*
 ************************************************************************************************
 ** Function Declaration
 ************************************************************************************************
 */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
	send_msg = 1;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	static uint32_t prev_input_capture;
	static uint32_t curr_input_capture;

	prev_input_capture = curr_input_capture;
	curr_input_capture = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
	diff_input_capture = curr_input_capture - prev_input_capture;

	IC_capture_Callback_Done = 1;
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
//	tim2_ch1_init.Pulse = tim2_ch1_init.Pulse + (uint16_t)(25e3); // generate 500Hz signals
//
//	if(HAL_TIM_OC_ConfigChannel(&htim2, &tim2_ch1_init, TIM_CHANNEL_1) != HAL_OK){
//		Error_handler();
//	}

	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
}

void TIM1_Init(void){

	htim1.Instance = TIM1;

	htim1.Init.Prescaler = 50 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 125 - 1;

	if(HAL_TIM_OC_Init(&htim1) != HAL_OK){
		Error_handler();
	}

	TIM_OC_InitTypeDef oc_ch1_init = {0};

	oc_ch1_init.OCMode = TIM_OCMODE_TOGGLE;
	oc_ch1_init.Pulse = (htim1.Init.Period + 1) / 2;
	oc_ch1_init.OCPolarity = TIM_OCPOLARITY_HIGH;

	if(HAL_TIM_OC_ConfigChannel(&htim1, &oc_ch1_init, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}
}

void TIM2_Init(void){

	htim2.Instance = TIM2;

	// TIM2 clock is 25 MHz
	htim2.Init.Prescaler = 2 - 1; // Timer clock is halved
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 0xFFFFFFFF; // maximum value

	if(HAL_TIM_IC_Init(&htim2) != HAL_OK){
		Error_handler();
	}

	TIM_IC_InitTypeDef ic_ch1_init = {0};

	ic_ch1_init.ICPolarity = TIM_ICPOLARITY_RISING;
	ic_ch1_init.ICSelection = TIM_ICSELECTION_DIRECTTI;
	ic_ch1_init.ICPrescaler = TIM_ICPSC_DIV1;
	ic_ch1_init.ICFilter = 0;

	if(HAL_TIM_IC_ConfigChannel(&htim2, &ic_ch1_init, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}
}

void TIM17_Init(void){
	htim17.Instance =  TIM17;

	// If timer input clock is 50MHz, then TIM17 triggers once every second
	htim17.Init.Prescaler = (uint16_t)50e3 - 1;
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = (uint16_t)1e3 - 1;

	if(HAL_TIM_Base_Init(&htim17) != HAL_OK){
		Error_handler();
	}
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

void GPIO_Init(void){
	GPIO_InitTypeDef gpio_init = {0};

	/*
	 *****************************************
	 ** Configure Blue, Green and Red LED
	 *****************************************
	 */
	// GPIOB Clock enable
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Init GPIO
	gpio_init.Pin = GPIO_PIN_5 | GPIO_PIN_0 | GPIO_PIN_1;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init.Pull = GPIO_NOPULL;
	gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOB, &gpio_init);
}

void LSE_Configuration(void){
	RCC_OscInitTypeDef lse_osc_init = {0};

	// Initialize oscillator
	lse_osc_init.OscillatorType = RCC_OSCILLATORTYPE_LSE;
	lse_osc_init.LSEState = RCC_LSE_ON;

	// Configure LSE
	if (HAL_RCC_OscConfig(&lse_osc_init) != HAL_OK){
		Error_handler();
	}

	// Microcontroller Output Configuration
	// Connect LSE pin to PA15 output
	HAL_RCC_MCOConfig(RCC_MCO3_PA15, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
}

void SystemClockConfig(uint8_t clk_freq){

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

void Error_handler(void) {

	while(1){
		// If any error wait here
	}
}
