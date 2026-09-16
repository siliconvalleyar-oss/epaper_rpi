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

struct WeatherData {
    double tempC = -999;
    double feelsLikeC = -999;
    int humidity = -1;
    double windKmph = -1;
    double pressure = -1;
    double visibility = -1;
    std::string condition;
};

static double extractDouble(const char* json, const char* key) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    char* p = strstr((char*)json, search);
    if (!p) return -999;
    p += strlen(search);
    while (*p && ((*p < '0' || *p > '9') && *p != '-' && *p != '.')) p++;
    if (!*p) return -999;
    return atof(p);
}

static std::string extractCondition(const char* json) {
    char* desc = strstr((char*)json, "\"weatherDesc\"");
    if (!desc) return "";
    char* val = strstr(desc, "\"value\"");
    if (!val) return "";
    val += 7;
    while (*val && *val != '"') val++;
    if (*val == '"') val++;
    char* end = strchr(val, '"');
    if (!end) return "";
    return std::string(val, end - val);
}

static bool fetchWeather(WeatherData& wd) {
    FILE* fp = popen(
        "curl -s --connect-timeout 10 --max-time 15 "
        "\"wttr.in/Buenos+Aires?format=j1\" 2>/dev/null",
        "r"
    );
    if (!fp) return false;

    char buf[8192];
    size_t len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF && len < sizeof(buf) - 1)
        buf[len++] = (char)c;
    buf[len] = '\0';
    pclose(fp);

    if (len < 100) return false;

    char* cc = strstr(buf, "\"current_condition\"");
    if (!cc) return false;

    wd.tempC       = extractDouble(cc, "temp_C");
    wd.feelsLikeC  = extractDouble(cc, "FeelsLikeC");
    wd.humidity    = (int)extractDouble(cc, "humidity");
    wd.windKmph    = extractDouble(cc, "windspeedKmph");
    wd.pressure    = extractDouble(cc, "pressure");
    wd.visibility  = extractDouble(cc, "visibility");
    wd.condition   = extractCondition(cc);

    return wd.tempC > -900;
}

static std::string tempStr(double t) {
    if (t < -900) return "--";
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f", t);
    return buf;
}

static std::string oneDecimal(double v) {
    if (v < -900) return "--";
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", v);
    return buf;
}

static std::string intStr(int v) {
    if (v < 0) return "--";
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", v);
    return buf;
}

static std::string capitalize(std::string s) {
    if (s.empty()) return s;
    for (size_t i = 0; i < s.size(); i++) {
        if (i == 0 || (i > 0 && s[i-1] == ' '))
            s[i] = toupper(s[i]);
    }
    return s;
}

