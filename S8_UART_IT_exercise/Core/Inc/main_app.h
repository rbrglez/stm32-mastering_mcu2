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

// Redundant because NVIC_PRIORITYGROUP_4 is used, which has only one sub-priority!
#define  DEFAULT_SUB_PRIORITY	0U /*!< Default interrupt sub-priority */

#endif /* INC_MAIN_APP_H_ */
