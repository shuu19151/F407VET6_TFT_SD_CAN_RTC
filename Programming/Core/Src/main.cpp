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
#include <sstream>
#include <vector>
#include "../Lib/DS3231/DS3231.h"
#include "../Lib/ILI9341/ILI9341.h"
#include "../Lib/ILI9341/fonts.h"
#include "../Lib/ILI9341/Image.h"
#include "../Lib/JPGEdecoder/jpgDecoder.h"
#include "../Lib/SerialDebug/UartRingBuffer.h"
#include "../Lib/Button/Button.h"
#include "../Lib/mySDCard/mySDCard.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  P_FREEZE,
  P_OK,
  P_BACK,
  P_UP,
  P_DOWN,
  P_LEFT,
  P_RIGHT,
} user_move_t;

typedef enum {
  APP_SELECTING,
  APP_CANBUS,
  APP_SDCARD,
  APP_SET_DATETIME,
  APP_SET_THEME,
  APP_SIZE = 5,
} application_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POINTER_CUR_POSITION ((Font_11x18.height * rowIndex) + 4)
#define POINTER_PREV_POSITION ((Font_11x18.height * prevRowIndex) + 4)
#define zDEBUG(msg)     do { Uart_sendstring(((std::string(msg)) + "\n").c_str()); } while(0)
#define UPDATE_POINTER  do { lcd->writeString(0, POINTER_PREV_POSITION, "  ", Font_11x18, FontColor, BGColor);  \
                        lcd->writeString(0, POINTER_CUR_POSITION, "->", Font_11x18, FontColor, BGColor); } while(0)

#define DELETE_DIALOG "-----------------------\n  | [L]: Delete [R]: No |\n  -----------------------\n"
#define MENU_UI       "---------SELECT APP----------\n  1. View CANbus\n  2. SDCard\n  3. Set datetime\n  4. Set theme\n"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t BGColor = ILI9341_BLACK;
uint16_t FontColor = ILI9341_WHITE;

uint8_t rowIndex = 1;
uint8_t prevRowIndex;
SDCard* file;
ILI9341* lcd;
JpgDecoder* jpgDec;
DS3231* rtc;
Time_t* time;
Button *btn_ok, *btn_up, *btn_down, *btn_left, *btn_right, *btn_back;

/* Application selection ---------------------------------------------------------------*/
application_t eApp = APP_SELECTING;

/* File handle ---------------------------------------------------------------*/
std::string filePath = "";
uint8_t maxPage;
uint8_t lastFileIndex;
uint8_t page = 1;

/* Text handle ---------------------------------------------------------------*/
std::string textFromFile;
uint16_t totalTextLine;
uint16_t lastTextLine = 1;
uint16_t startTextLine = 1;
uint16_t endTextLine = 13;

/* User moves & files handle -------------------------------------------------*/
user_move_t eUserMove = P_FREEZE;
file_type_t fileType = FTYP_DIR;
std::string selectedEntry;
bool buttonPressed = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool jpegRender(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if(y>=lcd->height()) return 0; // 0 - to stop decoding
  lcd->drawImage(x, y, w, h, bitmap);
  return 1; // 1 - to decode next block
}

void vUpdateLCDToSelectedDir(bool update) {
  if(update) {
    lcd->fillScreen(BGColor);
    std::string data;
    file->listEntryOfDir(filePath.c_str());
    file->listFilesOnPage(1, data);
    lcd->writeString(0, 4, data.c_str(), Font_11x18, FontColor, BGColor);
    zDEBUG(data);
  }
}

void vUpdateLCDToNextPage(bool update) {
  if(update) {
    lcd->fillScreen(BGColor);
    std::string data;
    file->listFilesOnPage(page, data);
    lcd->writeString(0, 4, data.c_str(), Font_11x18, FontColor, BGColor);
  }
}

