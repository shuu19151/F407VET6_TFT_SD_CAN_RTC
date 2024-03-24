/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <iostream>
#include "../Lib/DS3231/DS3231.h"
#include "../Lib/ILI9341/ILI9341.h"
#include "../Lib/ILI9341/fonts.h"
#include "../Lib/ILI9341/Image.h"
#include "../Lib/JPGEdecoder/jpgDecoder.h"
#include "../Lib/SerialDebug/UartRingBuffer.h"
#include "../Lib/Button/Button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
DS3231* rtc;
ILI9341* lcd;
JpgDecoder* jpgDec;
Time_t* time;
Button *btn_ok, *btn_up, *btn_down, *btn_left, *btn_right, *btn_back;

uint32_t TxMailbox;

uint8_t TxData[8] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

bool renderLCD(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if(y>=lcd->height()) return 0; // 0 - to stop decoding
  lcd->drawImage(x, y, w, h, bitmap);
  return 1; // 1 - to decode next block
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void button_cb(uint8_t id, Button_event_state_t e) {
	if(id == btn_ok->buttonId) {
		switch(e) {
		case BUTTON_PRESSED:
			Uart_sendstring("Button OK pressed!\n");
			break;
		case BUTTON_RELEASED:
			Uart_sendstring("Button OK released!\n");
      Uart_sendstring("Showing image 08.jpg\n");
      jpgDec->show(0, 0, "08.jpg");
			break;
		case BUTTON_CLICKED:
			Uart_sendstring("Button OK clicked!\n");
			break;
		case BUTTON_PRESSED_LONG:
			Uart_sendstring("Button OK pressed long!\n");
			break;
		default: break;
		}
	}
	if(id == btn_up->buttonId) {
		switch(e) {
		case BUTTON_PRESSED:
			Uart_sendstring("Button UP pressed!\n");
			break;
		case BUTTON_RELEASED:
			Uart_sendstring("Button UP released!\n");
      Uart_sendstring("Showing image 07.jpg\n");
      jpgDec->show(0, 0, "07.jpg");
			break;
		case BUTTON_CLICKED:
			Uart_sendstring("Button UP clicked!\n");
			break;
		case BUTTON_PRESSED_LONG:
			Uart_sendstring("Button UP pressed long!\n");
			break;
		default: break;
		}
	}
	if(id == btn_down->buttonId) {
		switch(e) {
		case BUTTON_PRESSED:
			Uart_sendstring("Button DOWN pressed!\n");
			break;
		case BUTTON_RELEASED:
			Uart_sendstring("Button DOWN released!\n");
      Uart_sendstring("Showing image 06.jpg\n");
      jpgDec->show(0, 0, "06.jpg");
			break;
		case BUTTON_CLICKED:
			Uart_sendstring("Button DOWN clicked!\n");
			break;
		case BUTTON_PRESSED_LONG:
			Uart_sendstring("Button DOWN pressed long!\n");
			break;
		default: break;
		}
	}
  if(id == btn_left->buttonId) {
    switch(e) {
    case BUTTON_PRESSED:
      Uart_sendstring("Button LEFT pressed!\n");
      break;
    case BUTTON_RELEASED:
      Uart_sendstring("Button LEFT released!\n");
      Uart_sendstring("Showing image 05.jpg\n");
      jpgDec->show(0, 0, "05.jpg");
      break;
    case BUTTON_CLICKED:
      Uart_sendstring("Button LEFT clicked!\n");
      break;
    case BUTTON_PRESSED_LONG:
      Uart_sendstring("Button LEFT pressed long!\n");
      break;
    default: break;
    }
  }
  if(id == btn_right->buttonId) {
    switch(e) {
    case BUTTON_PRESSED:
      Uart_sendstring("Button RIGHT pressed!\n");
      break;
    case BUTTON_RELEASED:
      Uart_sendstring("Button RIGHT released!\n");
      Uart_sendstring("Showing image 04.jpg\n");
      jpgDec->show(0, 0, "04.jpg");
      break;
    case BUTTON_CLICKED:
      Uart_sendstring("Button RIGHT clicked!\n");
      break;
    case BUTTON_PRESSED_LONG:
      Uart_sendstring("Button RIGHT pressed long!\n");
      break;
    default: break;
    }
  }
  if(id == btn_back->buttonId) {
    switch(e) {
    case BUTTON_PRESSED:
      Uart_sendstring("Button BACK pressed!\n");
      break;
    case BUTTON_RELEASED:
      Uart_sendstring("Button BACK released!\n");
      Uart_sendstring("Showing image 03.jpg\n");
      jpgDec->show(0, 0, "03.jpg");
      break;
    case BUTTON_CLICKED:
      Uart_sendstring("Button BACK clicked!\n");
      break;
    case BUTTON_PRESSED_LONG:
      Uart_sendstring("Button BACK pressed long!\n");
      break;
    default: break;
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
extern "C" int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_CAN1_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  Ringbuffer_init();
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  TxHeader.DLC = 8;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x407;
  TxHeader.TransmitGlobalTime = DISABLE;

  time = new Time_t {0};
  rtc = new DS3231(hi2c1);
  lcd = new ILI9341(hspi2,
		  SPI2_CS_GPIO_Port, SPI2_CS_Pin,
		  SPI2_DC_GPIO_Port, SPI2_DC_Pin,
		  SPI2_RST_GPIO_Port, SPI2_RST_Pin);
  jpgDec = new JpgDecoder(renderLCD);

  btn_ok = new Button(GPIOE, PE4_KEY_2_Pin, BUTTON_ACTIVE_LOW, button_cb);
  btn_up = new Button(GPIOE, PE6_KEY_4_Pin, BUTTON_ACTIVE_LOW, button_cb);
  btn_down = new Button(GPIOE, PE3_KEY_1_Pin, BUTTON_ACTIVE_LOW, button_cb);
  btn_left = new Button(GPIOE, PE2_KEY_0_Pin, BUTTON_ACTIVE_LOW, button_cb);
  btn_right = new Button(GPIOE, PE5_KEY_3_Pin, BUTTON_ACTIVE_LOW, button_cb);
  btn_back = new Button(GPIOB, PB9_KEY_5_Pin, BUTTON_ACTIVE_LOW, button_cb);

  lcd->init();
  lcd->fillScreen(ILI9341_BLACK);

  jpgDec->setJpgScale(2);
  jpgDec->setSwapBytes(true);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  jpgDec->show(0, 0, "08.jpg");
  jpgDec->show(lcd->height()/2 + 40, 0, "07.jpg");
  jpgDec->show(0, lcd->width()/2 - 40, "03.jpg");
  jpgDec->show(lcd->height()/2 + 40, lcd->width()/2 - 40, "05.jpg");
  jpgDec->setJpgScale(1);
  uint8_t i = 10;
  uint8_t j = 1;
  uint32_t interval = 0;
  while (1) {
  /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
	  btn_left->handleButton();
	  btn_ok->handleButton();
	  btn_up->handleButton();
	  btn_down->handleButton();
    btn_right->handleButton();
    btn_back->handleButton();

    if(HAL_GetTick() - interval >= 1000) {
    	interval = HAL_GetTick();
        rtc->getDateTime(time);
    	std::string time_str;
    	time_str += "Date Time: ";
    	time_str += std::to_string(time->Hour);
    	time_str += " : ";
    	time_str += std::to_string(time->Min);
    	time_str += " : ";
    	time_str += std::to_string(time->Sec);
    	TxData[0] = ++i;
    	TxData[1] = ++j;
    	HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);

    	std::string CAN_label = "CAN data received:";
    	std::string CAN_data1 = "data0: ";
    	CAN_data1 += std::to_string(RxData[0]);
    	std::string CAN_data2 = "data1: ";
    	CAN_data2 += std::to_string(RxData[1]);
    	lcd->writeString(0, 0, time_str.c_str(), Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
    	lcd->writeString(0, 80, CAN_label.c_str(), Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
    	lcd->writeString(0, 120, CAN_data1.c_str(), Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
    	lcd->writeString(0, 160, CAN_data2.c_str(), Font_16x26, ILI9341_WHITE, ILI9341_BLACK);
    }
  }
  /* USER CODE END 3 */
  delete rtc;
  delete lcd;
  delete jpgDec;
  delete time;
  return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
