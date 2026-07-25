# Conexiones Hardware

## Cableado EXT3 → Raspberry Pi Zero 2W

| Cable  | Color    | EXT3 Pin | Función  | GPIO BCM | RPi Pin |
|--------|----------|----------|----------|----------|---------|
| Rojo   | 🔴       | 3        | BUSY     | GPIO25   | Pin 22  |
| Naranja| 🟠       | 4        | D/C      | GPIO24   | Pin 18  |
| Amarillo| 🟡      | 5        | RESET    | GPIO23   | Pin 16  |
| Verde  | 🟢       | 6        | MISO     | GPIO9    | Pin 21  |
| Azul   | 🔵       | 7        | MOSI     | GPIO10   | Pin 19  |
| Gris   | ⚪       | 8        | CS       | GPIO27   | Pin 13  |
| Marrón | 🟤       | 2        | SCLK     | GPIO11   | Pin 23  |
| Violeta| 🟣       | 1        | Flash CS | GPIO22   | Pin 15  |

## SPI0 (bcm2835)

| Señal | GPIO | Función ALT0 | Notas |
|-------|------|-------------|-------|
| SCLK  | GPIO11 | ALT0       | Clock SPI, Marrón |
| MOSI  | GPIO10 | ALT0       | Master Out Slave In, Azul |
| MISO  | GPIO9  | ALT0       | Master In Slave Out, Verde |

## GPIO de control

| Señal  | GPIO | Dirección | Descripción |
|--------|------|-----------|-------------|
| BUSY   | 25   | INPUT     | Alto = display listo |
| D/C    | 24   | OUTPUT    | LOW = comando, HIGH = datos |
| RESET  | 23   | OUTPUT    | Pulso bajo = reset hardware |
| CS     | 27   | OUTPUT    | Chip Select, pulsado por byte |
| Flash CS | 22 | OUTPUT   | Chip Select de flash (opcional) |

## Configuración de tablas de pines

```cpp
// boards.h
const pins_t boardRaspberryPiZero2W = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Rojo
    .panelDC          = 24,    // GPIO24 - Pin 18 - Naranja
    .panelReset       = 23,    // GPIO23 - Pin 16 - Amarillo
    .panelCS          = 27,    // GPIO27 - Pin 13 - Gris
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Violeta
};
```

## SPI Configuration

- **Librería**: bcm2835 (no spidev)
- **Modo**: SPI MODE 0 (CPOL=0, CPHA=0)
- **Bit order**: MSB first
- **Clock divider**: 256 → ~976 KHz (250 MHz / 256)
- **CS hardware**: Deshabilitado (`BCM2835_SPI_CS_NONE`), CS manual por GPIO27

## Protocolo CS (Chip Select)

El CS se pulsa manualmente por cada byte transferido:

```
CS LOW → enviar byte → CS HIGH
```

Esto es diferente a mantener CS bajo durante toda la transacción.
El driver EPD de Pervasive Displays requiere este comportamiento.