file_type_t vPointerSDFile(user_move_t& um) {
  prevRowIndex = rowIndex;
  bool updateLCD = false;
  switch(um) {
    case P_FREEZE: return FTYP_DIR;
    case P_BACK: {
      if(filePath != "") {
        filePath = filePath.substr(0, filePath.find_last_of('/'));
        file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
        vUpdateLCDToSelectedDir(true);
        rowIndex = 1;
        page = 1;
        UPDATE_POINTER;
      }
      else {
        eApp = APP_SELECTING;
         return FTYP_DIR;
      }
    } break;
    case P_UP: {
      if(rowIndex == 1) {
        if(1 == maxPage) {
          rowIndex = lastFileIndex;
        }
        else {
          page--;
          if(page == 0) {
            page = maxPage;
            rowIndex = lastFileIndex;
          } else rowIndex = 12;
          updateLCD = true;
        }
      } else rowIndex--;
    } break;
    case P_DOWN: {
      if((1 == maxPage) && (rowIndex == lastFileIndex)) {
        rowIndex = 1;
      }
      else if((page == maxPage) && (rowIndex == lastFileIndex)) {
        rowIndex = 1;
        page = 1;
        updateLCD = true;
      }
      else if(rowIndex == 12) {
        rowIndex = 1;
        page++;
        if(page > maxPage) page = 1;
        updateLCD = true;
      } else rowIndex++;
    } break;
    case P_OK: {
      selectedEntry.clear();
      selectedEntry += filePath; // include the path
      fileType = file->checkSelectedFileType(rowIndex, selectedEntry);
      // zDEBUG(selectedEntry);
      um = P_FREEZE;
      if(fileType == FTYP_DIR) {
        rowIndex = 1;
        page = 1;
        std::string rawDir;
        for(auto c : selectedEntry) {
        	if(c != '[' && c != ']') {
        		rawDir += c;
        	}
        }
        filePath.clear();
        filePath = rawDir;
        file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
        vUpdateLCDToSelectedDir(true);
        UPDATE_POINTER;
      }
      return fileType;
    }
    case P_LEFT: { // If user is browsing files then hit button left, ask them whether they want to delete the selected file
      lcd->writeString(20, (Font_11x18.height)*(rowIndex+1.25), DELETE_DIALOG, Font_11x18, FontColor, BGColor);
      bool deleteFile = false;
      eUserMove = P_FREEZE;
      while(!deleteFile) {
        btn_back->handleButton();
        if(eUserMove == P_BACK) {
          if(filePath != "") {
          filePath = filePath.substr(0, filePath.find_last_of('/'));
          file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
          vUpdateLCDToSelectedDir(true);
          rowIndex = 1;
          page = 1;
          UPDATE_POINTER;
          }
          else {
            eApp = APP_SELECTING;
          }
          um = P_FREEZE;
          return FTYP_DIR;
        }
        btn_left->handleButton();
        btn_right->handleButton();
        if(eUserMove == P_LEFT) {
          selectedEntry.clear();
          selectedEntry += filePath; // include the path
          file->checkSelectedFileType(rowIndex, selectedEntry);
          file->deleteFile(selectedEntry.c_str());
          lcd->fillScreen(BGColor);
          file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
          vUpdateLCDToSelectedDir(true);
          rowIndex = 1;
          page = 1;
          deleteFile = true;
          UPDATE_POINTER;
          um = P_FREEZE;
          eUserMove = P_FREEZE;
          return FTYP_DIR;
        }
        else if(eUserMove == P_RIGHT) {
          lcd->fillScreen(BGColor);
          file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
          vUpdateLCDToSelectedDir(true);
          rowIndex = 1;
          page = 1;
          UPDATE_POINTER;
          um = P_FREEZE;
          eUserMove = P_FREEZE;
          return FTYP_DIR;
        }
      }
    }
    case P_RIGHT: return FTYP_DIR;
  }
  zDEBUG(std::to_string(rowIndex));
  vUpdateLCDToNextPage(updateLCD);
  UPDATE_POINTER;
  um = P_FREEZE;
  return FTYP_DIR;
}

