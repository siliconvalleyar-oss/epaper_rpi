#include <iostream>
#include <memory>
#include <cstring>
#include <cstdio>
#include <string>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include <epaper/epaper_display.h>
#include <app/config.h>

static volatile bool running = true;

static void onSignal(int) {
    running = false;
}

static std::string priceStr(double price) {
    bool neg = price < 0;
    if (neg) price = -price;
    long long intPart = (long long)price;
    int cents = (int)round((price - intPart) * 100);
    if (cents >= 100) { intPart++; cents = 0; }

    std::string s = neg ? "-" : "";
    long long t = intPart / 1000;
    long long h = intPart % 1000;

    if (t > 0) {
        s += std::to_string(t) + ",";
        char buf3[8];
        snprintf(buf3, sizeof(buf3), "%03lld", h);
        s += buf3;
    } else {
        s += std::to_string(h);
    }
    char buf[8];
    snprintf(buf, sizeof(buf), ".%02d", cents);
    s += buf;
    return s;
}

static double fetchBtcPrice() {
    FILE* fp = popen(
        "curl -s --connect-timeout 5 --max-time 10 "
        "'https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd' "
        "2>/dev/null",
        "r"
    );
    if (!fp) return -1;
    char buf[512];
    if (!fgets(buf, sizeof(buf), fp)) { pclose(fp); return -1; }
    pclose(fp);

    char* p = strstr(buf, "\"usd\":");
    if (!p) return -1;
    p += 5;
    while (*p && ((*p < '0' || *p > '9') && *p != '-' && *p != '.')) p++;
    if (!*p) return -1;
    return atof(p);
}

int main() {
    setenv("TZ", "America/Argentina/Buenos_Aires", 1);
    tzset();
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    std::cout << "=== BITCOIN TICKER ===" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() failed" << std::endl;
        return 1;
    }

    auto display = std::make_unique<EPAPER_DISPLAY::EpaperDisplay>(
        eScreen_EPD_266, EPAPER::boardRaspberryPi
    );
    if (!display->init()) {
        std::cerr << "ERROR: display init failed" << std::endl;
        return 1;
    }

    double btcPrice = -1;
    time_t lastFetch = 0;
    int fetchErrors = 0;

    while (running) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);

        if (now - lastFetch >= 60 || btcPrice < 0) {
            std::cout << "Fetching BTC price..." << std::endl;
            double p = fetchBtcPrice();
            if (p > 0) {
                btcPrice = p;
                fetchErrors = 0;
                lastFetch = now;
                std::cout << "BTC: $" << priceStr(btcPrice) << std::endl;
            } else {
                fetchErrors++;
                std::cerr << "Fetch error #" << fetchErrors << std::endl;
            }
        }

        display->clearScreen(true);

        int y = 0;
        display->drawCenteredString(y, "BITCOIN", FONT_7x8_THICK, true);
        y += 10;
        display->drawLine(10, y, 285, y, true);
        y += 6;

        if (btcPrice > 0) {
            std::string ps = priceStr(btcPrice);

            int digitCount = 0;
            for (char c : ps) if (c >= '0' && c <= '9') digitCount++;
            int dollarW = display->getTextWidth("$", FONT_5x8);
            int totalW = dollarW + 4 + digitCount * 17;
            int x0 = (296 - totalW) / 2;
            if (x0 < 0) x0 = 0;

            display->drawString(x0, y + 10, "$", FONT_5x8, true);
            x0 += dollarW + 4;

            for (char c : ps) {
                if (c == ',') continue;
                if (c == '.') {
                    display->drawChar(x0, y + 16, '.', FONT_5x8, true);
                    x0 += 6;
                } else {
                    display->drawChar(x0, y, c, FONT_16x32_BIGNUM, true);
                    x0 += 17;
                }
            }
            y += 36;

            char priceLine[32];
            snprintf(priceLine, sizeof(priceLine), "USD 1 BTC");
            display->drawCenteredString(y, priceLine, FONT_5x8, true);
            y += 12;
        } else {
            std::string msg = fetchErrors > 3 ? "NO DATA" : "FETCHING...";
            display->drawCenteredString(y, msg, FONT_8x8, true);
            y += 20;
        }

        char dateBuf[32];
        strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", t);
        display->drawCenteredString(y, dateBuf, FONT_5x8, true);
        y += 11;

        char timeBuf[32];
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S ART", t);
        display->drawCenteredString(y, timeBuf, FONT_5x8, true);
        y += 11;

        char refreshBuf[64];
        if (btcPrice > 0) {
            int secs = (int)(now - lastFetch);
            if (secs < 60)
                snprintf(refreshBuf, sizeof(refreshBuf), "actualizado hace %ds", secs);
            else
                snprintf(refreshBuf, sizeof(refreshBuf), "actualizado hace %dm", secs / 60);
        } else {
            snprintf(refreshBuf, sizeof(refreshBuf), "esperando datos...");
        }
        display->drawCenteredString(y, refreshBuf, FONT_3x8_TINY, true);
        y += 9;
        display->drawCenteredString(y, "via CoinGecko API", FONT_3x8_TINY, true);

        if (display->update()) {
            std::cout << "Display updated at " << timeBuf << std::endl;
        }

        for (int i = 0; i < 60 && running; i++) {
            sleep(1);
        }
    }

    std::cout << "Shutting down..." << std::endl;
    display.reset();
    bcm2835_close();
    std::cout << "Done." << std::endl;
    return 0;
}
