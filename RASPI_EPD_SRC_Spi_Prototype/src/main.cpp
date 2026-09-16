#include <iostream>
#include <memory>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#define SCREEN 266
#include <graphics/userImageData.h>
#include <tyme/tyme.h>
#include <app/config.h>

// Uncomment to run GPIO tests instead of the display
//#define TEST_GPIO

#ifdef TEST_GPIO
#include <gpio/test_gpio.h>
#endif

int main() {

#ifdef TEST_GPIO

    // ============================================================
    // GPIO Test Mode — verifies pin wiring before display operation
    // ============================================================
    bool gpio_ok = true;
    GPIO::Gpio_t gpio(gpio_ok);

    std::cout << "\n========================================" << std::endl;
    std::cout << "   GPIO TEST MODE" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 1. Test each display pin individually
    GPIO_TEST::testDisplayPins(gpio, 2, 400);

    TYME::delay(500);

    // 2. Carousel test — pins light up one by one
    GPIO_TEST::testCarousel(gpio, 2, 250);

    TYME::delay(500);

    // 3. Simultaneous test — all pins together
    GPIO_TEST::testSimultaneous(gpio, 800);

    std::cout << "\n========================================" << std::endl;
    std::cout << "   GPIO TEST COMPLETE" << std::endl;
    std::cout << "========================================\n" << std::endl;

#else

    // ============================================================
    // E-Paper Display Mode
    // ============================================================
    auto epaper {std::make_unique<EPAPER::EPD_Driver>(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W)};

    epaper->COG_initial();
    epaper->printGpios();

    // Sequence: QR code → full clear → mono image → BWR image
    epaper->globalUpdate(BW_QrBuffer, BW_0x00Buffer);
    TYME::delay(900);

    epaper->globalUpdate(BW_0x00Buffer, BW_0x00Buffer);
    epaper->globalUpdate(BW_monoBuffer, BW_0x00Buffer);
    TYME::delay(900);

    epaper->globalUpdate(BW_0x00Buffer, BW_0x00Buffer);

    TYME::delay(10);
    epaper->globalUpdate(BWR_blackBuffer, BWR_redBuffer);

    epaper->COG_powerOff();

#endif

    return 0;
}
