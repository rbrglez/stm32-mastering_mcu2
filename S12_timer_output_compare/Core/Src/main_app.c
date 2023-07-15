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
#define OC_PULSE_4KHZ 125
#define OC_PULSE_2KHZ 250
#define OC_PULSE_1KHZ 500
#define OC_PULSE_500HZ 1000

RCC_OscInitTypeDef osc_init = {0};
RCC_ClkInitTypeDef clk_init = {0};

UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim17;

uint32_t ch1_diff = 0;
uint32_t ch2_diff = 0;
uint32_t ch3_diff = 0;
uint32_t ch4_diff = 0;

uint32_t period_meas_ch1_ns = 0;
uint32_t period_meas_ch2_ns = 0;
uint32_t period_meas_ch3_ns = 0;
uint32_t period_meas_ch4_ns = 0;

uint32_t freq_meas_ch1_Hz = 0;
uint32_t freq_meas_ch2_Hz = 0;
uint32_t freq_meas_ch3_Hz = 0;
uint32_t freq_meas_ch4_Hz = 0;

uint32_t freq_tim2_Hz = 0;
uint32_t period_tim2_ns = 0;

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

	// Start timer 2 in Input Capture mode
	if(HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	// Start timer 2 in Input Capture mode
	if(HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3) != HAL_OK){
		Error_handler();
	}

	// Start timer 2 in Input Capture mode
	if(HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4) != HAL_OK){
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

	// Start timer1 in Output Compare mode
	if(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	// Start timer1 in Output Compare mode
	if(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_3) != HAL_OK){
		Error_handler();
	}

	while (1) {
		if(send_msg == 1){
			send_msg = 0;

			freq_tim2_Hz = (uint32_t)25e6;
			period_tim2_ns = (uint32_t)1e9 / freq_tim2_Hz;

			// CH1
			if(ch1_diff > 0){
				period_meas_ch1_ns = ch1_diff * period_tim2_ns;
				freq_meas_ch1_Hz = (uint32_t)1e9 / period_meas_ch1_ns;
			}
			else{
				period_meas_ch1_ns = 0;
				freq_meas_ch1_Hz = 0;
			}

			// CH2
			if(ch2_diff > 0){
				period_meas_ch2_ns = ch2_diff * period_tim2_ns;
				freq_meas_ch2_Hz = (uint32_t)1e9 / period_meas_ch2_ns;
			}
			else{
				period_meas_ch2_ns = 0;
				freq_meas_ch2_Hz = 0;
			}

			// CH3
			if(ch3_diff > 0){
				period_meas_ch3_ns = ch3_diff * period_tim2_ns;
				freq_meas_ch3_Hz = (uint32_t)1e9 / period_meas_ch3_ns;
			}
			else{
				period_meas_ch3_ns = 0;
				freq_meas_ch3_Hz = 0;
			}

			// CH4
			if(ch4_diff > 0){
				period_meas_ch4_ns = ch4_diff * period_tim2_ns;
				freq_meas_ch4_Hz = (uint32_t)1e9 / period_meas_ch4_ns;
			}
			else{
				period_meas_ch4_ns = 0;
				freq_meas_ch4_Hz = 0;
			}

			sprintf(usr_msg, "F_ch1: %luHz\tF_ch2: %luHz\tF_ch3: %luHz\tF_ch4: %luHz\n\r", freq_meas_ch1_Hz, freq_meas_ch2_Hz, freq_meas_ch3_Hz, freq_meas_ch4_Hz);
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
	static uint32_t ch1[2] = {0};
	static uint32_t ch2[2] = {0};
	static uint32_t ch3[2] = {0};
	static uint32_t ch4[2] = {0};

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		ch1[0] = ch1[1];
		ch1[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
		ch1_diff = ch1[1] - ch1[0];
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
		ch2[0] = ch2[1];
		ch2[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
		ch2_diff = ch2[1] - ch2[0];
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
		ch3[0] = ch3[1];
		ch3[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_3);
		ch3_diff = ch3[1] - ch3[0];
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
		ch4[0] = ch4[1];
		ch4[1] = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_4);
		ch4_diff = ch4[1] - ch4[0];
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	static uint32_t ccr_content;

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, ccr_content + OC_PULSE_4KHZ);
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, ccr_content + OC_PULSE_2KHZ);
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, ccr_content + OC_PULSE_1KHZ);
	}

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
		ccr_content = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, ccr_content + OC_PULSE_500HZ);
	}
}

void TIM1_Init(void){

	htim1.Instance = TIM1;

	htim1.Init.Prescaler = 50 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period =  0xFFFF;

	if(HAL_TIM_OC_Init(&htim1) != HAL_OK){
		Error_handler();
	}

	TIM_OC_InitTypeDef oc_ch_init = {0};

	oc_ch_init.OCMode = TIM_OCMODE_TOGGLE;
	oc_ch_init.OCPolarity = TIM_OCPOLARITY_HIGH;

	oc_ch_init.Pulse = OC_PULSE_4KHZ;
	if(HAL_TIM_OC_ConfigChannel(&htim1, &oc_ch_init, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	oc_ch_init.Pulse = OC_PULSE_2KHZ;
	if(HAL_TIM_OC_ConfigChannel(&htim1, &oc_ch_init, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	oc_ch_init.Pulse = OC_PULSE_1KHZ;
	if(HAL_TIM_OC_ConfigChannel(&htim1, &oc_ch_init, TIM_CHANNEL_3) != HAL_OK){
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

	TIM_IC_InitTypeDef ch_init = {0};

	ch_init.ICPolarity = TIM_ICPOLARITY_RISING;
	ch_init.ICSelection = TIM_ICSELECTION_DIRECTTI;
	ch_init.ICPrescaler = TIM_ICPSC_DIV1;
	ch_init.ICFilter = 0;

	if(HAL_TIM_IC_ConfigChannel(&htim2, &ch_init, TIM_CHANNEL_1) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_IC_ConfigChannel(&htim2, &ch_init, TIM_CHANNEL_2) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_IC_ConfigChannel(&htim2, &ch_init, TIM_CHANNEL_3) != HAL_OK){
		Error_handler();
	}

	if(HAL_TIM_IC_ConfigChannel(&htim2, &ch_init, TIM_CHANNEL_4) != HAL_OK){
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
