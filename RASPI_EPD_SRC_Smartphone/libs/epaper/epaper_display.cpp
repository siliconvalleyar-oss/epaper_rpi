#include <epaper/epaper_display.h>
#include <fonts/fonts_manager.h>
#include <cstring>
#include <cmath>

namespace EPAPER_DISPLAY {

EpaperDisplay::EpaperDisplay(uint32_t screen_type, const EPAPER::pins_t& board_config)
    : m_buffer(nullptr)
    , m_prevBuffer(nullptr)
    , m_width(0)
    , m_height(0)
    , m_bufferSize(0)
    , m_transparent(true)
    , m_screenType(screen_type)
    , m_boardConfig(board_config)
{
    switch(screen_type) {
        case eScreen_EPD_213:
            m_width = 212;
            m_height = 104;
            break;
        case eScreen_EPD_266:
            m_width = 296;
            m_height = 152;
            break;
        case eScreen_EPD_154:
            m_width = 200;
            m_height = 200;
            break;
        default:
            m_width = 296;
            m_height = 152;
            break;
    }

    m_bufferSize = (m_width * m_height) / 8;
    m_buffer = new uint8_t[m_bufferSize]();
    m_prevBuffer = new uint8_t[m_bufferSize]();

    m_driver = std::make_unique<EPAPER::EPD_Driver>(screen_type, m_boardConfig);

    memset(m_prevBuffer, 0xFF, m_bufferSize);
    clearScreen(true);
}

EpaperDisplay::~EpaperDisplay() {
    if (m_buffer) {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
    if (m_prevBuffer) {
        delete[] m_prevBuffer;
        m_prevBuffer = nullptr;
    }
}

bool EpaperDisplay::init() {
    if (!m_driver) return false;
    m_driver->COG_initial();
    clearScreen(true);
    m_driver->globalUpdate(m_buffer, m_buffer);
    memcpy(m_prevBuffer, m_buffer, m_bufferSize);
    return true;
}

void EpaperDisplay::clearScreen(bool white) {
    if (!m_buffer) return;
    uint8_t value = white ? 0x00 : 0xFF;
    memset(m_buffer, value, m_bufferSize);
}

bool EpaperDisplay::hasContentChanged() const {
    return memcmp(m_buffer, m_prevBuffer, m_bufferSize) != 0;
}

bool EpaperDisplay::update() {
    if (!m_driver || !m_buffer) return false;

    if (!hasContentChanged()) {
        return false;
    }

    m_driver->fastUpdate(m_prevBuffer, m_buffer);
    memcpy(m_prevBuffer, m_buffer, m_bufferSize);
    return true;
}

void EpaperDisplay::drawPixel(int x, int y, bool black) {
    if (!m_buffer) return;

    int mirroredX = m_width - 1 - x;

    if (!isValidCoordinate(mirroredX, y)) return;

    int byteIndex = (mirroredX * m_height + y) / 8;
    int bitIndex = 7 - (y % 8);

    if (black) {
        m_buffer[byteIndex] |= (1 << bitIndex);
    } else {
        m_buffer[byteIndex] &= ~(1 << bitIndex);
    }
}

void EpaperDisplay::drawLine(int x0, int y0, int x1, int y1, bool black) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        drawPixel(x0, y0, black);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void EpaperDisplay::drawRectangle(int x, int y, int w, int h, bool fill, bool black) {
    if (fill) {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
                drawPixel(x + i, y + j, black);
    } else {
        int x2 = x + w - 1, y2 = y + h - 1;
        drawLine(x, y, x2, y, black);
        drawLine(x2, y, x2, y2, black);
        drawLine(x2, y2, x, y2, black);
        drawLine(x, y2, x, y, black);
    }
}

void EpaperDisplay::drawCharToBuffer(int x, int y, char c, FontManager& fm, bool black) {
    const uint8_t* bitmap = fm.getCharBitmap(c);
    if (!bitmap) return;

    int width = fm.getFontWidth();
    int height = fm.getFontHeight();
    FontType type = fm.getCurrentFontType();

    if (type == FONT_16x32_BIGNUM || type == FONT_16x16_MEDNUM) {
        int bytesPerCol = height / 8;
        for (int col = 0; col < width; col++) {
            for (int row = 0; row < height; row++) {
                int byteIndex = col * bytesPerCol + (row / 8);
                int bitIndex = 7 - (row % 8);
                bool pixel = (bitmap[byteIndex] >> bitIndex) & 0x01;

                if (pixel) {
                    drawPixel(x + col, y + row, black);
                } else if (!m_transparent) {
                    drawPixel(x + col, y + row, false);
                }
            }
        }
    } else {
        for (int col = 0; col < width; col++) {
            uint8_t byte = bitmap[col];
            for (int row = 0; row < height; row++) {
                bool pixel = (byte >> row) & 0x01;

                if (pixel) {
                    drawPixel(x + col, y + row, black);
                } else if (!m_transparent) {
                    drawPixel(x + col, y + row, false);
                }
            }
        }
    }
}

void EpaperDisplay::drawChar(int x, int y, char c, FontType font, bool black) {
    FontManager fm;
    fm.setFont(font);
    drawCharToBuffer(x, y, c, fm, black);
}

void EpaperDisplay::drawString(int x, int y, const std::string& text, FontType font, bool black) {
    FontManager fm;
    fm.setFont(font);
    int charWidth = fm.getFontWidth();
    int spacing = 1;

    int currentX = x;
    for (char c : text) {
        drawCharToBuffer(currentX, y, c, fm, black);
        currentX += charWidth + spacing;
        if (currentX + charWidth > m_width) break;
    }
}

void EpaperDisplay::drawCenteredString(int y, const std::string& text, FontType font, bool black) {
    int textWidth = getTextWidth(text, font);
    int x = (m_width - textWidth) / 2;
    if (x < 0) x = 0;
    drawString(x, y, text, font, black);
}

int EpaperDisplay::getTextWidth(const std::string& text, FontType font) {
    FontManager fm;
    fm.setFont(font);
    int charWidth = fm.getFontWidth();
    int spacing = 1;
    return text.length() * (charWidth + spacing) - spacing;
}

int EpaperDisplay::getTextHeight(FontType font) {
    FontManager fm;
    fm.setFont(font);
    return fm.getFontHeight();
}

bool EpaperDisplay::isValidCoordinate(int x, int y) const {
    return (x >= 0 && x < m_width && y >= 0 && y < m_height);
}

} // namespace EPAPER_DISPLAY
