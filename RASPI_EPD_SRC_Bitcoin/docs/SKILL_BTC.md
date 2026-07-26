# SKILL: Bitcoin Ticker — E-Paper Display

## 1. Descripción General

Aplicación que muestra el precio de Bitcoin (BTC/USD) en tiempo real en una pantalla
e-paper de 2.66" (296×152), usando la API gratuita de CoinGecko.

**Branch**: `Bitcoin`
**Carpeta**: `RASPI_EPD_SRC_Bitcoin/`
**Binario**: `bin/bitcoin_app`

### Funcionalidad

- Precio BTC/USD desde CoinGecko (cada 60s)
- Zona horaria Argentina (ART) configurada por TZ
- 6 fuentes distintas: precio grande (16×32), texto fino, tiny para info
-   Refresh diferencial (`fastUpdate`) con softReset
-   Inicial con `globalUpdate`, luego `fastUpdate` para evitar flash

---

## 2. Estructura del Proyecto

```
RASPI_EPD_SRC_Bitcoin/
├── Makefile                          # Build system
├── README.md
├── src/
│   └── main.cpp                      # Entry point, loop, fetch + render
├── libs/
│   ├── epaper/
│   │   ├── epaper.h                  # Driver EPD: EPD_Driver, Spi_t, Gpio_t
│   │   ├── epaper.cpp                # Implementación SPI, COG, updates
│   │   ├── epaper_display.h          # Framebuffer + drawing (EpaperDisplay)
│   │   ├── epaper_display.cpp        # drawPixel, drawChar, drawString, update
│   │   └── boards.h                  # Pinouts para RPi 3B/4B/Zero2W/4/5
│   ├── fonts/
│   │   ├── fonts.h                   # Declaración global de todas las fuentes
│   │   ├── fonts.cpp                 # Datos de 8 fuentes bitmap
│   │   ├── fonts_manager.h           # FontManager class
│   │   └── fonts_manager.cpp         # getCharBitmap(), setFont(), ranges
│   ├── gpio/
│   │   └── gpio.h/.cpp               # Legacy GPIO class (no usado, mantener)
│   ├── tyme/
│   │   └── tyme.h/.cpp               # delay_ms, delay_us (usleep wrapper)
│   └── app/
│       └── config.h                  # CPU_32_BITS / CPU_64_BITS detection
└── docs/
    └── SKILL_BTC.md                  # Este archivo
```

---

## 3. Build System

### Makefile

```makefile
CC = g++
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
LIBRARIES = -pthread -lbcm2835
```

Compila automáticamente todos los `.cpp` en `src/` y recursivamente en `libs/*/`.

### Comandos

```bash
make            # Compila → bin/bitcoin_app
make run        # Compila + ejecuta con sudo y TZ=America/Argentina/Buenos_Aires
make clean      # Borra obj/ y bin/
```

### Dependencias del sistema (RPi)

```bash
sudo apt-get install libbcm2835-dev    # bcm2835.h + librería
```

**No requiere** `libqrencode-dev` (a diferencia de QR projects).

---

## 4. Arquitectura de la Aplicación

### 4.1 Flujo principal (main.cpp)

```
main()
├── setenv TZ=America/Argentina/Buenos_Aires
├── signal(SIGINT/SIGTERM → running=false)
├── bcm2835_init()
├── EpaperDisplay(266, boardRaspberryPi)
│   └── init()
│       ├── EPD_Driver.COG_initial()
│       │   ├── pinMode all GPIOs
│       │   ├── reset(5,5,10,5,5)          ← hardware RESET pulse
│       │   ├── softReset()                 ← command 0x00 + wait BUSY
│       │   ├── 0xE5 (input temp)           ← reg[2] = 0x19 (25°C)
│       │   ├── 0xE0 (active temp)          ← reg[3] = 0x02
│       │   └── 0x00 PSR (2 bytes)          ← reg[4..5] = 0xcf, 0x8d
│       └── globalUpdate(buffer, buffer)    ← full refresh inicial
├── loop (60s):
│   ├── fetchBtcPrice() via curl            ← CoinGecko API
│   ├── clearScreen(true)                   ← limpia buffer a blanco
│   ├── draw BITCOIN title (FONT_7x8_THICK)
│   ├── draw price $64,xxx.xx (FONT_16x32_BIGNUM + FONT_5x8)
│   ├── draw "USD 1 BTC" (FONT_5x8)
│   ├── draw date dd/mm/yyyy (FONT_5x8)
│   ├── draw time HH:MM:SS ART (FONT_5x8)
│   ├── draw refresh info (FONT_3x8_TINY)
│   ├── draw "via CoinGecko API" (FONT_3x8_TINY)
│   └── update()
│       ├── hasContentChanged() ?           ← memcmp buffers
│       └── fastUpdate(prev, current)       ← diff-based refresh
└── ~EpaperDisplay → bcm2835_close()
```