void vHandleFileBrowse(user_move_t um) {
  switch(fileType) {
    case FTYP_DIR: { // if user is browsing directory
      switch(vPointerSDFile(um)) {
        case FTYP_DIR: {
          // vPointerSDFile will handle browsing directory
          return;
        }
        case FTYP_TXT: { //if user select text file
          totalTextLine = 0;
          textFromFile.clear();
          file->readTextFromFile(selectedEntry, textFromFile, 1024, 1, 14);
          if(textFromFile.size() > 0) {
            std::stringstream ss(textFromFile);
            std::string line;
            while(getline(ss, line)) {
              totalTextLine++;
            }
            lcd->fillScreen(BGColor);
            lcd->writeString(0, 4, textFromFile.c_str(), Font_11x18, FontColor, BGColor);
          }
        } break;
        case FTYP_JPG: {
          jpgDec->show(0, 0, selectedEntry.c_str());
        } break;
      }
    }
    break;
    case FTYP_TXT: { // if user is reading text file
      switch(um) {
        case P_LEFT: break;
        case P_RIGHT: break;
        case P_OK:      break;
        case P_FREEZE:  break;
        case P_UP: {
          if(startTextLine > 1) {
            startTextLine-=13;
            endTextLine-=13;
            textFromFile.clear();
            file->readTextFromFile(selectedEntry, textFromFile, 1024, startTextLine, endTextLine);
            std::stringstream ss(textFromFile);
            std::string line;
            std::string showText;
            uint8_t lineCount = 0;
            while(getline(ss, line) && lineCount < endTextLine) {
                showText += line + "\n";
                lineCount++;
            }
            lcd->fillScreen(BGColor);
            lcd->writeString(0, 4, showText.c_str(), Font_11x18, FontColor, BGColor);
          }
        } break;
        case P_DOWN: {
          std::string check; // check if can scroll more
          file->readTextFromFile(selectedEntry, check, 1024, startTextLine+13, endTextLine+13);
          if(check.size() <= 0) {
            return;
          }
          else {
            startTextLine+=13;
            endTextLine+=13;
            textFromFile.clear();
            file->readTextFromFile(selectedEntry, textFromFile, 1024, startTextLine, endTextLine);
            std::stringstream ss(textFromFile);
            std::string line;
            std::string showText;
            uint8_t lineCount = 0;
            while(getline(ss, line) && lineCount < 13) {
            	showText += line + "\n";
            	lineCount++;
            }
            lcd->fillScreen(BGColor);
            lcd->writeString(0, 4, showText.c_str(), Font_11x18, FontColor, BGColor);
          }
        } break;
        case P_BACK: {
          fileType = FTYP_DIR;
          startTextLine = 1;
          endTextLine = 12;
          vUpdateLCDToNextPage(true);
          UPDATE_POINTER;
          break;
        }
      }
    }
    break;
    case FTYP_JPG: { // if user is viewing jpg file
      switch(um) {
        case P_LEFT: break;
        case P_RIGHT: break;
        case P_OK:      break;
        case P_FREEZE:  break;
        case P_UP:      break;
        case P_DOWN:    break;
        case P_BACK: {
          fileType = FTYP_DIR;
          vUpdateLCDToNextPage(true);
          UPDATE_POINTER;
          break;
        }
      }
    }
    break;
  }
}

void buttonCb(uint8_t id, Button_event_state_t e) {
	if(id == btn_ok->buttonId) {
		switch(e) {
		case BUTTON_RELEASED:
			zDEBUG("Button OK released!");
      buttonPressed = true;
      eUserMove = P_OK;
			break;
		default: break;
		}
	}
	else if(id == btn_up->buttonId) {
		switch(e) {
		case BUTTON_RELEASED:
      zDEBUG("Button UP released!");
      buttonPressed = true;
      eUserMove = P_UP;
			break;
		default: break;
		}
	}
	else if(id == btn_down->buttonId) {
		switch(e) {
		case BUTTON_RELEASED:
      zDEBUG("Button DOWN released!");
      buttonPressed = true;
      eUserMove = P_DOWN;
			break;
		default: break;
		}
	}
  else if(id == btn_back->buttonId) {
    switch(e) {
    case BUTTON_RELEASED:
      zDEBUG("Button BACK released!");
      buttonPressed = true;
      eUserMove = P_BACK;
      break;
    default: break;
    }
  }
  else if(id == btn_left->buttonId) {
    switch(e) {
    case BUTTON_RELEASED:
      zDEBUG("Button LEFT released!");
      buttonPressed = true;
      eUserMove = P_LEFT;
      break;
    default: break;
    }
  }
  else if(id == btn_right->buttonId) {
    switch(e) {
    case BUTTON_RELEASED:
      zDEBUG("Button RIGHT released!");
      buttonPressed = true;
      eUserMove = P_RIGHT;
      break;
    default: break;
    }
  }
}

void vShowMenu(void) {
  lcd->fillScreen(BGColor);
  std::string menu = MENU_UI;
  lcd->writeString(0, 4, menu.c_str(), Font_11x18, FontColor, BGColor);
  rowIndex = 1;
  UPDATE_POINTER;
}

void vPointerSelectApp(user_move_t& um) {
  prevRowIndex = rowIndex;
  switch(um) {
    case P_LEFT: break;
    case P_RIGHT: break;
    case P_FREEZE: return;
    case P_BACK: {
      // eApp = APP_SELECTING;
      // vSelectingApp();
    } break;
    case P_UP: {
      if(rowIndex == 1) rowIndex = APP_SIZE;
      else rowIndex--;
    } break;
    case P_DOWN: {
      if(rowIndex == APP_SIZE) rowIndex = 1;
      else rowIndex++;
    } break;
    case P_OK: {
      switch(rowIndex) {
        case 1: {
          eApp = APP_CANBUS;
          um = P_FREEZE;
          return;
        }
        case 2: {
          eApp = APP_SDCARD;
          um = P_FREEZE;
          return;
        }
        case 3: {
          eApp = APP_SET_DATETIME;
          um = P_FREEZE;
          return;
        }
        case 4: {
          eApp = APP_SET_THEME;
          um = P_FREEZE;
          return;
        }
      }
    } break;
  }
  UPDATE_POINTER;
  um = P_FREEZE;
}

