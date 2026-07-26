//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   epaper.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#include "epaper.h"
#include <unistd.h>
#include <cstring>

//#define DEBUG

namespace EPAPER {

// Función de delay en milisegundos
static void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

// Definición de registros
const uint8_t register_data_mid[] = { 0x00, 0x0e, 0x19, 0x02, 0x0f, 0x89 };
const uint8_t register_data_sm[] = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };

// Implementación de Spi_t
Spi_t::Spi_t() {
    // Inicializar hardware SPI (configura pines GPIO 9-11 como ALT0)
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // ~976 KHz (igual que referencia)
    
    // Deshabilitar CS hardware, usamos CS manual por GPIO
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
}

Spi_t::~Spi_t() {
    // Restaurar pines SPI (GPIO 9-11) a GPIO input
    bcm2835_spi_end();
}

uint8_t Spi_t::Transfer1bytes(uint8_t data) {
    return bcm2835_spi_transfer(data);
}

// Implementación de Gpio_t
Gpio_t::Gpio_t(bool enable) : enabled(enable) {}

void Gpio_t::pinMode(uint16_t pin, uint8_t mode) {
    if (pin != NOT_CONNECTED) {
        bcm2835_gpio_fsel(pin, mode);
    }
}

void Gpio_t::digitalWrite(uint16_t pin, uint8_t value) {
    if (pin != NOT_CONNECTED) {
        bcm2835_gpio_write(pin, value);
    }
}

int Gpio_t::digitalRead(uint16_t pin) {
    if (pin != NOT_CONNECTED) {
        return bcm2835_gpio_lev(pin);
    }
    return 0;
}

// Implementación de EPD_Driver
EPD_Driver::EPD_Driver(uint32_t eScreen_EPD, const pins_t& board)
    : Gpio_t(true)
    , spi_ptr(std::make_unique<Spi_t>())
    , pin_cfg_epaper(board)
{
    // Tipo de pantalla
    pdi_size = (uint16_t)(eScreen_EPD >> 8);

    uint16_t screenSizeV = 0;
    uint16_t screenSizeH = 0;
    
    switch (pdi_size) {
        case 0x15: // 1.54"
            screenSizeV = 152;
            screenSizeH = 152;
            break;

        case 0x21: // 2.13"
            screenSizeV = 212;
            screenSizeH = 104;
            break;

        case 0x26: // 2.66"
            screenSizeV = 296;
            screenSizeH = 152;
            break;

        case 0x27: // 2.71"
            screenSizeV = 264;
            screenSizeH = 176;
            break;

        case 0x28: // 2.87"
            screenSizeV = 296;
            screenSizeH = 128;
            break;

        case 0x37: // 3.70"
            screenSizeV = 416;
            screenSizeH = 240;
            break;

        case 0x41: // 4.17"
            screenSizeV = 300;
            screenSizeH = 400;
            break;

        case 0x43: // 4.37"
            screenSizeV = 480;
            screenSizeH = 176;
            break;

        default:
            break;
    }

    // Calcular tamaño de datos de imagen
    image_data_size = (uint32_t)screenSizeV * (uint32_t)(screenSizeH / 8);

    // Configurar registros según tamaño de pantalla
    memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}

int EPD_Driver::digitalRead(int gpio) {
    return Gpio_t::digitalRead(gpio);
}

uint8_t EPD_Driver::hV_HAL_SPI_transfer(uint8_t data) {
    return spi_ptr->Transfer1bytes(data);
}

void EPD_Driver::COG_initial() {
    // Configurar pines
    pinMode(pin_cfg_epaper.panelBusy, INPUT);
    pinMode(pin_cfg_epaper.panelDC, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);
    
    pinMode(pin_cfg_epaper.panelReset, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    
    pinMode(pin_cfg_epaper.panelCS, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    
    if (pin_cfg_epaper.panelON_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelON_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelON_EXT2, HIGH);
    }
    
    if (pin_cfg_epaper.panelSPI43_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelSPI43_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelSPI43_EXT2, LOW);
    }
    
    // Delay inicial
    delay_ms(5);
    
    // Secuencia de reset
    reset(5, 5, 10, 5, 5);
    
    // Soft reset
    softReset();
    
    // Configuración de temperatura
    sendIndexData(0xe5, &register_data[2], 1);  // Input Temperature:  register_data[2] = 0x19 (25°C)
    sendIndexData(0xe0, &register_data[3], 1);  // Active Temperature: register_data[3] = 0x02 (coincide con referencia)
    sendIndexData(0x00, &register_data[4], 2);  // PSR:              register_data[4..5] = 0xcf, 0x8d (coincide con referencia)
}

void EPD_Driver::sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    // Enviar comando: DC=LOW, CS=LOW → transferir byte → CS=HIGH
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(index);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);

    // Enviar datos: DC=HIGH, CS=LOW → transferir cada byte → CS=HIGH entre cada uno
    for (uint32_t i = 0; i < len; i++) {
        digitalWrite(pin_cfg_epaper.panelDC, HIGH);
        digitalWrite(pin_cfg_epaper.panelCS, LOW);
        hV_HAL_SPI_transfer(data[i]);
        digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    }
}

void EPD_Driver::sendCommand8(uint8_t command) {
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(command);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
}

