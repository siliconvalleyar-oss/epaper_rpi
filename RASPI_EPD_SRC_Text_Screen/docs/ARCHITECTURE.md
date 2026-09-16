# Arquitectura del Sistema

## Capas

```
┌─────────────────────────────────────┐
│  main.cpp                           │  ← Aplicación
├─────────────────────────────────────┤
│  EpaperDisplay                      │  ← API de alto nivel (dibujo, texto, patrones)
├─────────────────────────────────────┤
│  EPD_Driver                         │  ← Driver CoG (SPI + GPIO, protocolo EPD)
│  FontManager / epd_graphics         │  ← Renderizado de fuentes y gráficos
├─────────────────────────────────────┤
│  EPAPER::Spi_t / EPAPER::Gpio_t     │  ← Wrapper bcm2835 (SPI y GPIO hardware)
├─────────────────────────────────────┤
│  bcm2835 library                    │  ← Acceso directo a registros del SoC
├─────────────────────────────────────┤
│  Hardware Raspberry Pi              │  ← SPI0 + GPIO BCM
└─────────────────────────────────────┘
```

## Componentes principales

### EPD_Driver (`libs/epaper/epaper.cpp`)

Driver de bajo nivel para el chip COG (Chip On Glass) de Pervasive Displays.

- `COG_initial()` — Secuencia de inicio: configura pines, reset hardware, soft reset, temperatura
- `sendIndexData()` — Protocolo SPI: envía comando (DC=LOW) + datos (DC=HIGH), CS pulsado por byte
- `globalUpdate()` — Envía frames de imagen (cmds 0x10 y 0x13), enciende DC/DC y refresca
- `COG_powerOff()` — Apaga DC/DC y pone pines en estado seguro

### EpaperDisplay (`libs/epaper/epaper_display.cpp`)

API de alto nivel para dibujo en el buffer de pantalla.

- Gestiona buffer de imagen (1 bit por píxel, column-major con espejo horizontal)
- Funciones de dibujo: pixel, línea, rectángulo, texto, centrado
- Coordina con FontManager para renderizar caracteres

### FontManager (`libs/fonts/fonts_manager.cpp`)

Sistema de fuentes bitmap con 9 fuentes disponibles.

- `setFont()` — Selecciona fuente activa
- `getCharBitmap()` — Obtiene bitmap de un carácter
- `printChar()` / `printString()` — Debug visual en consola

### epd_graphics (`libs/epaper/epd_graphics.cpp`)

Utilidades C de bajo nivel para manipulación de buffer (alternativa al sistema de clases).

## Flujo de ejecución

```
bcm2835_init()
    │
    ├── EpaperDisplay(screen_type, board)
    │       ├── new buffer[]
    │       ├── EPD_Driver(screen_type, board)
    │       │       └── Spi_t() → bcm2835_spi_begin()
    │       └── clearScreen()
    │
    ├── display.init() → COG_initial()
    │       ├── pinMode(BUSY=IN, DC=OUT, RESET=OUT, CS=OUT)
    │       ├── reset(5,5,10,5,5)
    │       ├── softReset()
    │       ├── sendIndexData(0xe5, temp, 1)
    │       ├── sendIndexData(0xe0, reg[2], 1)
    │       └── sendIndexData(0x00, reg[3], 2)
    │
    ├── drawString() / drawLine() / etc.  ← Modifica buffer[]
    │
    ├── display.update() → globalUpdate(buffer, buffer)
    │       ├── sendIndexData(0x10, buffer, image_size)    ← Frame 1
    │       ├── sendIndexData(0x13, buffer, image_size)    ← Frame 2
    │       ├── DCDC_powerOn() → cmd 0x04
    │       └── displayRefresh() → cmd 0x12
    │
    └── ~EpaperDisplay() → delete buffer
            └── ~Spi_t() → bcm2835_spi_end()
                    └── bcm2835_close()
```

## Orden de inicialización crítico

1. `bcm2835_init()` **primero** (main)
2. `Spi_t()` llama `bcm2835_spi_begin()` (configura GPIO 9-11 como ALT0)
3. Al destruir: `~Spi_t()` llama `bcm2835_spi_end()` (restaura GPIO)
4. `bcm2835_close()` **último** (main)

El display se destruye dentro de un bloque `{}` ANTES de `bcm2835_close()`.
