#include "stm32f4xx_hal.h"
#include "ILI9341.h"

ILI9341::ILI9341(SPI_HandleTypeDef& hspi, GPIO_TypeDef* csPort, uint16_t csPin, GPIO_TypeDef* dcPort, uint16_t dcPin, GPIO_TypeDef* resPort, uint16_t resPin)
    : m_hspi(hspi), m_csPort(csPort), m_csPin(csPin), m_dcPort(dcPort), m_dcPin(dcPin), m_resPort(resPort), m_resPin(resPin) {}

void ILI9341::init(void) {
	/* *** optional *** */
	if(HAL_SPI_DeInit(&m_hspi) != HAL_OK) {
		return;
	}
	m_hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* Adjust the scaler to change the loading speed */
	if(HAL_SPI_Init(&m_hspi) != HAL_OK) {
		return;
	}

	/* *** *** *** */
    unselect();
    select();
    reset();

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    writeCommand(0x01);
    HAL_Delay(1000);
        
    // POWER CONTROL A
    writeCommand(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        writeData(data, sizeof(data));
    }

    // POWER CONTROL B
    writeCommand(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        writeData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    writeCommand(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        writeData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    writeCommand(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        writeData(data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    writeCommand(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        writeData(data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    writeCommand(0xF7);
    {
        uint8_t data[] = { 0x20 };
        writeData(data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    writeCommand(0xC0);
    {
        uint8_t data[] = { 0x23 };
        writeData(data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    writeCommand(0xC1);
    {
        uint8_t data[] = { 0x10 };
        writeData(data, sizeof(data));
    }

    // VCM CONTROL
    writeCommand(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        writeData(data, sizeof(data));
    }

    // VCM CONTROL 2
    writeCommand(0xC7);
    {
        uint8_t data[] = { 0x86 };
        writeData(data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    writeCommand(0x36);
    {
        uint8_t data[] = { 0x48 };
        writeData(data, sizeof(data));
    }

    // PIXEL FORMAT
    writeCommand(0x3A);
    {
        uint8_t data[] = { 0x55 };
        writeData(data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    writeCommand(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        writeData(data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    writeCommand(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        writeData(data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    writeCommand(0xF2);
    {
        uint8_t data[] = { 0x00 };
        writeData(data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    writeCommand(0x26);
    {
        uint8_t data[] = { 0x01 };
        writeData(data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    writeCommand(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        writeData(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    writeCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        writeData(data, sizeof(data));
    }

    // EXIT SLEEP
    writeCommand(0x11);
    HAL_Delay(120);

    // TURN ON DISPLAY
    writeCommand(0x29);

    // MADCTL
    writeCommand(0x36);
    {
        uint8_t data[] = { ILI9341_ROTATION };
        writeData(data, sizeof(data));
    }
    unselect();
}

void ILI9341::select(void) {
    HAL_GPIO_WritePin(m_csPort, m_csPin, GPIO_PIN_RESET);
}

void ILI9341::unselect(void) {
    HAL_GPIO_WritePin(m_csPort, m_csPin, GPIO_PIN_SET);
}

void ILI9341::reset(void) {
    HAL_GPIO_WritePin(m_resPort, m_resPin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(m_resPort, m_resPin, GPIO_PIN_SET);
}

void ILI9341::writeCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(m_dcPort, m_dcPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&m_hspi, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

void ILI9341::writeData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(m_dcPort, m_dcPin, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&m_hspi, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

void ILI9341::writeData_DMA(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(m_dcPort, m_dcPin, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
        HAL_SPI_Transmit_DMA(&m_hspi, buff, chunk_size);
        while(m_hspi.hdmatx->State != HAL_DMA_STATE_READY) {
            // wait for transfer complete
        }
        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

void ILI9341::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // column address set
    writeCommand(0x2A); // CASET
    {
		uint8_t data[] = { static_cast<uint8_t>((x0 >> 8) & 0xFF), static_cast<uint8_t>(x0 & 0xFF), static_cast<uint8_t>((x1 >> 8) & 0xFF), static_cast<uint8_t>(x1 & 0xFF) };
		writeData(data, sizeof(data));
    }
    // row address set
    writeCommand(0x2B); // RASET
    {
        uint8_t data[] = { static_cast<uint8_t>((y0 >> 8) & 0xFF), static_cast<uint8_t>(y0 & 0xFF), static_cast<uint8_t>((y1 >> 8) & 0xFF), static_cast<uint8_t>(y1 & 0xFF) };
        writeData(data, sizeof(data));
    }

    // write to RAM
    writeCommand(0x2C); // RAMWR
}

void ILI9341::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)){
        return;
    }
    select();
    setAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color & 0xFF) };
    writeData(data, sizeof(data));
    unselect();
}

void ILI9341::writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;
    setAddressWindow(x, y, x+font.width-1, y+font.height-1);
    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color & 0xFF) };
                writeData(data, sizeof(data));
            } else {
                uint8_t data[] = { static_cast<uint8_t>(bgcolor >> 8), static_cast<uint8_t>(bgcolor & 0xFF) };
                writeData(data, sizeof(data));
            }
        }
    }
}

void ILI9341::writeString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    select();
    while(*str) {
        if(x + font.width >= ILI9341_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ILI9341_HEIGHT) {
                break;
            }
            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        writeChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    unselect();
}

void ILI9341::fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // clipping
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    select();
    setAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[] = { static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color & 0xFF) };
    HAL_GPIO_WritePin(m_dcPort, m_dcPin, GPIO_PIN_SET);
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
        	writeData((uint8_t*)data, sizeof(data));
        }
    }

    unselect();
}

void ILI9341::fillScreen(uint16_t color) {
    fillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341::drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) return;
    if((y + h - 1) >= ILI9341_HEIGHT) return;

    select();
    setAddressWindow(x, y, x+w-1, y+h-1);
//    uint8_t dataByte = static_cast<uint8_t>(*data);
#ifdef HAL_DMA_MODULE_ENABLED
    writeData_DMA((uint8_t*)data, sizeof(uint16_t)*w*h); /* Width * height * 2 <bytes> */
#else
    writeData((uint8_t*)data, sizeof(uint16_t)*w*h); /* Width * height * 2 <bytes> */
#endif
    unselect();
}

void ILI9341::drawHline(int16_t xStart, int16_t xEnd, int16_t y, uint16_t color) {
    int16_t temp;
    if(xStart > xEnd) {
        temp = xStart;
        xStart = xEnd;
        xEnd = temp;
    }
    fillRectangleClip(xStart, y, (uint16_t)((xEnd - xStart) + 1), 1, color);
}

void ILI9341::drawVline(int16_t x, int16_t yStart, int16_t yEnd, uint16_t colour) {
    int16_t temp;
    if(yStart > yEnd) {
        temp = yStart;
        yStart = yEnd;
        yEnd = temp;
    }
    fillRectangleClip(x, yStart, 1U, (uint16_t)((yEnd - yStart) + 1), colour);
}


//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
//}

void ILI9341::invertColors(bool invert) {
    select();
    writeCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
    unselect();
}

void ILI9341::setRotation(uint8_t m) {
  m_rotation = m % 4; // can't be higher than 3
  switch (m_rotation) {
  case 0:
    m_rotation = (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
    _width = ILI9341_WIDTH;
    _height = ILI9341_HEIGHT;
    break;
  case 1:
    m_rotation = (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
    _width = ILI9341_HEIGHT;
    _height = ILI9341_WIDTH;
    break;
  case 2:
    m_rotation = (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
    _width = ILI9341_WIDTH;
    _height = ILI9341_HEIGHT;
    break;
  case 3:
    m_rotation = (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
    _width = ILI9341_HEIGHT;
    _height = ILI9341_WIDTH;
    break;
  }
    select();
    writeCommand(0x36);
    writeData(&m_rotation, sizeof(m_rotation));
    unselect();
}

void ILI9341::fillRectangleClip(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    int16_t x_end, y_end;

    if((x >= (int16_t)_width - 1) || (y >= (int16_t)_height - 1)) return;
    if(x + (int16_t)w < 0 || y + (int16_t)h < 0) return;
    if(x < 0) {
        w -= (uint16_t)-x;
        x = 0;
    }
    if(y < 0) {
        h -= (uint16_t)-y;
        y = 0;
    }
    x_end = x + (int16_t)w - 1;
    if(x_end > (int16_t)_width){
        x_end = (int16_t)_width - 1;
    }
    y_end = y + (int16_t)h - 1;
    if(y_end > (int16_t)_height){
        y_end = (int16_t)_height - 1;
    }

    w = (uint16_t)(x_end - x + 1);
    h = (uint16_t)(y_end - y + 1);

    select();
    fillRectangle(x, y, w, h, color);
    unselect();
}

void ILI9341::graphicsRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
	drawHline(x, x + (int16_t)w - 1, y, color);
	drawHline(x, x + (int16_t)w - 1, y + (int16_t)h - 1, color);
	drawVline(x, y, y + (int16_t)h - 1, color);
	drawVline(x + (int16_t)w - 1, y, y + (int16_t)h - 1, color);
}













