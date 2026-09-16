# E-Paper Clock - RASPI_EPD_SRC_Clock_Ascii

Reloj en pantalla e-paper configurable (2.13" o 2.66") para Raspberry Pi Zero 2W.

## Que hace

Muestra un reloj en tiempo real con:
- Hora: `HH:MM:SS` en fuente mediana (FONT_16x16_MEDNUM, 16x16 pixeles)
- Fecha: `YYYY-MM-DD` en fuente 5x8
- Dia de la semana: en fuente 7x8 THICK
- Tiempo de actividad: `UP: HH:MM:SS`
- Lineas separadoras horizontales

## Configuracion de pantalla

Edita `src/main.cpp` y cambia la macro `SCREEN`:

```cpp
#define SCREEN 213   // 2.13" (212x104)
#define SCREEN 266   // 2.66" (296x152)
```

El layout se ajusta automaticamente segun el display seleccionado.

## Layout de pantalla

### 2.13" (212x104)
```
y=5:    "E-PAPER CLOCK"       FONT_5x8
y=16:   ───────────────
y=18:   "12:34:56"            FONT_16x16_MEDNUM (centrado)
y=38:   ───────────────
y=42:   "2026-07-24"          FONT_5x8
y=55:   "DOMINGO"             FONT_7x8_THICK
y=68:   "UP: 00:05:32"        FONT_5x8
y=80:   ───────────────
```

### 2.66" (296x152)
```
y=5:    "E-PAPER CLOCK"       FONT_5x8
y=18:   ───────────────
y=22:   "12:34:56"            FONT_16x16_MEDNUM (centrado)
y=42:   ───────────────
y=48:   "2026-07-24"          FONT_5x8
y=65:   "DOMINGO"             FONT_7x8_THICK
y=85:   "UP: 00:05:32"        FONT_5x8
y=105:  ───────────────
```

## Compilar y ejecutar

```bash
make clean && make -j4
make run   # ejecuta con sudo
```

## Compilacion remota

```bash
sshpass -p zero ssh pi@raspi.local \
  "cd /home/pi/src/epaper_rpi && git pull && \
    rm -rf RASPI_EPD_SRC_Clock_Ascii/obj RASPI_EPD_SRC_Clock_Ascii/bin && \
   make -C RASPI_EPD_SRC_Clock_Ascii -j4 && \
   sudo ./RASPI_EPD_SRC_Clock_Ascii/bin/epaper_app"
```

## Cableado (Zero 2W)

| Color   | GPIO | Pin | Funcion |
|---------|------|-----|---------|
| Rojo    | 25   | 22  | BUSY (input) |
| Naranja | 24   | 18  | DC (output) |
| Amarillo| 23   | 16  | RESET (output) |
| Gris    | 27   | 13  | CS (output) |
| Violeta | 22   | 15  | Flash CS |
| Azul    | 10   | 19  | MOSI (SPI) |
| Marron  | 11   | 23  | SCLK (SPI) |
| Verde   | 9    | 21  | MISO (SPI, no usado) |

## Dependencias

```bash
sudo apt install libbcm2835-dev
```

## Estructura

```
RASPI_EPD_SRC_Clock_Ascii/
├── Makefile
├── libs/
│   ├── app/config.h           # Deteccion 32/64 bits
│   ├── epaper/
│   │   ├── boards.h           # Configuraciones de pines
│   │   ├── epaper.h           # Driver EPD (bcm2835 SPI)
│   │   ├── epaper.cpp         # Implementacion driver
│   │   ├── epaper_display.h   # API de dibujo (buffer, texto, lineas)
│   │   └── epaper_display.cpp # Implementacion API
│   ├── fonts/
│   │   ├── fonts.h            # 9 fuentes bitmap declaradas
│   │   ├── fonts.cpp          # Datos de las fuentes
│   │   ├── fonts_manager.h    # Gestor de fuentes
│   │   └── fonts_manager.cpp  # Implementacion gestor
│   └── tyme/
│       ├── tyme.h             # Funciones de delay
│       └── tyme.cpp           # Implementacion (usleep)
└── src/
    └── main.cpp               # Aplicacion reloj
```

## Fuentes disponibles

| Fuente | Tamano | Descripcion |
|--------|--------|-------------|
| FONT_5x8 | 5x8 | ASCII estandar |
| FONT_7x8_THICK | 7x8 | Negrita (solo mayusculas) |
| FONT_4x8_SEG | 4x8 | Siete segmentos |
| FONT_8x8_WIDE | 8x8 | Ancha (solo mayusculas) |
| FONT_3x8_TINY | 3x8 | Minima |
| FONT_7x8_HOMESPUN | 7x8 | Retro |
| FONT_8x8 | 8x8 | Original C64 |
| FONT_16x32_BIGNUM | 16x32 | Numeros grandes (0-9, :) |
| FONT_16x16_MEDNUM | 16x16 | Numeros medianos (0-9, :) |

## Nota sobre refresh

El refresh completo de la pantalla e-paper 2.66" toma ~15 segundos (hardware). El reloj actualiza cada segundo, pero el display solo muestra el cambio cuando el refresh termina.
