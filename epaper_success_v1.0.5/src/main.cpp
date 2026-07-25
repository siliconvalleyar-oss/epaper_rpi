#include <iostream>
#include <memory>
#include <cstring>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#define SCREEN 266
#include <graphics/userImageData.h>
#include <tyme/tyme.h>
#include <app/config.h>

// Descomentar para ejecutar test de pines GPIO en lugar del flujo normal
// #define TEST_PINS

// ============================================================================
// FUNCIONES DE TEST DE PINES GPIO (como en PIC32)
// ============================================================================
#if defined(TEST_PINS)

// Pines del display para boardRaspberryPiZero2W
#define PIN_BUSY    25
#define PIN_DC      24
#define PIN_RESET   23
#define PIN_CS      27

static void testGpioPin(uint16_t pin, const char* name, int high_ms) {
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    
    bcm2835_gpio_write(pin, HIGH);
    std::cout << "  " << name << " (GPIO" << pin << ") = HIGH" << std::endl;
    bcm2835_delay(high_ms);
    
    bcm2835_gpio_write(pin, LOW);
    std::cout << "  " << name << " (GPIO" << pin << ") = LOW" << std::endl;
    bcm2835_delay(high_ms);
}

/// Prueba cada pin del display individualmente en secuencia
static void test_pines_display(int repeticiones, int tiempo_ms) {
    std::cout << "\n=== TEST PINES DISPLAY (individual) ===" << std::endl;
    
    int pins[4] = { PIN_BUSY, PIN_DC, PIN_RESET, PIN_CS };
    
    for (int r = 0; r < repeticiones; r++) {
        for (int i = 0; i < 4; i++) {
            bcm2835_gpio_fsel(pins[i], BCM2835_GPIO_FSEL_OUTP);
            bcm2835_gpio_write(pins[i], LOW);
            bcm2835_delay(10);
            
            bcm2835_gpio_write(pins[i], HIGH);
            bcm2835_delay(tiempo_ms);
            
            bcm2835_gpio_write(pins[i], LOW);
            bcm2835_delay(tiempo_ms);
        }
    }
    std::cout << "=== TEST COMPLETADO ===\n" << std::endl;
}

/// Prueba todos los pines del display simultáneamente
static void test_pines_simultaneos(int tiempo_ms) {
    std::cout << "\n=== TEST PINES SIMULTÁNEOS ===" << std::endl;
    
    int pins[4] = { PIN_BUSY, PIN_DC, PIN_RESET, PIN_CS };
    
    // Configurar todos como salida
    for (int i = 0; i < 4; i++) {
        bcm2835_gpio_fsel(pins[i], BCM2835_GPIO_FSEL_OUTP);
    }
    
    // Todos HIGH
    std::cout << "  Todos HIGH..." << std::endl;
    for (int i = 0; i < 4; i++) {
        bcm2835_gpio_write(pins[i], HIGH);
    }
    bcm2835_delay(tiempo_ms);
    
    // Todos LOW
    std::cout << "  Todos LOW" << std::endl;
    for (int i = 0; i < 4; i++) {
        bcm2835_gpio_write(pins[i], LOW);
    }
    bcm2835_delay(tiempo_ms);
    
    std::cout << "=== TEST COMPLETADO ===\n" << std::endl;
}

/// Prueba los pines en carrusel (uno enciende, los otros apagan)
static void test_pines_carrusel(int repeticiones, int tiempo_ms) {
    std::cout << "\n=== TEST PINES CARRUSEL ===" << std::endl;
    
    int pins[4] = { PIN_BUSY, PIN_DC, PIN_RESET, PIN_CS };
    const char* names[4] = { "BUSY", "DC", "RESET", "CS" };
    
    // Configurar todos como salida
    for (int i = 0; i < 4; i++) {
        bcm2835_gpio_fsel(pins[i], BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(pins[i], LOW);
    }
    
    for (int r = 0; r < repeticiones; r++) {
        for (int i = 0; i < 4; i++) {
            bcm2835_gpio_write(pins[i], HIGH);
            std::cout << "  " << names[i] << " ON" << std::endl;
            bcm2835_delay(tiempo_ms);
            bcm2835_gpio_write(pins[i], LOW);
        }
    }
    std::cout << "=== TEST COMPLETADO ===\n" << std::endl;
}

#endif // TEST_PINS

// ============================================================================

int main() {

#if defined(TEST_PINS)
    // === MODO TEST DE PINES GPIO ===
    // Inicializar bcm2835 (solo en modo test)
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }

    std::cout << "=== MODO TEST DE PINES GPIO ===" << std::endl;
    
    // 1. Test individual
    std::cout << "--- Test individual de cada pin ---" << std::endl;
    testGpioPin(PIN_RESET, "RESET", 500);
    testGpioPin(PIN_DC, "DC", 500);
    testGpioPin(PIN_CS, "CS", 500);
    testGpioPin(PIN_BUSY, "BUSY", 500);
    
    // 2. Test secuencial
    test_pines_display(2, 300);
    
    // 3. Test simultáneo
    test_pines_simultaneos(500);
    
    // 4. Test carrusel
    test_pines_carrusel(3, 200);
    
    bcm2835_close();
    std::cout << "Test de pines completado." << std::endl;
    return 0;
#else
    // === MODO NORMAL (E-Paper) ===
    std::cout << "Iniciando E-Paper en Raspberry Pi..." << std::endl;

    // Inicializar bcm2835 (necesario para GPIO y SPI)
    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() falló" << std::endl;
        return 1;
    }

    #ifdef CPU_32_BITS
        auto epaper = std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W);
            std::cout << "  Raspberry Pi CPU 32_BITS detectada " << std::endl;
    #else
        auto epaper = std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_266, EPAPER::boardRaspberryPi);
    #endif

    // Inicializar COG
    std::cout << "Inicializando COG..." << std::endl;
    epaper->COG_initial();

    // Mostrar información de GPIOs
    epaper->printGpios();

    // Global Update - QR directo (sin flash: ambos frames = QR)
    std::cout << "Actualización 1: QR Code (global)" << std::endl;
    epaper->globalUpdate(BW_QrBuffer, BW_QrBuffer);
    TYME::delay(900);

    // Fast Update - Limpiar pantalla
    std::cout << "Actualización 2: Limpiando (fast)" << std::endl;
    epaper->fastUpdate(BW_QrBuffer, BW_0x00Buffer);

    // Fast Update - Mostrar imagen Mono
    std::cout << "Actualización 3: Imagen Mono (fast)" << std::endl;
    epaper->fastUpdate(BW_0x00Buffer, BW_monoBuffer);
    TYME::delay(900);

    // Fast Update - Limpiar pantalla
    std::cout << "Actualización 4: Limpiando (fast)" << std::endl;
    epaper->fastUpdate(BW_monoBuffer, BW_0x00Buffer);

    TYME::delay(10);

    // Fast Update - Mostrar imagen BWR
    std::cout << "Actualización 5: Imagen BWR (fast)" << std::endl;
    epaper->fastUpdate(BW_0x00Buffer, BWR_blackBuffer);

    std::cout << "Apagando COG..." << std::endl;
    epaper->COG_powerOff();

    // Liberar driver (destruye Spi_t → llama a bcm2835_spi_end())
    epaper.reset();

    // Cerrar bcm2835
    bcm2835_close();

    std::cout << "Programa finalizado correctamente" << std::endl;

    return 0;
#endif

}
