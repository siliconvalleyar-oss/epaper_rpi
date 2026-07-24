#pragma once

#include <stdint.h>
#include <string.h>

#define TEST_IMG_WIDTH  296
#define TEST_IMG_HEIGHT 152
#define TEST_IMG_SIZE   5624

static inline void generateTestImage(uint8_t* buffer) {
    uint16_t w = 296, h = 152;
    uint16_t bpr = w / 8;
    memset(buffer, 0x00, 5624);
    for (uint16_t x = 0; x < w; x++) {
        buffer[(0) * bpr + (x / 8)] |= (0x80 >> (x % 8));
        buffer[(h - 1) * bpr + (x / 8)] |= (0x80 >> (x % 8));
    }
    for (uint16_t y = 0; y < h; y++) {
        buffer[y * bpr + 0] |= 0x80;
        buffer[y * bpr + ((w - 1) / 8)] |= (0x80 >> ((w - 1) % 8));
    }
    for (uint16_t y = 20; y < 132; y++) {
        for (uint16_t x = 20; x < 276; x++) {
            if (((x + y) & 1) == 0) {
                buffer[y * bpr + (x / 8)] |= (0x80 >> (x % 8));
            }
        }
    }
    for (uint16_t i = 0; i < w && i < h; i++) {
        buffer[i * bpr + (i / 8)] |= (0x80 >> (i % 8));
        buffer[i * bpr + ((w - 1 - i) / 8)] |= (0x80 >> ((w - 1 - i) % 8));
    }
}

