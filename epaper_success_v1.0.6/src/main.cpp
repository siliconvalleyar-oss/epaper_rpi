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
#include <csignal>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <fonts/fonts_manager.h>
#include <tyme/tyme.h>
#include <app/config.h>
#include <ecg/ecg_signal.h>

#define SCREEN 266

// Screen dimensions
#define SCREEN_WIDTH  296
#define SCREEN_HEIGHT 152

// Layout
#define ECG_AREA_Y     20
#define ECG_AREA_H     90
#define ECG_BASELINE_Y (ECG_AREA_Y + ECG_AREA_H / 2)

static volatile sig_atomic_t running = 1;

static void signalHandler(int) {
    running = 0;
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

    // Create ECG signal generator
    ECGSignal ecg(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Buffer for the display
    uint8_t buffer[(SCREEN_WIDTH * SCREEN_HEIGHT) / 8];
    memset(buffer, 0x00, sizeof(buffer));

    // Previous buffer for fast update
    uint8_t prevBuffer[(SCREEN_WIDTH * SCREEN_WIDTH) / 8];
    memset(prevBuffer, 0x00, sizeof(prevBuffer));

    // ECG waveform position (scrolling from left to right)
    int ecgX = 0;
    int lastEcgY = ECG_BASELINE_Y;
    
    // Sample counter for ECG
    int sampleCount = 0;
    int samplesPerPixel = 3;  // 3 samples per pixel width
    
    // Time tracking
    int heartBeatCount = 0;

    std::cout << "Iniciando ECG..." << std::endl;

    // First update - show initial screen with title and baseline
    {
        memset(buffer, 0x00, sizeof(buffer));
        
        // Title
        FontManager fm;
        fm.setFont(FONT_5x8);
        // "ECG DEMO" centered at y=8
        const char* title = "ECG DEMO - 72 BPM";
        int titleWidth = strlen(title) * 6;
        int titleX = (SCREEN_WIDTH - titleWidth) / 2;
        // Draw title manually to buffer
        for (int i = 0; i < (int)strlen(title); i++) {
            int charX = titleX + i * 6;
            const uint8_t* bitmap = fm.getCharBitmap(title[i]);
            if (bitmap) {
                for (int col = 0; col < 5; col++) {
                    uint8_t byte = bitmap[col];
                    for (int row = 0; row < 8; row++) {
                        bool pixel = (byte >> row) & 0x01;
                        if (pixel) {
                            int px = charX + col;
                            int py = 8 + row;
                            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                                int mirroredX = SCREEN_WIDTH - 1 - px;
                                int byteIndex = (mirroredX * SCREEN_HEIGHT + py) / 8;
                                int bitIndex = 7 - (py % 8);
                                buffer[byteIndex] |= (1 << bitIndex);
                            }
                        }
                    }
                }
            }
        }
        
        // Draw horizontal line for ECG baseline
        for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
            int mirroredX = SCREEN_WIDTH - 1 - x;
            int byteIndex = (mirroredX * SCREEN_HEIGHT + ECG_BASELINE_Y) / 8;
            int bitIndex = 7 - (ECG_BASELINE_Y % 8);
            buffer[byteIndex] |= (1 << bitIndex);
        }
        
        // Draw vertical scale markers
        for (int y = ECG_AREA_Y; y < ECG_AREA_Y + ECG_AREA_H; y += 10) {
            for (int x = 10; x < 15; x++) {
                int mirroredX = SCREEN_WIDTH - 1 - x;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
                int bitIndex = 7 - (y % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }
        
        // Bottom info line
        const char* info = "HR: 72 BPM | LEAD II";
        int infoWidth = strlen(info) * 6;
        int infoX = (SCREEN_WIDTH - infoWidth) / 2;
        for (int i = 0; i < (int)strlen(info); i++) {
            int charX = infoX + i * 6;
            const uint8_t* bitmap = fm.getCharBitmap(info[i]);
            if (bitmap) {
                for (int col = 0; col < 5; col++) {
                    uint8_t byte = bitmap[col];
                    for (int row = 0; row < 8; row++) {
                        bool pixel = (byte >> row) & 0x01;
                        if (pixel) {
                            int px = charX + col;
                            int py = SCREEN_HEIGHT - 16 + row;
                            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                                int mirroredX = SCREEN_WIDTH - 1 - px;
                                int byteIndex = (mirroredX * SCREEN_HEIGHT + py) / 8;
                                int bitIndex = 7 - (py % 8);
                                buffer[byteIndex] |= (1 << bitIndex);
                            }
                        }
                    }
                }
            }
        }
        
        // Send initial frame
        memcpy(prevBuffer, buffer, sizeof(buffer));
        epaper.globalUpdate(buffer, buffer);
        TYME::delay(500);
    }

    std::cout << "ECG Iniciado. Mostrando señal...\n" << std::endl;

    // Main ECG loop
    while (running) {
        // Generate ECG samples
        for (int i = 0; i < samplesPerPixel; i++) {
            int ecgY = ecg.nextSample();
            sampleCount++;
            
            // Detect R peak for heart rate counting
            if (ecgY < ECG_BASELINE_Y - 30) {
                heartBeatCount++;
            }
            
            lastEcgY = ecgY;
        }
        
        // Draw vertical line from last position to current position
        int y1 = lastEcgY;
        int y2 = ecg.nextSample();
        
        // Draw line segment
        int startY = (y1 < y2) ? y1 : y2;
        int endY = (y1 < y2) ? y2 : y1;
        
        for (int y = startY; y <= endY; y++) {
            if (y >= ECG_AREA_Y && y < ECG_AREA_Y + ECG_AREA_H) {
                int mirroredX = SCREEN_WIDTH - 1 - ecgX;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
                int bitIndex = 7 - (y % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }
        
        // Also draw horizontal line if needed
        if (abs(y2 - y1) > 1) {
            // Already drew vertical
        } else {
            // Draw single point
            if (y2 >= ECG_AREA_Y && y2 < ECG_AREA_Y + ECG_AREA_H) {
                int mirroredX = SCREEN_WIDTH - 1 - ecgX;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + y2) / 8;
                int bitIndex = 7 - (y2 % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }
        
        // Advance X position
        ecgX++;
        
        // When we reach the right edge, scroll or reset
        if (ecgX >= SCREEN_WIDTH - 20) {
            // Reset to left side (could implement scrolling here)
            ecgX = 10;
            
            // Clear ECG area for new waveform
            for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
                for (int y = ECG_AREA_Y; y < ECG_AREA_Y + ECG_AREA_H; y++) {
                    int mirroredX = SCREEN_WIDTH - 1 - x;
                    int byteIndex = (mirroredX * SCREEN_HEIGHT + y) / 8;
                    int bitIndex = 7 - (y % 8);
                    buffer[byteIndex] &= ~(1 << bitIndex);
                }
            }
            
            // Redraw baseline
            for (int x = 10; x < SCREEN_WIDTH - 10; x++) {
                int mirroredX = SCREEN_WIDTH - 1 - x;
                int byteIndex = (mirroredX * SCREEN_HEIGHT + ECG_BASELINE_Y) / 8;
                int bitIndex = 7 - (ECG_BASELINE_Y % 8);
                buffer[byteIndex] |= (1 << bitIndex);
            }
        }
        
        // Update display every 10 pixels
        if (ecgX % 10 == 0) {
            // Fast update
            epaper.fastUpdate(prevBuffer, buffer);
            memcpy(prevBuffer, buffer, sizeof(buffer));
        }
        
        // Delay between samples (10ms = 100 Hz sample rate)
        TYME::delay(10);
    }

    std::cout << "\n\nApagando COG..." << std::endl;
    epaper.COG_powerOff();
    bcm2835_close();

    std::cout << "Programa finalizado." << std::endl;
    return 0;
}