### 4.2 fetchBtcPrice() — CoinGecko API

```cpp
static double fetchBtcPrice() {
    // curl -s --connect-timeout 5 --max-time 10
    //   'https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd'
    // Parse JSON response: {"bitcoin":{"usd":64375.0}}
    // Extrae valor después de "usd": → atof()
}
```

- Timeout 5s conexión, 10s total
- Retorna -1 en error (se reintenta en el próximo ciclo)
- Si > 3 errores consecutivos, muestra "NO DATA" en vez del precio

### 4.3 priceStr() — Formateo de precio

```cpp
static std::string priceStr(double price) {
    // Convierte 64375.00 → "64,375.00"
    // Separa miles con coma, 2 decimales
    // Maneja negativos (no aplica para BTC pero por robustez)
}
```

### 4.4 Timing

- Cada 60 segundos: fetch + render completo + update
- `hasContentChanged()` detecta cambios entre buffers
- `fastUpdate()` solo envia diff content (OLD vs NEW)
- Si contenido no cambió, no se envía nada al display

---

## 5. Driver E-Paper (EPD_Driver)

### 5.1 Comunicación SPI

- Modo: `MODE0` (CPOL=0, CPHA=0), MSB First
- Clock: `BCM2835_SPI_CLOCK_DIVIDER_256` (~976 KHz)
- CS: **manual por GPIO** (`BCM2835_SPI_CS_NONE`)
- CS se togglea entre cada byte (alto entre comando y datos, y entre cada byte de datos)

### 5.2 Registros de configuración

```cpp
// register_data[6] = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };
//                       [0]    [1]    [2]    [3]    [4]    [5]
// 0x00: dummy
// 0x01: soft-reset argument
// 0x02: input temperature (0x19 = 25°C)     → comando 0xE5
// 0x03: active temperature (0x02)           → comando 0xE0
// 0x04-0x05: PSR (Panel Setting Register)    → comando 0x00
```

### 5.3 COG_initial()

1. Configurar todos los pines GPIO
2. `reset(5, 5, 10, 5, 5)` — hardware RESET pulse
3. `softReset()` — comando SPI 0x00 + esperar BUSY=HIGH (timeout 5s)
4. `sendIndexData(0xE5, &reg[2], 1)` — input temperature
5. `sendIndexData(0xE0, &reg[3], 1)` — active temperature
6. `sendIndexData(0x00, &reg[4], 2)` — PSR (panel settings)

### 5.4 globalUpdate()

```cpp
void globalUpdate(const uint8_t *data1s, const uint8_t *data2s) {
    sendIndexData(0x10, data1s, image_data_size);  // canal negro/BW
    sendIndexData(0x13, data2s, image_data_size);  // canal rojo/segundo plano
    DCDC_powerOn();   // comando 0x04, wait BUSY
    displayRefresh(); // comando 0x12, wait BUSY (timeout 20s)
}
```

- `data2s` se envía a `0x13`
- Pixel final = `data1s XOR data2s`
- Si `data2s` = 0x00 (ceros), pixel = data1s

### 5.5 fastUpdate()

```cpp
void fastUpdate(const uint8_t *oldData, const uint8_t *newData) {
    // 1. Comparar buffers, si no hay cambios → return
    // 2. softReset() ← CRÍTICO: necesario para que fastUpdate funcione
    // 3. tempFast = reg[2] | 0x40   (bit 6 = fast update flag)
    // 4. sendCommandData8(0xE5, tempFast)
    // 5. sendCommandData8(0xE0, reg[3])
    // 6. psrFast[2] = { reg[4] | 0x10, reg[5] | 0x02 }
    // 7. sendCommandData8(0x50, 0x07)   ← CDI (Charge/Discharge Interval)
    // 8. sendIndexData(0x10, oldData)    ← OLD image (se borra)
    // 9. sendIndexData(0x13, newData)    ← NEW image (se muestra)
    // 10. DCDC_powerOn() + displayRefresh()
}
```

**Importante**: Los canales están **invertidos** respecto a globalUpdate:
- globalUpdate: `0x10=data`, `0x13=ceros`, pixel = data XOR 0
- fastUpdate:   `0x10=OLD`, `0x13=NEW`, el display muestra NEW sobre OLD

