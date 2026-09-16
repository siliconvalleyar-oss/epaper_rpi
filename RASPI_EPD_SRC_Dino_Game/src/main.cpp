//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   main.cpp
//          Description         :   E-Paper Dino Jump Game
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66" (296x152)
//          Complier            :   g++
//          Dependencies        :   bcm2835
//     
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include <cstring>
#include <csignal>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <tyme/tyme.h>
#include <app/config.h>
#include <game/dino_game.h>

#define SCREEN 266
#define BUTTON_PIN RPI_V2_GPIO_P1_07  // GPIO4 - button for jump

static volatile sig_atomic_t running = 1;

static void signalHandler(int) {
    running = 0;
}

// Button state
static volatile bool buttonPressed = false;

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "=== E-PAPER DINO GAME ===" << std::endl;
    std::cout << "Pantalla: 2.66\" (296x152)" << std::endl;
    std::cout << "Presiona Ctrl+C para salir\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() fallo" << std::endl;
        return 1;
    }

    // Setup button pin
    bcm2835_gpio_fsel(BUTTON_PIN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(BUTTON_PIN, BCM2835_GPIO_PUD_UP);

    EPAPER::EPD_Driver epaper(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W);

    std::cout << "Inicializando COG..." << std::endl;
    epaper.COG_initial();
    std::cout << "COG listo.\n" << std::endl;

    // Create game
    DinoGame game;
    
    // Buffers
    uint8_t buffer[(SCREEN_W * SCREEN_H) / 8];
    uint8_t prevBuffer[(SCREEN_W * SCREEN_H) / 8];
    memset(buffer, 0x00, sizeof(buffer));
    memset(prevBuffer, 0x00, sizeof(prevBuffer));

    // Splash screen
    std::cout << "Mostrando presentacion..." << std::endl;
    game.renderSplash(buffer);
    memcpy(prevBuffer, buffer, sizeof(buffer));
    epaper.globalUpdate(buffer, buffer);
    TYME::delay(2000);

    std::cout << "Juego iniciado! Modo demo: dino salta solo.\n" << std::endl;
    
    int frameCount = 0;

    // Main game loop — solo fastUpdate (sin globalUpdate periódico para evitar flash)
    while (running) {
        game.autoJump();
        game.update();

        if (frameCount % 3 == 0) {
            game.render(buffer);

            if (!epaper.isBusy()) {
                epaper.fastUpdate(prevBuffer, buffer);
                memcpy(prevBuffer, buffer, sizeof(buffer));
            }
        }

        frameCount++;
        TYME::delay(33);
    }

    std::cout << "\n\nApagando COG..." << std::endl;
    epaper.COG_powerOff();
    bcm2835_close();

    std::cout << "Programa finalizado." << std::endl;
    return 0;
}