void EPD_Driver::sendCommandData8(uint8_t command, uint8_t data) {
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(command);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);

    digitalWrite(pin_cfg_epaper.panelDC, HIGH);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(data);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
}

void EPD_Driver::softReset() {
    sendIndexData(0x00, &register_data[1], 1);  // Soft-reset
    
    uint32_t timeout = 5000;  // 5 segundos de timeout
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH && timeout > 0) {
        delay_ms(1);
        timeout--;
    }
    
    if (timeout == 0) {
        std::cerr << "ERROR: softReset() timeouteó - BUSY nunca llegó a HIGH" << std::endl;
    }
}

void EPD_Driver::reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5) {
    delay_ms(ms1);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    delay_ms(ms2);
    digitalWrite(pin_cfg_epaper.panelReset, LOW);
    delay_ms(ms3);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    delay_ms(ms4);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    delay_ms(ms5);
}

void EPD_Driver::DCDC_powerOn() {
    uint8_t dummy = 0x00;
    sendIndexData(0x04, &dummy, 0);  // Power on — solo comando, sin dato extra
    
    uint32_t timeout = 5000;  // 5 segundos de timeout
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH && timeout > 0) {
        delay_ms(1);
        timeout--;
    }
    
    if (timeout == 0) {
        std::cerr << "ERROR: DCDC_powerOn() timeouteó - BUSY nunca llegó a HIGH" << std::endl;
    }
}

void EPD_Driver::displayRefresh() {
    uint8_t dummy = 0x00;
    sendIndexData(0x12, &dummy, 0);  // Display Refresh — solo comando
    
    uint32_t timeout = 20000;  // 20 segundos para refresh completo
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH && timeout > 0) {
        delay_ms(1);
        timeout--;
    }
    
    if (timeout == 0) {
        std::cerr << "ERROR: displayRefresh() timeouteó - BUSY nunca llegó a HIGH" << std::endl;
    }
}

void EPD_Driver::globalUpdate(const uint8_t *data1s, const uint8_t *data2s) {
    // Enviar primer frame (0x10 = canal negro/BW)
    sendIndexData(0x10, data1s, image_data_size);

    // Enviar segundo frame (0x13 = canal rojo/segundo plano)
    sendIndexData(0x13, data2s, image_data_size);

    // Encender DC/DC y refrescar
    DCDC_powerOn();
    displayRefresh();
}

void EPD_Driver::fastUpdate(const uint8_t *oldData, const uint8_t *newData) {
    bool hasChanges = false;

    for (uint32_t i = 0; i < image_data_size; i++) {
        if (oldData[i] != newData[i]) {
            hasChanges = true;
            break;
        }
    }

    if (!hasChanges) {
        return;
    }

    softReset();

    uint8_t tempFast = register_data[2] | 0x40;
    sendCommandData8(0xE5, tempFast);

    sendCommandData8(0xE0, register_data[3]);

    uint8_t psrFast[2] = { static_cast<uint8_t>(register_data[4] | 0x10), static_cast<uint8_t>(register_data[5] | 0x02) };
    sendIndexData(0x00, psrFast, 2);

    sendCommandData8(0x50, 0x07);

    // Fast update: 0x10=OLD image, 0x13=NEW image (swapped)
    sendIndexData(0x10, oldData, image_data_size);
    sendIndexData(0x13, newData, image_data_size);

    DCDC_powerOn();
    displayRefresh();
}

void EPD_Driver::COG_powerOff() {
    sendIndexData(0x02, &register_data[0], 0);  // Turn off DC/DC
    
    uint32_t timeout = 5000;  // 5 segundos de timeout
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH && timeout > 0) {
        delay_ms(1);
        timeout--;
    }
    
    if (timeout == 0) {
        std::cerr << "ERROR: COG_powerOff() timeouteó - BUSY nunca llegó a HIGH" << std::endl;
    }
    
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    
    delay_ms(150);
    
    digitalWrite(pin_cfg_epaper.panelReset, LOW);
}

void EPD_Driver::printGpios() {
    std::cout << "========================================" << std::endl;
    std::cout << "Configuración de GPIOs para E-Paper:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Panel BUSY  : GPIO" << pin_cfg_epaper.panelBusy << std::endl;
    std::cout << "Panel DC    : GPIO" << pin_cfg_epaper.panelDC << std::endl;
    std::cout << "Panel RESET : GPIO" << pin_cfg_epaper.panelReset << std::endl;
    std::cout << "Panel CS    : GPIO" << pin_cfg_epaper.panelCS << std::endl;
    std::cout << "Panel ON_EXT2: ";
    if (pin_cfg_epaper.panelON_EXT2 == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.panelON_EXT2 << std::endl;
    }
    std::cout << "Panel SPI43 : ";
    if (pin_cfg_epaper.panelSPI43_EXT2 == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.panelSPI43_EXT2 << std::endl;
    }
    std::cout << "Flash CS    : ";
    if (pin_cfg_epaper.flashCS == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.flashCS << std::endl;
    }
    std::cout << "========================================" << std::endl;
    std::cout << "SPI Configuración:" << std::endl;
    std::cout << "  - Clock: SCLK (GPIO11)" << std::endl;
    std::cout << "  - MOSI: GPIO10" << std::endl;
    std::cout << "  - MISO: GPIO9 (no usado)" << std::endl;
    std::cout << "========================================" << std::endl;
}

} // namespace EPAPER