void vAppSDCard(void) {
  file = new SDCard();
  file->getPagesAndLastRow(filePath.c_str(), maxPage, lastFileIndex);
  rowIndex = 1;
  { // debug
    std::string data;
    file->scanFiles(filePath.c_str(), data);
    zDEBUG(data);
  }
  vUpdateLCDToSelectedDir(true);
  UPDATE_POINTER;

  { // debug
    std::string data;
    file->getFilesMap(data);
    zDEBUG(data);
  }
  while(eApp == APP_SDCARD) {
    btn_left->handleButton();
    btn_ok->handleButton();
    btn_up->handleButton();
    btn_down->handleButton();
    btn_right->handleButton();
    btn_back->handleButton();
    if(buttonPressed) {
      buttonPressed = false;
      vHandleFileBrowse(eUserMove);
    }
  }
  delete file;
  file = nullptr;
}

void vAppCANBus(void) {
  lcd->fillScreen(BGColor);
  HAL_CAN_Start(&hcan1);
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  TxHeader.DLC = 8;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.StdId = 0x407;
  TxHeader.TransmitGlobalTime = DISABLE;

  uint32_t TxMailbox;
  uint8_t CANTxData[8] = {0};
  time = new Time_t {0};
  rtc = new DS3231(hi2c1);
  file = new SDCard();

  uint8_t i = 10;
  uint8_t j = 1;
  uint32_t interval = 0;
  while(eApp == APP_CANBUS) {
    btn_back->handleButton();
    if(eUserMove == P_BACK) {
      eApp = APP_SELECTING;
      /* reset CANRxData */
      for(uint8_t i = 0; i < 8; i++) {
        CANRxData[i] = 0;
      }
      delete file;
      file = nullptr;
      delete time;
      time = nullptr;
      delete rtc;
      rtc = nullptr;
      return;
    }
    if(HAL_GetTick() - interval >= 1000) {
      interval = HAL_GetTick();
      rtc->getDateTime(time);
    	std::string time_str;
    	time_str += std::to_string(time->Hour);
    	time_str += " : ";
    	time_str += std::to_string(time->Min);
    	time_str += " : ";
    	time_str += std::to_string(time->Sec);
    	CANTxData[0] = ++i;
    	CANTxData[1] = ++j;
    	HAL_CAN_AddTxMessage(&hcan1, &TxHeader, CANTxData, &TxMailbox);

    	std::string CAN_label = "CAN data received:";
    	std::string CAN_data1 = "data0: ";
    	CAN_data1 += std::to_string(CANRxData[0]);
    	std::string CAN_data2 = "data1: ";
    	CAN_data2 += std::to_string(CANRxData[1]);
    	lcd->writeString(0, 4, time_str.c_str(), Font_11x18, FontColor, BGColor);
    	lcd->writeString(0, (Font_11x18.height)+4, CAN_label.c_str(), Font_11x18, FontColor, BGColor);
    	lcd->writeString(0, ((Font_11x18.height)*2)+4, CAN_data1.c_str(), Font_11x18, FontColor, BGColor);
    	lcd->writeString(0, ((Font_11x18.height)*3)+4, CAN_data2.c_str(), Font_11x18, FontColor, BGColor);
      std::string strDT;
      
      rtc->getStringDateTime(strDT);
      strDT += " data1: "; strDT += std::to_string(CANRxData[0]); strDT += " data2:"; strDT += std::to_string(CANRxData[1]); strDT += "\n";
      file->writeToFile("CANBusdata.txt", strDT);
    }
  }
}

