// libs/epaper/epaper_display.cpp
#include <epaper/epaper_display.h>
#include <fonts/fonts_manager.h>
#include <cstring>
#include <cmath>

namespace EPAPER_DISPLAY {

EpaperDisplay::EpaperDisplay(uint32_t screen_type, const EPAPER::pins_t& board_config)
    : m_buffer(nullptr)
    , m_width(0)
    , m_height(0)
    , m_bufferSize(0)
    , m_cursor(0, 0)
    , m_transparent(true)
    , m_screenType(screen_type)
    , m_boardConfig(board_config)
{
    // Inicializar dimensiones según el tipo de pantalla
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
    
    // Calcular tamaño del buffer
    m_bufferSize = (m_width * m_height) / 8;
    
    // Crear buffer con new (asegurar que se inicializa a 0)
    m_buffer = new uint8_t[m_bufferSize]();  // ← Importante: los paréntesis inicializan a cero
    
    // Crear driver
    m_driver = std::make_unique<EPAPER::EPD_Driver>(screen_type, m_boardConfig);
    
    // Limpiar buffer
    clearScreen(true);
}

EpaperDisplay::~EpaperDisplay() {
    if (m_buffer) {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

bool EpaperDisplay::init() {
    if (!m_driver) return false;
    
    try {
        m_driver->COG_initial();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error en init: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Error desconocido en init" << std::endl;
        return false;
    }
}

void EpaperDisplay::clearScreen(bool white) {
    if (!m_buffer) return;
    
    // Usar 0xFF para negro, 0x00 para blanco (ajustar según tu pantalla)
    uint8_t value = white ? 0x00 : 0xFF;
    memset(m_buffer, value, m_bufferSize);
}

void EpaperDisplay::update() {
    if (!m_driver || !m_buffer) return;
    
    // Enviar buffer al display - igual que en tu código que funciona
    m_driver->globalUpdate(m_buffer, m_buffer);
}

void EpaperDisplay::drawPixel(int x, int y, bool black) {
    if (!m_buffer) return;
    
    // Invertir coordenada X (espejo horizontal) para hardware e-paper
    int mirroredX = m_width - 1 - x;
    
    if (!isValidCoordinate(mirroredX, y)) return;
    
    // Organización por columnas (requerido por el driver EPD)
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
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void EpaperDisplay::drawRectangle(int x, int y, int w, int h, bool fill, bool black) {
    if (fill) {
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                drawPixel(x + i, y + j, black);
            }
        }
    } else {
        drawLine(x, y, x + w, y, black);
        drawLine(x + w, y, x + w, y + h, black);
        drawLine(x + w, y + h, x, y + h, black);
        drawLine(x, y + h, x, y, black);
    }
}

void EpaperDisplay::drawCharToBuffer(int x, int y, char c, FontManager& fm, bool black) {
    const uint8_t* bitmap = fm.getCharBitmap(c);
    if (!bitmap) return;
    
    int width = fm.getFontWidth();
    int height = fm.getFontHeight();
    
    // Para fuentes regulares (cada byte es una columna)
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            uint8_t byte = bitmap[col];
            bool pixel = (byte >> row) & 0x01;
            
            if (pixel) {
                drawPixel(x + col, y + row, black);
            } else if (!m_transparent) {
                drawPixel(x + col, y + row, false);
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

void EpaperDisplay::testPattern() {
    clearScreen(true);
    
    for (int x = 0; x < m_width; x++) {
        drawPixel(x, 0, true);
        drawPixel(x, m_height - 1, true);
    }
    for (int y = 0; y < m_height; y++) {
        drawPixel(0, y, true);
        drawPixel(m_width - 1, y, true);
    }
    
    for (int i = 0; i < std::min(m_width, m_height); i++) {
        drawPixel(i, i, true);
    }
    
    int centerX = m_width / 2;
    int centerY = m_height / 2;
    drawRectangle(centerX - 20, centerY - 20, 40, 40, false, true);
    
    drawString(10, 10, "TEST", FONT_5x8, true);
    
    update();
}

void EpaperDisplay::setCursor(int x, int y) {
    m_cursor = Point(x, y);
}

bool EpaperDisplay::isValidCoordinate(int x, int y) const {
    return (x >= 0 && x < m_width && y >= 0 && y < m_height);
}

} // namespace EPAPER_DISPLAY
