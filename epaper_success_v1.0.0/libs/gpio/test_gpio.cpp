//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   test_gpio.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Description         :   GPIO test helper implementations
//
//////////////////////////////////////////////////////////////////////////////

#include "test_gpio.h"
#include <epaper/boards.h>
#include <tyme/tyme.h>
#include <iostream>
#include <string>
#include <vector>

namespace GPIO_TEST {

// Struct to hold display pin info for tests
struct PinInfo {
    uint16_t number;    // GPIO number (sysfs)
    std::string name;   // Human-readable name
};

// Get the display pins from boardRaspberryPi config
static std::vector<PinInfo> getDisplayPins() {
    return {
        {EPAPER::boardRaspberryPi.panelReset,  "RST"},
        {EPAPER::boardRaspberryPi.panelDC,     "DC"},
        {EPAPER::boardRaspberryPi.panelCS,     "CS"},
        {EPAPER::boardRaspberryPi.panelBusy,   "BUSY"},
        {EPAPER::boardRaspberryPi.flashCS,     "FLASH_CS"}
    };
}

void testPin(GPIO::Gpio_t& gpio, uint16_t pin, const char* name, int high_ms) {
    std::cout << "[TEST] Pin " << name << " (GPIO " << pin << "): ";
    
    // Export
    if (gpio.gpio_export(pin) != 0) {
        std::cout << "FAIL - cannot export" << std::endl;
        return;
    }
    
    // Set as output
    gpio.pinMode(pin, GPIO::DIR_OUT);
    
    // Set high
    gpio.digitalWrite(pin, GPIO::VALUE_HIGH);
    std::cout << "HIGH ";
    TYME::delay(high_ms);
    
    // Set low
    gpio.digitalWrite(pin, GPIO::VALUE_LOW);
    std::cout << "LOW";
    
    // Unexport
    gpio.gpio_unexport(pin);
    
    std::cout << " OK" << std::endl;
}

void testDisplayPins(GPIO::Gpio_t& gpio, int repeticiones, int tiempo_ms) {
    std::cout << "\n=== TEST: Display Pins Sequential (" << repeticiones << "x, " << tiempo_ms << "ms) ===" << std::endl;
    
    auto pins = getDisplayPins();
    
    for (int r = 0; r < repeticiones; ++r) {
        std::cout << "--- Round " << (r + 1) << " ---" << std::endl;
        
        for (const auto& pin : pins) {
            // Export & set direction
            if (gpio.gpio_export(pin.number) == 0) {
                gpio.pinMode(pin.number, GPIO::DIR_OUT);
                gpio.digitalWrite(pin.number, GPIO::VALUE_HIGH);
                TYME::delay(tiempo_ms);
                gpio.digitalWrite(pin.number, GPIO::VALUE_LOW);
                gpio.gpio_unexport(pin.number);
            }
            std::cout << "  " << pin.name << " OK" << std::endl;
        }
    }
    
    std::cout << "=== Sequential Test Complete ===\n" << std::endl;
}

void testSimultaneous(GPIO::Gpio_t& gpio, int tiempo_ms) {
    std::cout << "\n=== TEST: Display Pins Simultaneous (" << tiempo_ms << "ms) ===" << std::endl;
    
    auto pins = getDisplayPins();
    
    // Export all and set high
    for (const auto& pin : pins) {
        if (gpio.gpio_export(pin.number) == 0) {
            gpio.pinMode(pin.number, GPIO::DIR_OUT);
            gpio.digitalWrite(pin.number, GPIO::VALUE_HIGH);
            std::cout << "  " << pin.name << " HIGH" << std::endl;
        }
    }
    
    TYME::delay(tiempo_ms);
    
    // Set all low
    for (const auto& pin : pins) {
        gpio.digitalWrite(pin.number, GPIO::VALUE_LOW);
        gpio.gpio_unexport(pin.number);
        std::cout << "  " << pin.name << " LOW" << std::endl;
    }
    
    std::cout << "=== Simultaneous Test Complete ===\n" << std::endl;
}

void testCarousel(GPIO::Gpio_t& gpio, int repeticiones, int tiempo_ms) {
    std::cout << "\n=== TEST: Carousel (" << repeticiones << "x, " << tiempo_ms << "ms) ===" << std::endl;
    
    auto pins = getDisplayPins();
    
    for (int r = 0; r < repeticiones; ++r) {
        std::cout << "--- Round " << (r + 1) << " ---" << std::endl;
        
        // Light up each pin in sequence
        for (size_t i = 0; i < pins.size(); ++i) {
            // Export & set current pin high
            if (gpio.gpio_export(pins[i].number) == 0) {
                gpio.pinMode(pins[i].number, GPIO::DIR_OUT);
                gpio.digitalWrite(pins[i].number, GPIO::VALUE_HIGH);
            }
            
            TYME::delay(tiempo_ms);
            
            // Set current pin low & unexport
            gpio.digitalWrite(pins[i].number, GPIO::VALUE_LOW);
            gpio.gpio_unexport(pins[i].number);
            
            std::cout << "  -> " << pins[i].name << std::endl;
        }
    }
    
    std::cout << "=== Carousel Test Complete ===\n" << std::endl;
}

} // namespace GPIO_TEST
