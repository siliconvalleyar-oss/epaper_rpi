# Skill: epaper

E-Paper display driver for Raspberry Pi (Pervasive Displays EPD over SPI).

## Project Structure

```
epaper/
├── libs/
│   ├── app/config.h           # CPU arch detection (32 vs 64-bit)
│   ├── epaper/
│   │   ├── epaper.h           # EPD_Driver low-level SPI
│   │   ├── epaper.cpp
│   │   ├── epaper_display.h   # EpaperDisplay (high-level drawing)
│   │   ├── epaper_display.cpp
│   │   └── boards.h           # Pin configs per RPi model
│   ├── fonts/
│   │   ├── fonts.h/cpp        # Bitmap font data (7 fonts)
│   │   ├── fonts_manager.h/cpp # Font selection & rendering
│   ├── gpio/gpio.h/cpp        # GPIO sysfs wrapper
│   ├── graphics/              # Pre-rendered images per screen size
│   ├── qr/qr_gen.h/cpp        # QR code generation
│   ├── spi/spi.h/cpp          # SPI sysfs wrapper
│   ├── tyme/tyme.h/cpp        # Delay utilities
│   └── work/work.h            # Work queue macros
├── src/main.cpp               # Demo entry point
├── scripts_tools/             # GPIO/SPI configuration scripts
├── rules_debug/               # GDB debug rules
├── Makefile                   # g++ -std=c++20, links bcm2835 + qrencode
└── README.md
```

## Build & Run

```bash
make            # build (produces bin/epaper_app)
make run        # build + run with sudo
make clean      # clean obj/ and bin/
```

Dependencies: `libbcm2835-dev`, `libqrencode-dev`.

## Hardware

- Raspberry Pi Zero 2W or Pi 4
- Pervasive Displays EPD (2.13" 212x104, 2.66" 296x152, 1.54" 200x200)
- SPI interface: MOSI=GPIO10, SCLK=GPIO11, MISO=GPIO9
- Panel_CS=GPIO27, Flash_CS=GPIO22, RESET=GPIO25, D/C=GPIO8, BUSY=GPIO7

## Key Patterns

### drawPixel coordinate system
```cpp
// Horizontal mirror + column-major layout (required by EPD hardware)
int mirroredX = m_width - 1 - x;
int byteIndex = (mirroredX * m_height + y) / 8;
int bitIndex = 7 - (y % 8);
```

### Adding a new font
1. Add bitmap data array in `fonts.cpp`
2. Add extern declaration in `fonts.h`
3. Add FontType enum entry in `fonts_manager.h`
4. Add case in `FontManager::setFont()`

### Adding a new screen size
1. Add `eScreen_EPD_xxx` to board enum
2. Add width/height in `EpaperDisplay` constructor
3. Add pin config in `boards.h`
