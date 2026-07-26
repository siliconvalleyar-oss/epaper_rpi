#pragma once
#include <epaper/epaper.h>
#include <string>

namespace EPAPER {

// Configuración para Raspberry Pi 3B/4B/5 (BCM numbering)
// Cableado según documentación:
//   Rojo    -> BUSY  (GPIO25 - Pin 22)
//   Naranja -> DC    (GPIO24 - Pin 18)
//   Amarillo-> RESET (GPIO23 - Pin 16)
//   Gris    -> CS    (GPIO27 - Pin 13)
//   Violeta -> flashCS (GPIO22 - Pin 15)
const pins_t boardRaspberryPi = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Rojo
    .panelDC          = 24,    // GPIO24 - Pin 18 - Naranja
    .panelReset       = 23,    // GPIO23 - Pin 16 - Amarillo
    .panelCS          = 27,    // GPIO27 - Pin 13 - Gris
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Violeta
};

// Configuración para Raspberry Pi Zero 2W
const pins_t boardRaspberryPiZero2W = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Estado ocupado Rojo
    .panelDC          = 24,    // GPIO24 - Pin 18 - Data/Command Naranja
    .panelReset       = 23,    // GPIO23 - Pin 16 - Reset -Amarillo
    .panelCS          = 27,     // GPIO27  - Pin13  - Chip Select (CE0) Gris
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Flash CS
};

/*
| Señal | GPIO | Pin RPi |
Panel_CS | GPIO27 | Pin 13 |  Gris
 MOSI | GPIO10 | Pin 19 |      Azul
 SCLK | GPIO11 | Pin 23 |      Marron
 MISO | GPIO9 | Pin 21 |       Verde

 Flash_CS | GPIO22 | Pin 15 |  Violeta

 RESET | GPIO23 | Pin 16 |     Amarillo
 D/C | GPIO24 | Pin18  |        Naranja
 BUSY | GPI25 | Pin 22 |       Rojo
*/

// Configuración para Raspberry Pi 4/5
const pins_t boardRaspberryPi4 = {
    .panelBusy        = 27,    // GPIO27 - Pin 13
    .panelDC          = 18,    // GPIO18 - Pin 12
    .panelReset       = 17,    // GPIO17 - Pin 11
    .panelCS          = 8,     // GPIO8  - Pin 24
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15
};

// Función auxiliar para obtener configuración por modelo
inline const pins_t& getBoardConfig(const std::string& model = "RaspberryPi") {
    if (model == "RaspberryPiZero2W") {
        return boardRaspberryPiZero2W;
    } else if (model == "RaspberryPi4" || model == "RaspberryPi5") {
        return boardRaspberryPi4;
    }
    return boardRaspberryPi;
}

} // namespace EPAPER
