//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   main.cpp
//          Description         :   E-Paper ECG Demo - Electrocardiograma
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66" (296x152)
//          Complier            :   g++
//          Dependencies        :   bcm2835
//     
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <csignal>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <fonts/fonts_manager.h>
#include <tyme/tyme.h>
#include <app/config.h>
#include <ecg/ecg_signal.h>

#define SCREEN 266

#define SCREEN_WIDTH  296
#define SCREEN_HEIGHT 152

#define ECG_AREA_Y     20
#define ECG_AREA_H     90
#define ECG_BASELINE_Y (ECG_AREA_Y + ECG_AREA_H / 2)

static volatile sig_atomic_t running = 1;

static void signalHandler(int) {
    running = 0;
}

static void drawChar(uint8_t* buffer, int x, int y, char c, const FontManager& fm) {
    const uint8_t* bitmap = fm.getCharBitmap(c);
    if (!bitmap) return;
    for (int col = 0; col < 5; col++) {
        uint8_t byte = bitmap[col];
        for (int row = 0; row < 8; row++) {
            if (!((byte >> row) & 0x01)) continue;
            int px = x + col;
            int py = y + row;
            if (px < 0 || px >= SCREEN_WIDTH || py < 0 || py >= SCREEN_HEIGHT) continue;
            int mirroredX = SCREEN_WIDTH - 1 - px;
            int byteIndex = (mirroredX * SCREEN_HEIGHT + py) / 8;
            int bitIndex = 7 - (py % 8);
            buffer[byteIndex] |= (1 << bitIndex);
        }
    }
}

static void drawString(uint8_t* buffer, int x, int y, const char* str, const FontManager& fm) {
    for (int i = 0; str[i]; i++) {
        drawChar(buffer, x + i * 6, y, str[i], fm);
    }
}

static void clearArea(uint8_t* buffer, int x1, int y1, int x2, int y2) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int mirroredX = SCREEN_WIDTH - 1 - x;
            int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
            int bitIndex = 7 - (y % 8);
            buffer[byteIndex] &= ~(1 << bitIndex);
        }
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "=== E-PAPER ECG DEMO ===" << std::endl;
    std::cout << "Pantalla: 2.66\" (296x152)" << std::endl;
    std::cout << "Señal: ECG simulada 72 BPM" << std::endl;
    std::cout << "Presiona Ctrl+C para salir\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() fallo" << std::endl;
        return 1;
    }

    EPAPER::EPD_Driver epaper(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W);

    std::cout << "Inicializando COG..." << std::endl;
    epaper.COG_initial();
    std::cout << "COG listo.\n" << std::endl;

    ECGSignal ecg(SCREEN_WIDTH, SCREEN_HEIGHT);

    uint8_t buffer[(SCREEN_WIDTH * SCREEN_HEIGHT) / 8];
    memset(buffer, 0x00, sizeof(buffer));

    uint8_t prevBuffer[(SCREEN_WIDTH * SCREEN_HEIGHT) / 8];
    memset(prevBuffer, 0x00, sizeof(prevBuffer));

    FontManager fm;
    fm.setFont(FONT_5x8);

    // Initial screen
    {
        memset(buffer, 0x00, sizeof(buffer));

        const char* title = "ECG DEMO - 72 BPM";
        int titleWidth = strlen(title) * 6;
        int titleX = (SCREEN_WIDTH - titleWidth) / 2;
        drawString(buffer, titleX, 8, title, fm);

        for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
            int mirroredX = SCREEN_WIDTH - 1 - x;
            int byteIndex = (mirroredX * SCREEN_HEIGHT + ECG_BASELINE_Y) / 8;
            int bitIndex = 7 - (ECG_BASELINE_Y % 8);
            buffer[byteIndex] |= (1 << bitIndex);
        }

        for (int y = ECG_AREA_Y; y < ECG_AREA_Y + ECG_AREA_H; y += 10) {
            for (int x = 10; x < 15; x++) {
                int mirroredX = SCREEN_WIDTH - 1 - x;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
                int bitIndex = 7 - (y % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }

        const char* info = "HR: 72 BPM | LEAD II";
        int infoWidth = strlen(info) * 6;
        int infoX = (SCREEN_WIDTH - infoWidth) / 2;
        drawString(buffer, infoX, SCREEN_HEIGHT - 16, info, fm);

        memcpy(prevBuffer, buffer, sizeof(buffer));
        epaper.globalUpdate(buffer, buffer);
        TYME::delay(500);
    }

    std::cout << "ECG Iniciado. Mostrando señal...\n" << std::endl;

    int prevEcgY = ecg.nextSample();
    if (prevEcgY < ECG_AREA_Y) prevEcgY = ECG_AREA_Y;
    if (prevEcgY >= ECG_AREA_Y + ECG_AREA_H) prevEcgY = ECG_AREA_Y + ECG_AREA_H - 1;

    int ecgX = 10;

    while (running) {
        int ecgY = ecg.nextSample();

        if (ecgY < ECG_AREA_Y) ecgY = ECG_AREA_Y;
        if (ecgY >= ECG_AREA_Y + ECG_AREA_H) ecgY = ECG_AREA_Y + ECG_AREA_H - 1;

        int startY = (prevEcgY < ecgY) ? prevEcgY : ecgY;
        int endY = (prevEcgY < ecgY) ? ecgY : prevEcgY;

        for (int y = startY; y <= endY; y++) {
            int mirroredX = SCREEN_WIDTH - 1 - ecgX;
            int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
            int bitIndex = 7 - (y % 8);
            buffer[byteIndex] |= (1 << bitIndex);
        }

        prevEcgY = ecgY;
        ecgX++;

        if (ecgX >= SCREEN_WIDTH - 10) {
            ecgX = 10;
            prevEcgY = ecg.nextSample();
            if (prevEcgY < ECG_AREA_Y) prevEcgY = ECG_AREA_Y;
            if (prevEcgY >= ECG_AREA_Y + ECG_AREA_H) prevEcgY = ECG_AREA_Y + ECG_AREA_H - 1;

            clearArea(buffer, 10, ECG_AREA_Y, SCREEN_WIDTH - 10, ECG_AREA_Y + ECG_AREA_H - 1);

            for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
                int mirroredX = SCREEN_WIDTH - 1 - x;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + ECG_BASELINE_Y) / 8;
                int bitIndex = 7 - (ECG_BASELINE_Y % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }

        if (ecgX % 5 == 0) {
            int amp = ecgY - ECG_BASELINE_Y;

            clearArea(buffer, SCREEN_WIDTH - 80, 1, SCREEN_WIDTH - 10, 17);

            char ampStr[16];
            snprintf(ampStr, sizeof(ampStr), "AMP:%+d", amp);
            int ampLen = strlen(ampStr);
            drawString(buffer, SCREEN_WIDTH - 10 - ampLen * 6, 8, ampStr, fm);

            epaper.fastUpdate(prevBuffer, buffer);
            memcpy(prevBuffer, buffer, sizeof(buffer));
        }

        TYME::delay(10);
    }

    std::cout << "\n\nApagando COG..." << std::endl;
    epaper.COG_powerOff();
    bcm2835_close();

    std::cout << "Programa finalizado." << std::endl;
    return 0;
}
