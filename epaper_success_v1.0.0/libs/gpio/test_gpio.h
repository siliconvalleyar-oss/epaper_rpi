//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   test_gpio.h
//          License             :   GNU 
//          Author              :   Lio
//          Description         :   GPIO test helper functions
//          brief               :   Move GPIO tests out of main.cpp
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <gpio/gpio.h>

namespace GPIO_TEST {

/// @brief Test a single GPIO pin: export, set high, wait, set low, unexport
/// @param gpio     Reference to GPIO controller
/// @param pin      GPIO number (sysfs format, e.g. 534-539)
/// @param name     Human-readable pin name
/// @param high_ms  Milliseconds to keep pin high
void testPin(GPIO::Gpio_t& gpio, uint16_t pin, const char* name, int high_ms);

/// @brief Test each display pin sequentially (RST, DC, CS, BUSY, flashCS)
/// @param gpio          Reference to GPIO controller
/// @param repeticiones  How many times to cycle through pins
/// @param tiempo_ms     Delay between toggles (ms)
void testDisplayPins(GPIO::Gpio_t& gpio, int repeticiones, int tiempo_ms);

/// @brief Set all display pins high simultaneously, wait, then set low
/// @param gpio       Reference to GPIO controller
/// @param tiempo_ms  How long to keep pins high (ms)
void testSimultaneous(GPIO::Gpio_t& gpio, int tiempo_ms);

/// @brief Carousel: light up each pin one-by-one in sequence
/// @param gpio          Reference to GPIO controller
/// @param repeticiones  How many times to loop through all pins
/// @param tiempo_ms     Delay between pins (ms)
void testCarousel(GPIO::Gpio_t& gpio, int repeticiones, int tiempo_ms);

} // namespace GPIO_TEST
