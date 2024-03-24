#ifndef __ILI9341_H__
#define __ILI9341_H__

#include "fonts.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/* default orientation */
//#define ILI9341_WIDTH  240
//#define ILI9341_HEIGHT 320
//#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)

/* rotate right */
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)


/* rotate left */
// #define ILI9341_WIDTH  320
// #define ILI9341_HEIGHT 240
// #define ILI9341_ROTATION (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)


/* upside down */
// #define ILI9341_WIDTH  240
// #define ILI9341_HEIGHT 320
// #define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)

/****************************/

// Color definitions
#define	ILI9341_BLACK   	        (0x0000)
#define	ILI9341_BLUE    	        (0x001F)
#define	ILI9341_RED     	        (0xF800)
#define	ILI9341_GREEN   	        (0x07E0)
#define ILI9341_CYAN    	        (0x07FF)
#define ILI9341_MAGENTA 	        (0xF81F)
#define ILI9341_YELLOW  	        (0xFFE0)
#define ILI9341_WHITE   	        (0xFFFF)
#define ILI9341_COLOR565(r, g, b)   (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

class ILI9341 {
private:
    SPI_HandleTypeDef& m_hspi;
    GPIO_TypeDef* m_csPort;
    uint16_t m_csPin;
    GPIO_TypeDef* m_dcPort;
    uint16_t m_dcPin;
    GPIO_TypeDef* m_resPort;
    uint16_t m_resPin;
    uint8_t m_rotation;
    int16_t _width = ILI9341_WIDTH, _height = ILI9341_HEIGHT;
    void unselect(void);
    void select(void);
    void reset(void);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t* buff, size_t buff_size);
    void writeData_DMA(uint8_t* buff, size_t buff_size);
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
    void fillRectangleClip(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
public:
    ILI9341(SPI_HandleTypeDef& hspi, GPIO_TypeDef* csPort, uint16_t csPin,
        GPIO_TypeDef* dcPort, uint16_t dcPin, GPIO_TypeDef* resPort, uint16_t resPin);

    void init(void);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void writeString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
    void fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
    void drawHline(int16_t xStart, int16_t xEnd, int16_t y, uint16_t color);
    void drawVline(int16_t x, int16_t yStart, int16_t yEnd, uint16_t colour);
    void graphicsRectangle(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
    void invertColors(bool invert);
    void setRotation(uint8_t m);
    int16_t height(void) {
    	return _height;
    }
    int16_t width(void) {
    	return _width;
    }
};

#ifdef __cplusplus
}
#endif

#endif // __ILI9341_H__