void vAppSetDateTime(void) {
  lcd->fillScreen(BGColor);
  std::map<uint8_t, std::string> hour = {{0, "00"}, {1, "01"}, {2, "02"}, {3, "03"}, {4, "04"}, {5, "05"}, {6, "06"}, {7, "07"}, {8, "08"}, {9, "09"}, {10, "10"}, {11, "11"}, {12, "12"},
                                        {13, "13"}, {14, "14"}, {15, "15"}, {16, "16"}, {17, "17"}, {18, "18"}, {19, "19"}, {20, "20"}, {21, "21"}, {22, "22"}, {23, "23"}};
  std::map<uint8_t, std::string> sec_min = {{0, "00"}, {1, "01"}, {2, "02"}, {3, "03"}, {4, "04"}, {5, "05"}, {6, "06"}, {7, "07"}, {8, "08"}, {9, "09"}, {10, "10"}, {11, "11"}, {12, "12"},
                                        {13, "13"}, {14, "14"}, {15, "15"}, {16, "16"}, {17, "17"}, {18, "18"}, {19, "19"}, {20, "20"}, {21, "21"}, {22, "22"}, {23, "23"}, {24, "24"}, {25, "25"},
                                        {26, "26"}, {27, "27"}, {28, "28"}, {29, "29"}, {30, "30"}, {31, "31"}, {32, "32"}, {33, "33"}, {34, "34"}, {35, "35"}, {36, "36"}, {37, "37"}, {38, "38"},
                                        {39, "39"}, {40, "40"}, {41, "41"}, {42, "42"}, {43, "43"}, {44, "44"}, {45, "45"}, {46, "46"}, {47, "47"}, {48, "48"}, {49, "49"}, {50, "50"}, {51, "51"},
                                        {52, "52"}, {53, "53"}, {54, "54"}, {55, "55"}, {56, "56"}, {57, "57"}, {58, "58"}, {59, "59"}};
  std::map<uint8_t, std::string> date = {{1, "01"}, {2, "02"}, {3, "03"}, {4, "04"}, {5, "05"}, {6, "06"}, {7, "07"}, {8, "08"}, {9, "09"}, {10, "10"}, {11, "11"}, {12, "12"}, {13, "13"},
                                        {14, "14"}, {15, "15"}, {16, "16"}, {17, "17"}, {18, "18"}, {19, "19"}, {20, "20"}, {21, "21"}, {22, "22"}, {23, "23"}, {24, "24"}, {25, "25"}, {26, "26"},
                                        {27, "27"}, {28, "28"}, {29, "29"}, {30, "30"}, {31, "31"}};
  std::map<uint8_t, std::string> month = {{1, "01"}, {2, "02"}, {3, "03"}, {4, "04"}, {5, "05"}, {6, "06"}, {7, "07"}, {8, "08"}, {9, "09"}, {10, "10"}, {11, "11"}, {12, "12"}};
  std::map<uint8_t, std::string> year = {{24, "2024"} , {25, "2025"}, {26, "2026"}, {27, "2027"}, {28, "2028"}, {29, "2029"}, {30, "2030"}, {31, "2031"}, {32, "2032"}, {33, "2033"},
                                        {34, "2034"}, {35, "2035"}, {36, "2036"}, {37, "2037"}, {38, "2038"}, {39, "2039"}, {40, "2040"}, {41, "2041"}, {42, "2042"}, {43, "2043"}, {44, "2044"},
                                        {45, "2045"}};
  std::map<uint8_t, std::string> dow = {{1, "Mon"}, {2, "Tue"}, {3, "Wed"}, {4, "Thu"}, {5, "Fri"}, {6, "Sat"}, {7, "Sun"}};

#define DOW_POSX    (40)
#define HOUR_POSX   (unsigned char)(DOW_POSX + Font_16x26.width*4)
#define MINUTE_POSX (unsigned char)(HOUR_POSX + Font_16x26.width*3)
#define SECOND_POSX (unsigned char)(MINUTE_POSX + Font_16x26.width*3)
#define DATE_POSX   (unsigned char)(DOW_POSX + Font_16x26.width)
#define MONTH_POSX  (unsigned char)(DATE_POSX + Font_16x26.width*3)
#define YEAR_POSX   (unsigned char)(MONTH_POSX + Font_16x26.width*3)

#define DOW_POSY    (unsigned char)(Font_16x26.height+4)
#define HOUR_POSY   DOW_POSY
#define MINUTE_POSY DOW_POSY
#define SECOND_POSY DOW_POSY
#define DATE_POSY   (unsigned char)(DOW_POSY + (Font_16x26.height+4))
#define MONTH_POSY  DATE_POSY
#define YEAR_POSY   DATE_POSY

#define FLASH_SELECTION(x, y, str, blank) (flash ? lcd->writeString(x, y, str.c_str(), Font_16x26, FontColor, BGColor) : lcd->writeString(x, y, blank.c_str(), Font_16x26, FontColor, BGColor))

  typedef enum {
    SEL_DOW,
    SEL_HOUR,
    SEL_MINUTE,
    SEL_SECOND,
    SEL_DATE,
    SEL_MONTH,
    SEL_YEAR,
  } selecting_obj_t;

  rtc = new DS3231(hi2c1);
  Time_t* pTimeTemp = new Time_t{0};
  rtc->getDateTime(pTimeTemp);
  uint32_t interval = 0;
  bool flash = false;
  uint16_t xPos = DOW_POSX;
  uint16_t yPos = DOW_POSY;
  std::vector<uint8_t> vXPos = {DOW_POSX, HOUR_POSX, MINUTE_POSX, SECOND_POSX, DATE_POSX, MONTH_POSX, YEAR_POSX};
  std::vector<uint8_t> vYPos = {DOW_POSY, HOUR_POSY, MINUTE_POSY, SECOND_POSY, DATE_POSY, MONTH_POSY, YEAR_POSY};
  std::map<uint8_t, std::string> mpStr = {{SEL_DOW, dow[pTimeTemp->DaysOfWeek]}, {SEL_HOUR, hour[pTimeTemp->Hour]}, {SEL_MINUTE, sec_min[pTimeTemp->Min]}, {SEL_SECOND, sec_min[pTimeTemp->Sec]}, {SEL_DATE, date[pTimeTemp->Date]}, {SEL_MONTH, month[pTimeTemp->Month]}, {SEL_YEAR, year[pTimeTemp->Year]}};
  uint8_t i = 0;
  std::string str = dow[pTimeTemp->DaysOfWeek];
  std::string blank = "   ";
  lcd->writeString(0, 4, "Set date and time", Font_16x26, FontColor, BGColor);
  lcd->writeString(DOW_POSX, DOW_POSY, dow[pTimeTemp->DaysOfWeek].c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(HOUR_POSX, HOUR_POSY, (hour[pTimeTemp->Hour] + ":").c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(MINUTE_POSX, MINUTE_POSY, (sec_min[pTimeTemp->Min] + ":").c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(SECOND_POSX, SECOND_POSY, sec_min[pTimeTemp->Sec].c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(DATE_POSX, DATE_POSY, (date[pTimeTemp->Date] + "-").c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(MONTH_POSX, MONTH_POSY, (month[pTimeTemp->Month] + "-").c_str(), Font_16x26, FontColor, BGColor);
  lcd->writeString(YEAR_POSX, YEAR_POSY, year[pTimeTemp->Year].c_str(), Font_16x26, FontColor, BGColor);
  while(eApp == APP_SET_DATETIME) {
    btn_back->handleButton();
    if(eUserMove == P_BACK) {
      eApp = APP_SELECTING;
      delete pTimeTemp;
      pTimeTemp = nullptr;
      delete rtc;
      rtc = nullptr;
      return;
    }
    if(HAL_GetTick() - interval >= 500) {
      interval = HAL_GetTick();
      flash = !flash;
      FLASH_SELECTION(xPos, yPos, str, blank);
    }
    btn_ok->handleButton();
    btn_left->handleButton();
    btn_right->handleButton();
    btn_up->handleButton();
    btn_down->handleButton();

    if (eUserMove != P_FREEZE) {
      interval = 0;
      switch (eUserMove) {
        case P_BACK:    break;
        case P_FREEZE:  break;
        case P_OK: {
          rtc->setDateTime(pTimeTemp);
          lcd->writeString(40, 120, "SAVED!", Font_16x26, FontColor, BGColor);
        } break;
        case P_LEFT : {
          lcd->writeString(vXPos[i], vYPos[i], mpStr[i].c_str(), Font_16x26, FontColor, BGColor);
          i == 0 ? i = 6 : --i;
          str = mpStr[i];
        } break;
        case P_RIGHT: {
          lcd->writeString(vXPos[i], vYPos[i], mpStr[i].c_str(), Font_16x26, FontColor, BGColor);
          i == 6 ? i = 0 : ++i;
          str = mpStr[i];
        } break;
        case P_UP: {
          switch(i) {
            case SEL_DOW: {
                xPos = DOW_POSX;
                yPos = DOW_POSY;
                ++(pTimeTemp->DaysOfWeek);
                if (pTimeTemp->DaysOfWeek == 8) pTimeTemp->DaysOfWeek = 1;
                str = dow[pTimeTemp->DaysOfWeek];
              } break;
            case SEL_HOUR: {
                xPos = HOUR_POSX;
                yPos = HOUR_POSY;
                ++(pTimeTemp->Hour);
                if (pTimeTemp->Hour == 24) pTimeTemp->Hour = 0;
                str = hour[pTimeTemp->Hour];
            } break;
            case SEL_MINUTE: {
                xPos = MINUTE_POSX;
                yPos = MINUTE_POSY;
                ++(pTimeTemp->Min);
                if (pTimeTemp->Min == 60) pTimeTemp->Min = 0;
                str = sec_min[pTimeTemp->Min];
            } break;
            case SEL_SECOND: {
                xPos = SECOND_POSX;
                yPos = SECOND_POSY;
                ++(pTimeTemp->Sec);
                if (pTimeTemp->Sec == 60) pTimeTemp->Sec = 0;
                str = sec_min[pTimeTemp->Sec];
            } break;
            case SEL_DATE: {
                xPos = DATE_POSX;
                yPos = DATE_POSY;
                ++(pTimeTemp->Date);
                if (pTimeTemp->Date == 32) pTimeTemp->Date = 1;
                str = date[pTimeTemp->Date];
            } break;
            case SEL_MONTH: {
                xPos = MONTH_POSX;
                yPos = MONTH_POSY;
                ++(pTimeTemp->Month);
                if (pTimeTemp->Month == 13) pTimeTemp->Month = 1;
                str = month[pTimeTemp->Month];
            } break;
            case SEL_YEAR: {
                xPos = YEAR_POSX;
                yPos = YEAR_POSY;
                ++(pTimeTemp->Year);
                if (pTimeTemp->Year == 46) pTimeTemp->Year = 24;
                str = year[pTimeTemp->Year];
            } break;
          }
        } break;
        case P_DOWN: {
          switch(i) {
            case SEL_DOW: {
                xPos = DOW_POSX;
                yPos = DOW_POSY;
                --(pTimeTemp->DaysOfWeek);
                if (pTimeTemp->DaysOfWeek == 0) pTimeTemp->DaysOfWeek = 7;
                str = dow[pTimeTemp->DaysOfWeek];
              } break;
            case SEL_HOUR: {
                xPos = HOUR_POSX;
                yPos = HOUR_POSY;
                --(pTimeTemp->Hour);
                if (pTimeTemp->Hour == 0) pTimeTemp->Hour = 23;
                str = hour[pTimeTemp->Hour];
            } break;
            case SEL_MINUTE: {
                xPos = MINUTE_POSX;
                yPos = MINUTE_POSY;
                --(pTimeTemp->Min);
                if (pTimeTemp->Min == 0) pTimeTemp->Min = 59;
                str = sec_min[pTimeTemp->Min];
            } break;
            case SEL_SECOND: {
                xPos = SECOND_POSX;
                yPos = SECOND_POSY;
                --(pTimeTemp->Sec);
                if (pTimeTemp->Sec == 0) pTimeTemp->Sec = 59;
                str = sec_min[pTimeTemp->Sec];
            } break;
            case SEL_DATE: {
                xPos = DATE_POSX;
                yPos = DATE_POSY;
                --(pTimeTemp->Date);
                if (pTimeTemp->Date == 0) pTimeTemp->Date = 31;
                str = date[pTimeTemp->Date];
            } break;
            case SEL_MONTH: {
                xPos = MONTH_POSX;
                yPos = MONTH_POSY;
                --(pTimeTemp->Month);
                if (pTimeTemp->Month == 0) pTimeTemp->Month = 12;
                str = month[pTimeTemp->Month];
            } break;
            case SEL_YEAR: {
                xPos = YEAR_POSX;
                yPos = YEAR_POSY;
                --(pTimeTemp->Year);
                if (pTimeTemp->Year == 23) pTimeTemp->Year = 45;
                str = year[pTimeTemp->Year];
            } break;
          }
        } break;
        default:
          break;
      }
      xPos = vXPos[i];
      yPos = vYPos[i];
      blank.resize(str.size(), ' ');
      mpStr[i] = str;
      eUserMove = P_FREEZE;
    }
  }
}

void vAppSetTheme(void) {
  rowIndex = 1;
  prevRowIndex = rowIndex;
  lcd->fillScreen(BGColor);
  lcd->writeString(0, 4, "  Set theme", Font_16x26, FontColor, BGColor);
  lcd->writeString(0, Font_16x26.height + 4, "  1. Dark", Font_16x26, FontColor, BGColor);
  lcd->writeString(0, (Font_16x26.height)*2 + 4, "  2. Light", Font_16x26, FontColor, BGColor);
  lcd->writeString(0, (Font_16x26.height)*3 + 4, "  3. Color", Font_16x26, FontColor, BGColor);
  lcd->writeString(0, (Font_16x26.height)*4 + 4, "  4. Sport", Font_16x26, FontColor, BGColor);
  lcd->writeString(0, (Font_16x26.height * rowIndex) + 4, "->", Font_16x26, FontColor, BGColor);
  while(eApp == APP_SET_THEME) {
    btn_back->handleButton();
    if(eUserMove == P_BACK) {
      eApp = APP_SELECTING;
      return;
    }
    prevRowIndex = rowIndex;
    btn_up->handleButton();
    btn_down->handleButton();
    btn_ok->handleButton();
    switch(eUserMove) {
      case P_UP: {
        if(rowIndex == 1) rowIndex = 4;
        else rowIndex--;
      } break;
      case P_DOWN: {
        if(rowIndex == 4) rowIndex = 1;
        else rowIndex++;
      } break;
      case P_OK: {
        switch(rowIndex) {
          case 1: {
            BGColor = ILI9341_BLACK;
            FontColor = ILI9341_WHITE;
          } break;
          case 2: {
            BGColor = ILI9341_WHITE;
            FontColor = ILI9341_BLACK;
          } break;
          case 3: {
            BGColor = ILI9341_BLUE;
            FontColor = ILI9341_WHITE;
          } break;
          case 4: {
            BGColor = ILI9341_BLACK;
            FontColor = ILI9341_RED;
          } break;
        }
      } break;
    }
    if(eUserMove == P_OK) {
      lcd->fillScreen(BGColor);
      lcd->writeString(0, 4, "  Set theme", Font_16x26, FontColor, BGColor);
      lcd->writeString(0, Font_16x26.height + 4, "  1. Dark", Font_16x26, FontColor, BGColor);
      lcd->writeString(0, (Font_16x26.height)*2 + 4, "  2. Light", Font_16x26, FontColor, BGColor);
      lcd->writeString(0, (Font_16x26.height)*3 + 4, "  3. Color", Font_16x26, FontColor, BGColor);
      lcd->writeString(0, (Font_16x26.height)*4 + 4, "  4. Sport", Font_16x26, FontColor, BGColor);
    }
    if(eUserMove != P_FREEZE) {
      lcd->writeString(0, (Font_16x26.height * prevRowIndex) + 4, "  ", Font_16x26, FontColor, BGColor);
      lcd->writeString(0, (Font_16x26.height * rowIndex) + 4, "->", Font_16x26, FontColor, BGColor);
    }
    eUserMove = P_FREEZE;
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
  lcd = new ILI9341(hspi2,
		  SPI2_CS_GPIO_Port, SPI2_CS_Pin,
		  SPI2_DC_GPIO_Port, SPI2_DC_Pin,
		  SPI2_RST_GPIO_Port, SPI2_RST_Pin);
  jpgDec = new JpgDecoder(jpegRender);
  jpgDec->setSwapBytes(true);

  btn_ok = new Button(GPIOE, PE4_KEY_2_Pin, BUTTON_ACTIVE_LOW, buttonCb);
  btn_up = new Button(GPIOE, PE6_KEY_4_Pin, BUTTON_ACTIVE_LOW, buttonCb);
  btn_down = new Button(GPIOE, PE3_KEY_1_Pin, BUTTON_ACTIVE_LOW, buttonCb);
  btn_left = new Button(GPIOE, PE2_KEY_0_Pin, BUTTON_ACTIVE_LOW, buttonCb);
  btn_right = new Button(GPIOE, PE5_KEY_3_Pin, BUTTON_ACTIVE_LOW, buttonCb);
  btn_back = new Button(GPIOB, PB9_KEY_5_Pin, BUTTON_ACTIVE_LOW, buttonCb);

  lcd->init();
  lcd->fillScreen(BGColor);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
  /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    vShowMenu();
    /* User have to select the application first*/
    while(eApp == APP_SELECTING) {
      btn_left->handleButton();
      btn_ok->handleButton();
      btn_up->handleButton();
      btn_down->handleButton();
      btn_right->handleButton();
      btn_back->handleButton();
      if(buttonPressed) {
        buttonPressed = false;
        vPointerSelectApp(eUserMove);
      }
    }

    switch(eApp) {
      case APP_SELECTING: break;
      case APP_SIZE: break;
      case APP_SDCARD: vAppSDCard(); break;
      case APP_CANBUS: vAppCANBus(); break;
      case APP_SET_DATETIME: vAppSetDateTime(); break;
      case APP_SET_THEME: vAppSetTheme(); break;
    }
  }
  /* USER CODE END 3 */

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
