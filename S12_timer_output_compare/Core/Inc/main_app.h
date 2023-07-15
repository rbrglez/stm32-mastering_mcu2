/*
 * main_app.h
 *
 *  Created on: 3 Jul 2023
 *      Author: Rene
 */

#ifndef INC_MAIN_APP_H_
#define INC_MAIN_APP_H_

#include "stm32wbxx_hal.h"

// Use NVIC_PRIORITYGROUP_4, which has 16 different preempt-priorities
#define  MEM_MNG_PRIORITY	0U /*!< memory management interrupt preempt-priority */
#define  BUS_FLT_PRIORITY	0U /*!< bus fault interrupt preempt-priority */
#define  USG_FLT_PRIORITY	0U /*!< usage fault interrupt preempt-priority */
#define  UART1_PRIORITY		1U /*!< Uart1 interrupt preempt-priority */
#define  TIM2_PRIORITY		13U /*!< Timer2 interrupt preempt-priority */
#define  TIM1_PRIORITY		14U /*!< Timer1 interrupt preempt-priority */
#define  TIM17_PRIORITY		15U /*!< Timer17 interrupt preempt-priority */

// Redundant because NVIC_PRIORITYGROUP_4 is used, which has only one sub-priority!
#define  DEFAULT_SUB_PRIORITY	0U /*!< Default interrupt sub-priority */

// Sys Clock freqs
#define SYS_CLOCK_FREQ_50_MHZ 50
#define SYS_CLOCK_FREQ_64_MHZ 64
#define SYS_CLOCK_FREQ_25_MHZ 25

void Error_handler(void);

#endif /* INC_MAIN_APP_H_ */
