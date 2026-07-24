#pragma once
#include <epaper/epaper.h>

namespace EPAPER {

// ============================================================================
// Configuraciones de pines para Raspberry Pi
//
// Cableado (colores de cables):
//   Rojo    -> BUSY  (GPIO25 - Pin 22)
//   Naranja -> DC    (GPIO24 - Pin 18)
//   Amarillo-> RESET (GPIO23 - Pin 16)
//   Gris    -> CS    (GPIO27 - Pin 13)
//   Violeta -> flashCS (GPIO22 - Pin 15)
//   Azul    -> MOSI  (GPIO10 - Pin 19)
//   Marrón  -> SCLK  (GPIO11 - Pin 23)
//   Verde   -> MISO  (GPIO9  - Pin 21, no usado)
// ============================================================================

// Configuración para Raspberry Pi (BCM numbering estándar)
const pins_t boardRaspberryPi = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Rojo
    .panelDC          = 24,    // GPIO24 - Pin 18 - Naranja
    .panelReset       = 23,    // GPIO23 - Pin 16 - Amarillo
    .panelCS          = 27,    // GPIO27 - Pin 13 - Gris
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Violeta
};

// Configuración para Raspberry Pi Zero 2W (mismos pines que boardRaspberryPi)
const pins_t boardRaspberryPiZero2W = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Rojo
    .panelDC          = 24,    // GPIO24 - Pin 18 - Naranja
    .panelReset       = 23,    // GPIO23 - Pin 16 - Amarillo
    .panelCS          = 27,    // GPIO27 - Pin 13 - Gris
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Violeta
};

// Configuración alternativa para Raspberry Pi 4/5 (cableado diferente)
const pins_t boardRaspberryPi4 = {
    .panelBusy        = 27,    // GPIO27 - Pin 13
    .panelDC          = 18,    // GPIO18 - Pin 12
    .panelReset       = 17,    // GPIO17 - Pin 11
    .panelCS          = 8,     // GPIO8  - Pin 24 (CE0 nativo)
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15
};

} // namespace EPAPER