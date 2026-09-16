#include "epd_graphics.h"
#include <fonts/fonts.h>
#include <cstring>

void epd_clearBuffer(uint8_t* buffer, uint32_t size, uint8_t color) {
    memset(buffer, color, size);
}

void epd_drawPixel(uint8_t* buffer, uint16_t x, uint16_t y, uint8_t color) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    
    uint32_t byteIndex = x * EPD_BYTES_PER_COL + (y / 8);
    uint8_t bitMask = 0x80 >> (y % 8);
    
    if (color == COLOR_BLACK) {
        buffer[byteIndex] |= bitMask;
    } else {
        buffer[byteIndex] &= ~bitMask;
    }
}

uint8_t epd_getPixel(const uint8_t* buffer, uint16_t x, uint16_t y) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return COLOR_WHITE;
    
    uint32_t byteIndex = x * EPD_BYTES_PER_COL + (y / 8);
    uint8_t bitMask = 0x80 >> (y % 8);
    
    return (buffer[byteIndex] & bitMask) ? COLOR_BLACK : COLOR_WHITE;
}

void epd_drawHLine(uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y, uint8_t color) {
    if (y >= EPD_HEIGHT) return;
    
    if (x0 > x1) {
        uint16_t temp = x0;
        x0 = x1;
        x1 = temp;
    }
    
    if (x0 >= EPD_WIDTH) x0 = EPD_WIDTH - 1;
    if (x1 >= EPD_WIDTH) x1 = EPD_WIDTH - 1;
    
    for (uint16_t x = x0; x <= x1; x++) {
        epd_drawPixel(buffer, x, y, color);
    }
}

void epd_drawVLine(uint8_t* buffer, uint16_t x, uint16_t y0, uint16_t y1, uint8_t color) {
    if (x >= EPD_WIDTH) return;
    
    if (y0 > y1) {
        uint16_t temp = y0;
        y0 = y1;
        y1 = temp;
    }
    
    if (y0 >= EPD_HEIGHT) y0 = EPD_HEIGHT - 1;
    if (y1 >= EPD_HEIGHT) y1 = EPD_HEIGHT - 1;
    
    for (uint16_t y = y0; y <= y1; y++) {
        epd_drawPixel(buffer, x, y, color);
    }
}

void epd_drawRect(uint8_t* buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color, uint8_t fill) {
    if (fill) {
        for (uint16_t i = 0; i < h; i++) {
            epd_drawHLine(buffer, x, x + w - 1, y + i, color);
        }
    } else {
        epd_drawHLine(buffer, x, x + w - 1, y, color);
        epd_drawHLine(buffer, x, x + w - 1, y + h - 1, color);
        epd_drawVLine(buffer, x, y, y + h - 1, color);
        epd_drawVLine(buffer, x + w - 1, y, y + h - 1, color);
    }
}

void epd_drawChar(uint8_t* buffer, uint16_t x, uint16_t y, char c, uint8_t color, uint8_t bgColor) {
    if (c < 32 || c > 127) return;
    
    uint16_t fontIndex = (c - 32) * 8;
    
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t fontByte = font[fontIndex + row];
        
        for (uint8_t col = 0; col < 8; col++) {
            uint8_t bit = (fontByte >> (7 - col)) & 0x01;
            
            if (bit) {
                epd_drawPixel(buffer, x + col, y + row, color);
            } else if (bgColor != COLOR_WHITE) {
                epd_drawPixel(buffer, x + col, y + row, bgColor);
            }
        }
    }
}

void epd_drawString(uint8_t* buffer, uint16_t x, uint16_t y, const char* str, uint8_t color, uint8_t bgColor) {
    uint16_t currentX = x;
    
    while (*str) {
        if (*str == '\n') {
            y += 8;
            currentX = x;
        } else if (*str == '\r') {
            currentX = x;
        } else {
            epd_drawChar(buffer, currentX, y, *str, color, bgColor);
            currentX += 8;
        }
        str++;
    }
}

void epd_loadImage(uint8_t* buffer, const uint8_t* imageData, uint32_t size) {
    uint32_t copySize = (size < EPD_BUFFER_SIZE) ? size : EPD_BUFFER_SIZE;
    memcpy(buffer, imageData, copySize);
}

void epd_invertBuffer(uint8_t* buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = ~buffer[i];
    }
}
