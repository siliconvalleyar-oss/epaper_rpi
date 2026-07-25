# E-Paper Display Driver for Raspberry Pi

Controlador para pantallas e-Paper Pervasive Displays via SPI + GPIO con bcm2835.
Soporta Raspberry Pi Zero 2W, Pi 3B/4B/5.

## Hardware

Pantallas soportadas: 2.66" (296x152), 2.13" (212x104), 1.54" (200x200).

Ver [docs/WIRING.md](docs/WIRING.md) para el diagrama completo de conexiones.

## Compilacion

```bash
# Dependencias
sudo apt-get install libqrencode-dev
# bcm2835: http://www.airspayce.com/mikem/bcm2835/

make            # Compilar
make run        # Compilar y ejecutar (sudo)
make clean      # Limpiar
```

Ver [docs/BUILD.md](docs/BUILD.md) para compilacion remota y verificacion de dependencias.

## Uso

```cpp
#include <epaper/epaper_display.h>
#include <epaper/boards.h>

bcm2835_init();

{
    EPAPER_DISPLAY::EpaperDisplay display(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W);
    display.init();

    display.clearScreen(true);
    display.drawString(10, 10, "Hola Mundo", FONT_7x8_THICK, true);
    display.drawCenteredString(50, "EPAPER", FONT_16x32_BIGNUM, true);
    display.update();

    TYME::delay(5000);
}

bcm2835_close();
```

Ver [docs/API.md](docs/API.md) para la referencia completa de la API de dibujo.

## Fuentes

9 fuentes bitmap: 5x8, 7x8 Thick, 7x8 Homespun, 3x8 Tiny, 4x8 Seven-Seg, 8x8, 8x8 Wide, 16x32 Big Numbers, 16x16 Medium Numbers.

Ver [docs/FONTS.md](docs/FONTS.md) para el formato de bitmap y como agregar fuentes.

## Documentacion

| Documento | Contenido |
|-----------|-----------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Capas, componentes, flujo de ejecucion |
| [docs/WIRING.md](docs/WIRING.md) | Conexiones GPIO, SPI, colores de cable |
| [docs/BUILD.md](docs/BUILD.md) | Dependencias, compilacion, remoto |
| [docs/API.md](docs/API.md) | API de dibujo, buffer, ciclo de vida |
| [docs/FONTS.md](docs/FONTS.md) | Sistema de fuentes, formato bitmap |
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | Protocolo SPI, comandos EPD |

## Estructura

```
Master/
  src/main.cpp              -- Aplicacion demo
  libs/
    epaper/
      epaper.h/cpp          -- Driver CoG (SPI + GPIO)
      epaper_display.h/cpp  -- API de alto nivel
      epd_graphics.h/cpp    -- Utilidades de buffer
      boards.h              -- Configuracion de pines por modelo
    fonts/
      fonts.h/cpp           -- Datos de fuentes bitmap
      fonts_manager.h/cpp   -- Seleccion y renderizado
    gpio/gpio.h/cpp         -- Wrapper bcm2835 GPIO
    tyme/tyme.h/cpp         -- Funciones de delay
    qr/qr_gen.h/cpp         -- Generador de QR
    graphics/               -- Imagenes pre-baked
    app/config.h            -- Deteccion 32/64 bits
  docs/                     -- Documentacion
```

## Licencia

GNU
