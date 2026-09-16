#pragma once

#include <fonts/fonts.h>
#include <fonts/fonts_manager.h>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <string>
#include <memory>

namespace EPAPER_DISPLAY {

struct Point {
    int x, y;
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
};

class EpaperDisplay {
public:
    explicit EpaperDisplay(uint32_t screen_type, const EPAPER::pins_t& board_config);
    ~EpaperDisplay();

    bool init();
    void clearScreen(bool white = true);
    bool update();

    void drawPixel(int x, int y, bool black);
    void drawLine(int x0, int y0, int x1, int y1, bool black);
    void drawRectangle(int x, int y, int w, int h, bool fill, bool black);

    void drawChar(int x, int y, char c, FontType font, bool black = true);
    void drawString(int x, int y, const std::string& text, FontType font, bool black = true);
    void drawCenteredString(int y, const std::string& text, FontType font, bool black = true);

    int getTextWidth(const std::string& text, FontType font);
    int getTextHeight(FontType font);

    uint8_t* getBuffer() { return m_buffer; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    EPAPER::EPD_Driver* getDriver() { return m_driver.get(); }

private:
    void drawCharToBuffer(int x, int y, char c, FontManager& fm, bool black);
    bool isValidCoordinate(int x, int y) const;
    bool hasContentChanged() const;

    uint8_t* m_buffer;
    uint8_t* m_prevBuffer;
    int m_width;
    int m_height;
    int m_bufferSize;
    bool m_transparent;
    std::unique_ptr<EPAPER::EPD_Driver> m_driver;
    uint32_t m_screenType;
    EPAPER::pins_t m_boardConfig;
};

} // namespace EPAPER_DISPLAY
