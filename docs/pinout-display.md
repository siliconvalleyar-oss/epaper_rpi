# Pinout del Display E-Paper (Conector FPC)

## Descripción

Pinout del conector FPC de la pantalla E-Paper, según el datasheet del controlador COG.

## Tabla de Pines

| Pin # | Símbolo | Descripción |
|:-----:|:-------:|-------------|
| 1 | `VCC` | Alimentación 3.3V |
| 2 | `SCK` | SPI Clock, entrada de reloj de comunicación serie |
| 3 | `BUSY` | Salida de estado ocupado. **L**: driver EPD ocupado. **H**: host puede enviar comandos/datos |
| 4 | `D/C` | Control de bus serie. **L**: comando; **H**: dato |
| 5 | `RST` | Entrada de reset |
| 6 | `MISO` | SPI MISO, salida de datos serie |
| 7 | `MOSI` | SPI MOSI, entrada de datos serie |
| 8 | `FCSM` | Chip Select maestro de Flash (U1) |
| 9 | `ECSM` | Chip Select maestro de EPD |
| 10 | `GND` | Tierra |
| 11 | `GND` | Tierra |
| 12 | `ECSS` | Chip Select esclavo de EPD |
| 13 | `VGH` | Tensión positiva de gate driving |
| 14 | `VDH` | Tensión positiva de source/data driving |
| 15 | `VGL` | Tensión negativa de gate driving |
| 16 | `VDL` | Tensión negativa de source/data driving |
| 17 | `VCOM` | Tensión de VCOM driving |
| 18 | `VDHR` | Tensión positiva de source/data driving para color rojo |
| 19 | `VPP` | Tensión de programación OTP |
| 20 | `FCSS` | Chip Select esclavo de Flash (U2) |

## Pines de Control (los que usa el driver)

| Señal | Pin Display | GPIO RPi (Zero 2W) | Pin Header RPi | Descripción |
|-------|:-----------:|:------------------:|:--------------:|-------------|
| `SCK` | 2 | GPIO11 | 23 | SPI Clock |
| `BUSY` | 3 | GPIO25 | 22 | Estado ocupado (entrada) |
| `D/C` | 4 | GPIO24 | 18 | Data/Command (salida) |
| `RST` | 5 | GPIO23 | 16 | Reset (salida) |
| `MISO` | 6 | GPIO9 | 21 | SPI MISO (no usado) |
| `MOSI` | 7 | GPIO10 | 19 | SPI MOSI |
| `ECSM` | 9 | GPIO27 | 13 | Chip Select EPD (salida) |
| `GND` | 10, 11 | GND | 6, 14, 20, 25 | Tierra |

## Mapa de Conexiones (colores de cables)

| Señal | Color Cable | Pin Display | GPIO RPi | Pin Header |
|-------|:-----------:|:-----------:|:--------:|:----------:|
| CS | Gris | 9 (ECSM) | GPIO27 | 13 |
| MOSI | Azul | 7 | GPIO10 | 19 |
| SCK | Marrón | 2 | GPIO11 | 23 |
| MISO | Verde | 6 | GPIO9 | 21 |
| Flash CS | Violeta | 8 (FCSM) | GPIO22 | 15 |
| RESET | Amarillo | 5 | GPIO23 | 16 |
| D/C | Naranja | 4 | GPIO24 | 18 |
| BUSY | Rojo | 3 | GPIO25 | 22 |

## Notas

- Los pines 13-20 son salidas del DC/DC converter interno del COG y **no deben ser conectados** a ningún GPIO de la Raspberry Pi.
- El display se alimenta a **3.3V** solamente. No usar 5V.
- La corriente pico durante actualización es de 10-50 mA.
- `BUSY` = LOW cuando el COG está ocupado, HIGH cuando está listo.
- `D/C` = LOW para enviar comandos, HIGH para enviar datos.
- `ECSM` (CS) es activo en LOW.
- `RST` requiere un pulso LOW de al menos 10 ms para resetear el COG.
