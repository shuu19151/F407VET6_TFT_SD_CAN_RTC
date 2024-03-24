/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define PE2_KEY_0_Pin GPIO_PIN_2
#define PE2_KEY_0_GPIO_Port GPIOE
#define PE2_KEY_0_EXTI_IRQn EXTI2_IRQn
#define PE3_KEY_1_Pin GPIO_PIN_3
#define PE3_KEY_1_GPIO_Port GPIOE
#define PE3_KEY_1_EXTI_IRQn EXTI3_IRQn
#define PE4_KEY_2_Pin GPIO_PIN_4
#define PE4_KEY_2_GPIO_Port GPIOE
#define PE4_KEY_2_EXTI_IRQn EXTI4_IRQn
#define PE5_KEY_3_Pin GPIO_PIN_5
#define PE5_KEY_3_GPIO_Port GPIOE
#define PE5_KEY_3_EXTI_IRQn EXTI9_5_IRQn
#define PE6_KEY_4_Pin GPIO_PIN_6
#define PE6_KEY_4_GPIO_Port GPIOE
#define PE6_KEY_4_EXTI_IRQn EXTI9_5_IRQn
#define TFT_BL_Pin GPIO_PIN_1
#define TFT_BL_GPIO_Port GPIOB
#define SPI2_RST_Pin GPIO_PIN_10
#define SPI2_RST_GPIO_Port GPIOB
#define SPI2_DC_Pin GPIO_PIN_11
#define SPI2_DC_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define SPI2_SCLK_Pin GPIO_PIN_13
#define SPI2_SCLK_GPIO_Port GPIOB
#define SPI2_MOSI_Pin GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define PB9_KEY_5_Pin GPIO_PIN_9
#define PB9_KEY_5_GPIO_Port GPIOB
#define PB9_KEY_5_EXTI_IRQn EXTI9_5_IRQn
#define PE0_LED_RED_Pin GPIO_PIN_0
#define PE0_LED_RED_GPIO_Port GPIOE
#define PE1_LED_GREEN_Pin GPIO_PIN_1
#define PE1_LED_GREEN_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
