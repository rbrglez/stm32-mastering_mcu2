/*
 * main_app.c
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */


#include "stm32wbxx_hal.h"
#include "main_app.h"

void SystemClockConfig(void);

int main(void)
{
	HAL_Init();
	SystemClockConfig();

	return 0;
}


void SystemClockConfig(void)
{

}
