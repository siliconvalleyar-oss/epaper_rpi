#pragma once

#include <stdint.h>

#define EPD_WIDTH       296
#define EPD_HEIGHT      152
#define EPD_BYTES_PER_COL   19
#define EPD_BUFFER_SIZE     5624

#define COLOR_WHITE     0x00
#define COLOR_BLACK     0xFF

void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color);
void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color);
uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y);
void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color);
void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color);
void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill);
void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor);
void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor);
void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size);
void epd_invertBuffer(uint8_t* buffer, uint32_t size);