**softReset() es obligatorio** en fastUpdate. Sin él, el contenido no se muestra.

---

## 6. EpaperDisplay — Framebuffer y Dibujo

### 6.1 Organización del buffer

```
Pantalla 2.66" (296×152):
  - 296 columnas, 152 filas
  - image_data_size = 296 * (152/8) = 5624 bytes
  - Formato: column-major, MSB = top
    Byte 0: [b7 b6 b5 b4 b3 b2 b1 b0] → filas 0-7 de columna 0
    ...
```

### 6.2 drawPixel()

```cpp
void drawPixel(int x, int y, bool black) {
    // Espejo horizontal: mirroredX = width - 1 - x
    // byteIndex = (mirroredX * height + y) / 8
    // bitIndex = 7 - (y % 8)
    // black ? set bit : clear bit
}
```

El espejo horizontal es necesario porque el controlador del display 2.66"
muestra la columna 0 en el borde derecho de la pantalla.

### 6.3 drawCharToBuffer()

Dos modos según tipo de fuente:

**Big number fonts** (FONT_16x32_BIGNUM, FONT_16x16_MEDNUM):
- Column-major, MSB=top
- `bytesPerCol = height / 8`
- `byteIndex = col * bytesPerCol + (row / 8)`
- `bitIndex = 7 - (row % 8)`

**Regular fonts** (FONT_5x8, FONT_7x8_THICK, etc.):
- Row-major, cada byte es una columna
- `byte = bitmap[col]`
- `pixel = (byte >> row) & 0x01`

### 6.4 update()

```cpp
bool update() {
    if (!hasContentChanged()) return false;  // memcmp buffers
    m_driver->fastUpdate(m_prevBuffer, m_buffer);
    memcpy(m_prevBuffer, m_buffer, m_bufferSize);
    return true;
}
```

### 6.5 init()

```cpp
bool init() {
    m_driver->COG_initial();
    clearScreen(true);
    m_driver->globalUpdate(m_buffer, m_buffer);  // full refresh inicial
    memcpy(m_prevBuffer, m_buffer, m_bufferSize);
    return true;
}
```

La primera actualización DEBE ser `globalUpdate` (full refresh). `fastUpdate`
no funciona como primera operación sin una inicialización completa.

---

## 7. Sistema de Fuentes

### 7.1 Fuentes disponibles

| FontType | Ancho | Alto | start_char | end_char | Bytes/char | Array |
|:---------|:-----:|:----:|:----------:|:--------:|:----------:|:------|
| FONT_8x8 | 8 | 8 | 0 | 127 | 8 | `font[1024]` |
| FONT_5x8 | 5 | 8 | 0 | 127 | 5 | `Font_One[]` |
| FONT_7x8_THICK | 7 | 8 | **32** | 127 | 7 | `Font_Two[]` |
| FONT_4x8_SEG | 4 | 8 | **32** | 122 | 4 | `Font_Three[]` |
| FONT_8x8_WIDE | 8 | 8 | **32** | 90 | 8 | `Font_Four[]` |
| FONT_3x8_TINY | 3 | 8 | **32** | 126 | 3 | `Font_Five[]` |
| FONT_7x8_HOMESPUN | 7 | 8 | **32** | 126 | 7 | `Font_Six[]` |
| FONT_16x32_BIGNUM | 16 | 32 | **48** | 58 | 64 | `Font_Seven[11][64]` |
| FONT_16x16_MEDNUM | 16 | 16 | **48** | 58 | 32 | `Font_Eight[11][32]` |

**Atención**: `start_char` debe coincidir con el primer carácter real en los datos:
- Fuentes que empiezan en espacio (ASCII 32): `start_char=32`
- Fuentes `FONT_8x8` y `FONT_5x8`: empiezan en ASCII 0, `start_char=0`
- Fuentes de números grandes: `start_char=48` ('0'), 11 caracteres ('0'-'9' + ':')

### 7.2 Caracteres en FONT_16x32_BIGNUM y FONT_16x16_MEDNUM

```cpp
// Font_Seven[11][64], Font_Eight[11][32]
// [0]='0', [1]='1', ..., [9]='9', [10]=':'
// layout: column-major, 16 columnas, cada columna = height/8 bytes
// MSB=top dentro de cada byte
```

### 7.3 FontManager

```cpp
FontManager fm;
fm.setFont(FONT_7x8_THICK);
const uint8_t* bitmap = fm.getCharBitmap('A');
int index = (int)'A' - currentFont.start_char;  // 65 - 32 = 33
return &fontData[index * charSize];              // Font_Two[33*7] = 231 bytes offset
```

