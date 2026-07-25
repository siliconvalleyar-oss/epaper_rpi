//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   epaper.h
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <memory>
#include <cstdint>
#include <cstring>

// Incluir bcm2835 primero
#include <bcm2835.h>

// bcm2835.h define: #define delay(x) bcm2835_delay(x)
// Esto rompe TYME::delay(), TYME::delay_us(), etc.
#undef delay
#undef delayMicroseconds

// No redefinir HIGH y LOW, usar los de bcm2835 directamente
// Solo definir si no existen
#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW 0x0
#endif

//#define DBG_EPAPER

namespace EPAPER {

// Definiciones para compatibilidad - usar valores de bcm2835
#define INPUT BCM2835_GPIO_FSEL_INPT
#define OUTPUT BCM2835_GPIO_FSEL_OUTP
#define NOT_CONNECTED 0xff

// Tipos de pantalla E-Paper
#define eScreen_EPD_154 (uint32_t)0x1509
#define eScreen_EPD_213 (uint32_t)0x2100
#define eScreen_EPD_266 (uint32_t)0x2600
#define eScreen_EPD_271 (uint32_t)0x2700
#define eScreen_EPD_287 (uint32_t)0x2800
#define eScreen_EPD_370 (uint32_t)0x3700
#define eScreen_EPD_417 (uint32_t)0x4100
#define eScreen_EPD_437 (uint32_t)0x430C

// Registros para diferentes tamaños de pantalla
[[maybe_unused]] extern const uint8_t register_data_mid[];
[[maybe_unused]] extern const uint8_t register_data_sm[];

struct pins_t {
    uint16_t panelBusy;      ///< GPIO para BUSY
    uint16_t panelDC;        ///< GPIO para Data/Command
    uint16_t panelReset;     ///< GPIO para RESET
    uint16_t panelCS;        ///< GPIO para Chip Select
    uint16_t panelON_EXT2;   ///< GPIO adicional
    uint16_t panelSPI43_EXT2;///< GPIO para BS
    uint16_t flashCS;        ///< GPIO para Flash CS
};

// Clase SPI para bcm2835
class Spi_t {
public:
    Spi_t();
    ~Spi_t();
    uint8_t Transfer1bytes(uint8_t data);
};

// Clase GPIO para bcm2835
class Gpio_t {
public:
    Gpio_t(bool enable);
    virtual ~Gpio_t() = default;
    
    void pinMode(uint16_t pin, uint8_t mode);
    void digitalWrite(uint16_t pin, uint8_t value);
    int digitalRead(uint16_t pin);
    
protected:
    bool enabled;
};

// Clase principal del driver E-Paper
struct EPD_Driver : public Gpio_t {
public:
    explicit EPD_Driver(uint32_t eScreen_EPD, const pins_t& board);
    ~EPD_Driver() = default;
    
    // Funciones principales
    uint8_t hV_HAL_SPI_transfer(uint8_t data);
    void COG_initial();
    void COG_powerOff();
    void globalUpdate(const uint8_t *data1s, const uint8_t *data2s);
    void fastUpdate(const uint8_t *oldData, const uint8_t *newData);
    
    // Funciones auxiliares
    void printGpios();

protected:
    void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len);
    void sendCommand8(uint8_t command);
    void sendCommandData8(uint8_t command, uint8_t data);
    void softReset();
    void displayRefresh();
    void reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5);
    void DCDC_powerOn();
    int digitalRead(int gpio);
    
    // Variables internas
    uint16_t pdi_size;
    uint32_t image_data_size;
    uint8_t register_data[6] = {};

private:
    std::unique_ptr<Spi_t> spi_ptr;
    const pins_t pin_cfg_epaper;
};

} // namespace EPAPER
