
// libs/fonts/epaper_display.h
#ifndef EPAPER_DISPLAY_H
#define EPAPER_DISPLAY_H

#include <fonts/fonts.h>
#include <fonts/fonts_manager.h>
#include <epaper/epaper.h>      // Incluir epaper.h para pins_t y macros
#include <epaper/boards.h>      // Incluir boards.h para las configuraciones
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>              // Para sleep()

namespace EPAPER_DISPLAY {

// Estructura para un punto en la pantalla
struct Point {
    int x, y;
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
};

// Clase principal para dibujar en e-paper usando las fuentes
class EpaperDisplay {
public:
    // Constructor - Usar el namespace completo EPAPER::pins_t
    explicit EpaperDisplay(uint32_t screen_type, const EPAPER::pins_t& board_config);
    ~EpaperDisplay();
    
    // Inicialización de la pantalla
    bool init();
    
    // Limpiar pantalla (rellenar con color especificado)
    void clearScreen(bool white = true);
    
    // Actualizar pantalla (enviar buffer al display)
    void update();
    
    // Funciones de dibujo básicas
    void drawPixel(int x, int y, bool black);
    void drawLine(int x0, int y0, int x1, int y1, bool black);
    void drawRectangle(int x, int y, int w, int h, bool fill, bool black);
    
    // Funciones de texto con las fuentes
    void drawChar(int x, int y, char c, FontType font, bool black = true);
    void drawString(int x, int y, const std::string& text, FontType font, bool black = true);
    
    // Dibujar texto centrado
    void drawCenteredString(int y, const std::string& text, FontType font, bool black = true);
    
    // Obtener dimensiones del texto
    int getTextWidth(const std::string& text, FontType font);
    int getTextHeight(FontType font);
    
    // Configurar posición de cursor
    void setCursor(int x, int y);
    Point getCursor() const { return m_cursor; }
    
    // Modo de impresión (si se dibuja el fondo)
    void setTransparent(bool transparent) { m_transparent = transparent; }
    
    // Acceso al buffer de imagen (para integración con tu código existente)
    uint8_t* getBuffer() { return m_buffer; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Acceso al driver (para control de bajo nivel)
    EPAPER::EPD_Driver* getDriver() { return m_driver.get(); }
    
    // Patrón de prueba (marco + diagonal + rectángulo + texto)
    void testPattern();
    
private:
    // Dibujar un carácter en el buffer
    void drawCharToBuffer(int x, int y, char c, FontManager& fm, bool black);
    
    // Conversión de coordenadas (si es necesario)
    bool isValidCoordinate(int x, int y) const;
    
    // Buffer de la pantalla (1 bit por píxel: 1 = negro, 0 = blanco)
    uint8_t* m_buffer;
    
    // Dimensiones de la pantalla
    int m_width;
    int m_height;
    int m_bufferSize;
    
    // Posición del cursor
    Point m_cursor;
    
    // Modo transparente (si se dibuja fondo)
    bool m_transparent;
    
    // Driver del e-paper
    std::unique_ptr<EPAPER::EPD_Driver> m_driver;
    
    // Tipo de pantalla
    uint32_t m_screenType;
    
    // Configuración de pines
    EPAPER::pins_t m_boardConfig;
};

} // namespace EPAPER_DISPLAY

#endif // EPAPER_DISPLAY_H