int main() {
    setenv("TZ", "America/Argentina/Buenos_Aires", 1);
    tzset();
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    std::cout << "=== WEATHER DISPLAY ===" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() failed" << std::endl;
        return 1;
    }

    auto display = std::make_unique<EPAPER_DISPLAY::EpaperDisplay>(
        eScreen_EPD_213, EPAPER::boardRaspberryPi
    );
    if (!display->init()) {
        std::cerr << "ERROR: display init failed" << std::endl;
        return 1;
    }

    WeatherData wd;
    time_t lastFetch = 0;
    time_t lastScroll = 0;
    int fetchErrors = 0;
    int scrollPx = 0;

    while (running) {
        time_t now = time(nullptr);
        struct tm* t = localtime(&now);

        bool newData = false;
        if (now - lastFetch >= 1800 || wd.tempC < -900) {
            std::cout << "Fetching weather..." << std::endl;
            WeatherData tmp;
            if (fetchWeather(tmp)) {
                wd = tmp;
                fetchErrors = 0;
                lastFetch = now;
                newData = true;
                std::cout << "Temp: " << wd.tempC << "C, "
                          << "Humidity: " << wd.humidity << "%, "
                          << "Condition: " << wd.condition << std::endl;
            } else {
                fetchErrors++;
                std::cerr << "Fetch error #" << fetchErrors << std::endl;
            }
        }

        bool needScroll = (now - lastScroll >= 8);
        if (!needScroll && !newData) {
            sleep(1);
            continue;
        }
        if (needScroll) lastScroll = now;

        display->clearScreen(true);

        int y = 0;

        display->drawCenteredString(y, "BUENOS AIRES", FONT_7x8_THICK, true);
        y += 12;

        display->drawLine(8, y, 204, y, true);
        y += 4;

        if (wd.tempC > -900) {
            std::string cond = capitalize(wd.condition);
            display->drawCenteredString(y, cond, FONT_5x8, true);
            y += 11;

            std::string bigT = tempStr(wd.tempC);
            int bigW = bigT.size() * 17;
            int labelW = display->getTextWidth("C", FONT_7x8_THICK);
            int totalW = bigW + 4 + labelW;
            int x0 = (212 - totalW) / 2;
            if (x0 < 0) x0 = 0;

            for (char c : bigT) {
                display->drawChar(x0, y, c, FONT_16x32_BIGNUM, true);
                x0 += 17;
            }
            x0 += 4;
            display->drawString(x0, y + 20, "C", FONT_7x8_THICK, true);
            y += 36;

            char line1[64];
            snprintf(line1, sizeof(line1), "Humedad: %s%%    Viento: %s km/h",
                     intStr(wd.humidity).c_str(), oneDecimal(wd.windKmph).c_str());
            display->drawCenteredString(y, line1, FONT_5x8, true);
            y += 11;

            char line2[64];
            snprintf(line2, sizeof(line2), "Sensacion: %sC    Presion: %s hPa",
                     tempStr(wd.feelsLikeC).c_str(), oneDecimal(wd.pressure).c_str());
            display->drawCenteredString(y, line2, FONT_5x8, true);
            y += 11;

            char line3[64];
            snprintf(line3, sizeof(line3), "Visibilidad: %s km",
                     oneDecimal(wd.visibility).c_str());
            display->drawCenteredString(y, line3, FONT_5x8, true);
            y += 11;
        } else {
            std::string msg = fetchErrors > 3 ? "SIN DATOS" : "OBTENIENDO...";
            display->drawCenteredString(y, msg, FONT_16x32_BIGNUM, true);
            y += 36;
        }

        y += 2;
        display->drawLine(8, y, 204, y, true);
        y += 4;

        char timeBuf[32];
        strftime(timeBuf, sizeof(timeBuf), "%H:%M", t);
        char updateBuf[64];
        snprintf(updateBuf, sizeof(updateBuf), "actualizado %s", timeBuf);
        display->drawCenteredString(y, updateBuf, FONT_3x8_TINY, true);
        y += 10;

        std::string marqueeText;
        if (wd.tempC > -900) {
            marqueeText = "Buenos Aires | " + capitalize(wd.condition) + " | "
                "Temperatura: " + tempStr(wd.tempC) + "C | "
                "Sensacion termica: " + tempStr(wd.feelsLikeC) + "C | "
                "Humedad: " + intStr(wd.humidity) + "% | "
                "Viento: " + oneDecimal(wd.windKmph) + " km/h | "
                "Presion: " + oneDecimal(wd.pressure) + " hPa | "
                "Visibilidad: " + oneDecimal(wd.visibility) + " km | "
                "Fuente: wttr.in | ";
        } else {
            marqueeText = "Esperando datos de wttr.in ... | ";
        }

        int textW = display->getTextWidth(marqueeText, FONT_3x8_TINY);
        int gap = 30;
        int cycle = textW + gap;

        if (newData) scrollPx = 0;
        int offset = scrollPx % cycle;
        int x1 = 212 - offset;

        if (x1 + textW > 0 && x1 < 212)
            display->drawString(x1, y, marqueeText, FONT_3x8_TINY, true);

        int x2 = x1 + textW + gap;
        if (x2 + textW > 0 && x2 < 212)
            display->drawString(x2, y, marqueeText, FONT_3x8_TINY, true);

        if (display->update()) {
            std::cout << "Display updated" << std::endl;
        }

        if (newData) {
            sleep(1);
        }
        scrollPx += 4;
    }

    std::cout << "Shutting down..." << std::endl;
    display.reset();
    bcm2835_close();
    std::cout << "Done." << std::endl;
    return 0;
}
