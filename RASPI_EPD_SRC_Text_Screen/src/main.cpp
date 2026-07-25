// src/main.cpp
#include <iostream>
#include <memory>
#include <cstring>
#include <unistd.h>
#include <epaper/epaper_display.h>
#include <epaper/boards.h>
#include <tyme/tyme.h>
#include <app/config.h>

int main() {
    // Inicializar bcm2835 PRIMERO (importante)
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }
    
    std::cout << "Iniciando E-Paper con sistema de fuentes..." << std::endl;
    
    // Seleccionar tipo de pantalla
    uint32_t screen_type = eScreen_EPD_266;
    
    // Configuración de pines para Raspberry Pi Zero 2W
    const EPAPER::pins_t& board = EPAPER::boardRaspberryPiZero2W;
    std::cout << "Usando configuración para Raspberry Pi Zero 2W" << std::endl;
    
    // Bloque para controlar el ciclo de vida del display
    // El destructor se ejecuta al salir del bloque, ANTES de bcm2835_close()
    {
        // Crear display
        EPAPER_DISPLAY::EpaperDisplay display(screen_type, board);
        
        // Inicializar
        if (!display.init()) {
            std::cerr << "Error al inicializar el display" << std::endl;
            bcm2835_close();
            return -1;
        }
        
        // Limpiar pantalla
        display.clearScreen(true);
        
        // ============================================
        // DIBUJAR TEXTO CON DIFERENTES FUENTES
        // ============================================
        
        std::cout << "Dibujando texto en pantalla..." << std::endl;
        
        // 1. Título con fuente gruesa
        display.drawString(10, 10, "EPAPER DISPLAY", FONT_7x8_THICK, true);
        
        // 2. Información
        display.drawString(10, 30, "Texto con fuentes:", FONT_5x8, true);
        
        // 3. Diferentes estilos de fuente
        display.drawString(10, 45, "Normal 5x8", FONT_5x8, true);
        display.drawString(10, 55, "Thick 7x8", FONT_7x8_THICK, true);
        display.drawString(10, 68, "Homespun", FONT_7x8_HOMESPUN, true);
        display.drawString(10, 80, "Tiny 3x8", FONT_3x8_TINY, true);
        display.drawString(10, 90, "Seven Seg", FONT_4x8_SEG, true);
        
        // 4. Números grandes (solo 0-9 y :)
        display.drawString(10, 105, "123456", FONT_16x32_BIGNUM, true);
        display.drawString(10, 125, "7890", FONT_16x16_MEDNUM, true);
        
        // 5. Texto centrado
        display.drawCenteredString(145, "CENTRADO", FONT_7x8_HOMESPUN, true);
        
        // 6. Mostrar información del sistema
        display.drawString(10, 160, "Sistema: OK", FONT_5x8, true);
        
        // Actualizar pantalla
        std::cout << "Actualizando pantalla..." << std::endl;
        display.update();
        
        std::cout << "Texto dibujado correctamente" << std::endl;
        std::cout << "Esperando 5 segundos..." << std::endl;
        
        // Esperar para ver el resultado
        TYME::delay(5000);
        
        // Limpiar pantalla
        std::cout << "Limpiando pantalla..." << std::endl;
        display.clearScreen(true);
        display.update();
        
        TYME::delay(1000);
        
        std::cout << "Apagando display..." << std::endl;
        
    }  // display se destruye aquí → ~Spi_t llama bcm2835_spi_end()
    
    // Ahora es seguro cerrar bcm2835
    bcm2835_close();
    
    std::cout << "Programa finalizado correctamente" << std::endl;
    
    return 0;
}