Para fuentes 2D (big number), usa `static_cast<const uint8_t(*)[64]>(data)`.

---

## 8. GPIO y Board Config

### 8.1 Pinout (RPi 3B/4B/Zero2W)

| Señal | GPIO | Pin RPi | Color cable |
|:------|:----:|:-------:|:-----------:|
| BUSY | 25 | 22 | Rojo |
| D/C | 24 | 18 | Naranja |
| RESET | 23 | 16 | Amarillo |
| CS | 27 | 13 | Gris |
| flashCS | 22 | 15 | Violeta (no usado) |
| MOSI | 10 | 19 | Azul |
| SCLK | 11 | 23 | Marrón |

### 8.2 Boards disponibles

```cpp
boardRaspberryPi       // Default: GPIO25,24,23,27
boardRaspberryPiZero2W // Igual que default
boardRaspberryPi4      // Alternativo: GPIO27,18,17,8
getBoardConfig("model")// Helper inline (no usado en Bitcoin)
```

---

## 9. Problemas Conocidos y Soluciones

### 9.1 fastUpdate no muestra contenido

**Causa**: Faltaba `softReset()` al inicio de `fastUpdate()`.
En la versión del driver copiada de QR, `fastUpdate` no llamaba a `softReset`.
El clock project sí lo tenía.

**Solución**: Agregar `softReset()` en `fastUpdate()` antes de configurar los
registros de fast update:
```cpp
if (!hasChanges) return;
softReset();  // ← necesario
uint8_t tempFast = register_data[2] | 0x40;
```

### 9.2 Fuente FONT_3x8_TINY muestra basura

**Causa**: `start_char = 0` cuando los datos de `Font_Five[]` empiezan en
ASCII 32 (espacio). Al renderizar texto normal, el índice calculado eras
incorrecto y se accedía a memoria fuera de los datos de la fuente.

**Solución**: Cambiar a `start_char = 32`, `end_char = 126`.

Afectó también a:
- `FONT_7x8_HOMESPUN` (Font_Six)
- `FONT_4x8_SEG` (Font_Three)
- `FONT_8x8_WIDE` (Font_Four)

### 9.3 globalUpdate parpadea cada 1s

**Causa**: Si se usa `globalUpdate` en lugar de `fastUpdate` en el loop,
cada cambio (incluso el segundero que cambia cada 1s) provoca un refresh
completo con flash.

**Solución**: Usar `fastUpdate()` en `update()`. `globalUpdate` solo para
la inicialización (`init()`).

### 9.4 Narrowing warning en psrFast

```cpp
uint8_t psrFast[2] = { register_data[4] | 0x10, ... };
// warning: narrowing conversion from 'int' to 'uint8_t'
// Solución:
uint8_t psrFast[2] = { static_cast<uint8_t>(register_data[4] | 0x10), ... };
```

---

## 10. Deploy Remoto

### Compilar y ejecutar en RPi remota

```bash
# Sync local → RPi
rsync -avz --exclude='.git/' --exclude='obj/' --exclude='bin/' \
  RASPI_EPD_SRC_Bitcoin/ pi@raspi.local:/home/pi/src/epaper_rpi/RASPI_EPD_SRC_Bitcoin/

# Compilar en RPi
ssh pi@raspi.local "cd /home/pi/src/epaper_rpi/RASPI_EPD_SRC_Bitcoin && make clean && make"

# Ejecutar en RPi
ssh pi@raspi.local "cd /home/pi/src/epaper_rpi/RASPI_EPD_SRC_Bitcoin && \
  sudo TZ=America/Argentina/Buenos_Aires ./bin/bitcoin_app"
```

### Via git_menu.sh

```bash
# En la raíz del repositorio:
./scripts_tools/git_menu.sh
# → q: Deploy QR (también sirve para otros proyectos)
#   o directamente deploy manual
```

---

## 11. Próximas Mejoras Posibles

1. **Throttling de update**: Solo hacer `fastUpdate` cuando cambie el precio,
   no cada 60s. El tiempo se actualiza cada segundo pero el display solo
   refresca con hasContentChanged.
2. **Múltiples criptos**: Añadir ETH, SOL con toggle por botón.
3. **Gráfico de velas**: Usar `drawLine` y `drawPixel` para mini chart.
4. **Configuración WiFi fallback**: Si no hay internet, mostrar último precio
   conocido.
5. **Detección de 7-segment display**: Usar FONT_4x8_SEG para estilo
   calculadora financiera.
