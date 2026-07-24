#ifndef EPD_CONFIGURATION_H
#define EPD_CONFIGURATION_H

#include <cstdint>

#define eScreen_EPD_t uint32_t
#define eScreen_EPD_154 (uint32_t)0x1509
#define eScreen_EPD_213 (uint32_t)0x2100
#define eScreen_EPD_266 (uint32_t)0x2600
#define eScreen_EPD_271 (uint32_t)0x2700
#define eScreen_EPD_287 (uint32_t)0x2800
#define eScreen_EPD_370 (uint32_t)0x3700
#define eScreen_EPD_417 (uint32_t)0x4100
#define eScreen_EPD_437 (uint32_t)0x430C

#define frameSize_EPD_EXT3_154 (uint32_t)(2888)
#define frameSize_EPD_EXT3_213 (uint32_t)(2756)
#define frameSize_EPD_EXT3_266 (uint32_t)(5624)
#define frameSize_EPD_EXT3_271 (uint32_t)(5808)
#define frameSize_EPD_EXT3_287 (uint32_t)(4736)
#define frameSize_EPD_EXT3_370 (uint32_t)(12480)
#define frameSize_EPD_EXT3_417 (uint32_t)(15000)
#define frameSize_EPD_EXT3_437 (uint32_t)(10560)

#define NOT_CONNECTED (uint8_t)0xff

struct pins_t
{
    uint8_t panelBusy;
    uint8_t panelDC;
    uint8_t panelReset;
    uint8_t panelCS;
    uint8_t panelON_EXT2;
    uint8_t panelSPI43_EXT2;
    uint8_t flashCS;
};

///
/// @brief Raspberry Pi Zero 2W EXT3 configuration
///
const pins_t boardRaspberryPiZero2W_EXT3 =
{
    .panelBusy = 25,       ///< GPIO25 -> Pin 22 -> Red wire
    .panelDC = 24,         ///< GPIO24 -> Pin 18 -> Orange wire
    .panelReset = 23,      ///< GPIO23 -> Pin 16 -> Yellow wire
    .panelCS = 27,         ///< GPIO27 -> Pin 13 -> Grey wire
    .panelON_EXT2 = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS = 22          ///< GPIO22 -> Pin 15 -> Violet wire
};

const uint8_t register_data_mid[] = { 0x00, 0x0e, 0x19, 0x02, 0x0f, 0x89 };
const uint8_t register_data_sm[] = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };

#endif
