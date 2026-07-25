//////////////////////////////////////////////////////////////////////////////
//
//          filename            :   main.cpp
//          Description         :   E-Paper Clock - Reloj en pantalla e-paper
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
#include <ctime>
#include <csignal>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <epaper/epaper_display.h>
#include <fonts/fonts_manager.h>
#include <tyme/tyme.h>
#include <app/config.h>

#define SCREEN 266

// Layout de la pantalla (296 x 152 pixeles):
//
//   y=8:   "E-PAPER CLOCK"       FONT_5x8      (h=8)
//   y=22:  ───────────────       linea horizontal
//   y=32:  "12:34:56"            FONT_4x8_SEG  (h=8)   → y=32..39
//   y=48:  ───────────────       linea horizontal
//   y=58:  "2026-07-24"          FONT_5x8      (h=8)   → y=58..65
//   y=75:  "DOMINGO"             FONT_7x8_THICK(h=8)   → y=75..82
//   y=95:  "UP: 00:05:32"        FONT_5x8      (h=8)   → y=95..102
//   y=115: ───────────────       linea horizontal
//

static volatile sig_atomic_t running = 1;

static void signalHandler(int) {
    running = 0;
}

static std::string formatTime(const struct tm* t) {
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    return std::string(buf);
}

static std::string formatDate(const struct tm* t) {
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    return std::string(buf);
}

static std::string formatDay(const struct tm* t) {
    static const char* days[] = {
        "DOMINGO", "LUNES", "MARTES", "MIERCOLES",
        "JUEVES", "VIERNES", "SABADO"
    };
    return std::string(days[t->tm_wday]);
}

static std::string formatUptime(unsigned long seconds) {
    unsigned long h = seconds / 3600;
    unsigned long m = (seconds % 3600) / 60;
    unsigned long s = seconds % 60;
    char buf[14];
    snprintf(buf, sizeof(buf), "UP: %02lu:%02lu:%02lu", h, m, s);
    return std::string(buf);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "=== E-PAPER CLOCK ===" << std::endl;
    std::cout << "Pantalla: 2.66\" (296x152)" << std::endl;
    std::cout << "Fuente reloj: FONT_4x8_SEG (4x8 seven segment)" << std::endl;
    std::cout << "Presiona Ctrl+C para salir\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() fallo" << std::endl;
        return 1;
    }

    EPAPER_DISPLAY::EpaperDisplay display(
        eScreen_EPD_266,
        EPAPER::boardRaspberryPiZero2W
    );

    std::cout << "Inicializando COG..." << std::endl;
    display.init();
    std::cout << "COG listo.\n" << std::endl;

    unsigned long uptime = 0;
    int lastSec = -1;

    while (running) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);

        if (t->tm_sec == lastSec) {
            TYME::delay(100);
            continue;
        }
        lastSec = t->tm_sec;

        display.clearScreen(true);

        display.drawCenteredString(8, "E-PAPER CLOCK", FONT_5x8, true);

        int lineY1 = 20;
        display.drawLine(10, lineY1, 285, lineY1, true);

        display.drawCenteredString(32, formatTime(t), FONT_4x8_SEG, true);

        int lineY2 = 48;
        display.drawLine(10, lineY2, 285, lineY2, true);

        display.drawCenteredString(58, formatDate(t), FONT_5x8, true);

        display.drawCenteredString(75, formatDay(t), FONT_7x8_THICK, true);

        display.drawCenteredString(95, formatUptime(uptime), FONT_5x8, true);

        display.drawLine(10, 115, 285, 115, true);

        bool changed = display.update();

        if (changed) {
            std::cout << "\r  " << formatTime(t)
                      << "  |  " << formatDate(t)
                      << "  |  " << formatDay(t)
                      << "  |  " << formatUptime(uptime)
                      << "   " << std::flush;
        }

        uptime++;
    }

    std::cout << "\n\nApagando COG..." << std::endl;

    display.getDriver()->COG_powerOff();
    bcm2835_close();

    std::cout << "Programa finalizado." << std::endl;
    return 0;
}
