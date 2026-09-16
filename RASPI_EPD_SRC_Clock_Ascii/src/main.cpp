//////////////////////////////////////////////////////////////////////////////
//
//          filename            :   main.cpp
//          Description         :   E-Paper Clock - Reloj en pantalla e-paper
//          License             :   GNU
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper configurable
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

#define SCREEN 213

struct ScreenLayout {
    int width;
    int height;
    int lineStartX;
    int lineEndX;
    int titleY;
    int line1Y;
    int timeY;
    int line2Y;
    int dateY;
    int dayY;
    int uptimeY;
    int line3Y;
};

static ScreenLayout getLayout(int screen) {
    if (screen == 213) {
        return {
            212, 104,
            8, 204,
            5, 14, 16, 32,
            36, 46, 58,
            72
        };
    }
    return {
        296, 152,
        10, 285,
        5, 18, 22, 42,
        48, 65, 85, 105
    };
}

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

    uint32_t screenType = (SCREEN == 213) ? eScreen_EPD_213 : eScreen_EPD_266;
    ScreenLayout layout = getLayout(SCREEN);

    std::cout << "=== E-PAPER CLOCK ===" << std::endl;
    std::cout << "Pantalla: " << (SCREEN == 213 ? "2.13\" (212x104)" : "2.66\" (296x152)") << std::endl;
    std::cout << "Fuente reloj: FONT_16x16_MEDNUM (16x16 medium numbers)" << std::endl;
    std::cout << "Presiona Ctrl+C para salir\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() fallo" << std::endl;
        return 1;
    }

    EPAPER_DISPLAY::EpaperDisplay display(
        screenType,
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

        display.drawCenteredString(layout.titleY, "E-PAPER CLOCK", FONT_5x8, true);

        display.drawLine(layout.lineStartX, layout.line1Y, layout.lineEndX, layout.line1Y, true);

        display.drawCenteredString(layout.timeY, formatTime(t), FONT_16x16_MEDNUM, true);

        display.drawLine(layout.lineStartX, layout.line2Y, layout.lineEndX, layout.line2Y, true);

        display.drawCenteredString(layout.dateY, formatDate(t), FONT_5x8, true);

        display.drawCenteredString(layout.dayY, formatDay(t), FONT_7x8_THICK, true);

        display.drawCenteredString(layout.uptimeY, formatUptime(uptime), FONT_5x8, true);

        display.drawLine(layout.lineStartX, layout.line3Y, layout.lineEndX, layout.line3Y, true);

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
