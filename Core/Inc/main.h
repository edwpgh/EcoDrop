/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Mosfet_8_Pin GPIO_PIN_13
#define Mosfet_8_GPIO_Port GPIOC
#define Mosfet_9_Pin GPIO_PIN_14
#define Mosfet_9_GPIO_Port GPIOC
#define Mosfet_10_Pin GPIO_PIN_15
#define Mosfet_10_GPIO_Port GPIOC
#define MCU_BTN_Pin GPIO_PIN_11
#define MCU_BTN_GPIO_Port GPIOB
#define MCU_LED_Pin GPIO_PIN_12
#define MCU_LED_GPIO_Port GPIOB
#define ASK_IN_SIG_Pin GPIO_PIN_8
#define ASK_IN_SIG_GPIO_Port GPIOA
#define ASK_IN_SIG_EXTI_IRQn EXTI4_15_IRQn
#define Mosfet_1_Pin GPIO_PIN_15
#define Mosfet_1_GPIO_Port GPIOA
#define Mosfet_2_Pin GPIO_PIN_3
#define Mosfet_2_GPIO_Port GPIOB
#define Mosfet_3_Pin GPIO_PIN_4
#define Mosfet_3_GPIO_Port GPIOB
#define Mosfet_4_Pin GPIO_PIN_5
#define Mosfet_4_GPIO_Port GPIOB
#define Mosfet_5_Pin GPIO_PIN_6
#define Mosfet_5_GPIO_Port GPIOB
#define Mosfet_6_Pin GPIO_PIN_8
#define Mosfet_6_GPIO_Port GPIOB
#define Mosfet_7_Pin GPIO_PIN_9
#define Mosfet_7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
