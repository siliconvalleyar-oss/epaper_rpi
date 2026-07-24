# Hardware y Conexiones Físicas

## Visión General

El E-Paper se conecta a Raspberry Pi mediante el bus SPI (Serial Peripheral Interface). La comunicación requiere señales de control adicionales: Reset, Data/Command, Chip Select y Busy.

## Arquitectura de Conexión

```
Raspberry Pi                  Pantalla E-Paper
      |                               |
      |--------- SPI BUS -------------|
      |   SCLK (GPIO11) -----------> SCK
      |   MOSI (GPIO10) -----------> SDIN
      |   MISO (GPIO9) <----------- SDOUT
      |                               |
      |---- GPIO -------- RESET ------|
      |---- GPIO -------- D/C --------|
      |---- GPIO -------- CS ---------|
      |---- GPIO -------- BUSY <------|
      |---- GPIO -------- Flash CS ---|
```

## Tablas de Pinout

### Configuración Pi 2W / Pi 4 (Recomendada)

| Señal Panel |Nombre Pin | GPIO BCM | Pin Header RPi | Función SPI |
|-------------|-----------|----------|----------------|-------------|
| Panel_CS   | WIRE_GRAY | 27       | 13             | SPI      |
| MOSI       | WIRE_BLUE | 10       | 19             | SPI MOSI    |
| SCK        | WIRE_BROWN| 11       | 23             | SPI SCLK    |
| MISO       | WIRE_GREEN| 9        | 21             | SPI MISO    |
| Flash_CS   | WIRE_VIOLET| 22     | 15             | GPIO        |

| RESET      | WIRE_YELLOW|   23   | 16             |         |
| D/C        | WIRE_ORANGE|   24   | 18             |       |
| BUSY       | WIRE_RED  |     25   | 22            |       |

### Configuración Others (Alternativa)

| Señal Panel |Nombre Pin | GPIO BCM | Pin Header RPi | Función SPI |
|-------------|-----------|----------|----------------|-------------|
| Panel_CS   | WIRE_GRAY | 2        | 13             | SPI CE1     |
| MOSI       | WIRE_BLUE | 12       | 19             | SPI MOSI    |
| SCK        | WIRE_BROWN| 14       | 23             | SPI SCLK    |
| MISO       | WIRE_GREEN| 13       | 21             | SPI MISO    |
| Flash_CS   | WIRE_VIOLET| 3      | 15             | GPIO        |
| RESET      | WIRE_YELLOW| 6      | 22             | GPIO        |
| D/C        | WIRE_ORANGE| 10     | 24             | SPI CE0     |
| BUSY       | WIRE_RED  | 11       | 26             | SPI CE1     |

## Especificaciones Eléctricas

| Parámetro | Valor | Unidad |
|-----------|-------|--------|
| Voltaje de operación | 3.3 | V |
| Corriente de reposo | < 1 | mA |
| Corriente de actualización | 10-50 | mA |
| Voltajes máximos de entrada | 3.6 | V |
| Niveles lógicos | 3.3V | CMOS |

## Rutas de Señal SPI

### SPI0 (HW)

| RPi Pin | GPIO | Función SPI0 | Descripción |
|---------|------|--------------|-------------|
| 19 | GPIO10 | MOSI | Master Out Slave In |
| 21 | GPIO9 | MISO | Master In Slave Out |
| 23 | GPIO11 | SCLK | Serial Clock |
| 24 | GPIO8 | CE0 | Chip Enable 0 |
| 26 | GPIO7 | CE1 | Chip Enable 1 |

### Frecuencia SPI

Configurada en `spi.h`:
- Velocidad máxima: 1.6 MHz (SPI_SPEED)
- Modo: SPI_MODE0 (CPOL=0, CPHA=0)
- Orden de bits: MSB primero

## Conexiones del Bus de Datos

### Datos de la Pantalla

La pantalla E-Paper 2.66" tiene una resolución de **296 x 152 píxeles**.

- **Total de píxeles**: 44,992
- **Bytes por frame**: 5,624 (296 x 152 / 8)
- **Total de bytes para BWR**: 11,248 (2 frames)

### Buffer de Imagen

```cpp
const size_t bufferSize = 5624; // bytes
```

El buffer se organiza en:
- **Columnas**: 296 píxeles (37 bytes por fila)
- **Filas**: 152 (152 filas de 37 bytes = 5,624 bytes)
- **Formato**: 1 bit por píxel (MSB primero)

## Mapa de Bits por Byte

```
Byte N:   [ b7 | b6 | b5 | b4 | b3 | b2 | b1 | b0 ]
Pixel:    [P+7 |P+6 |P+5 |P+4 |P+3 |P+2 |P+1 |P+0 ]
Valor:    [ 1  | 1  | 0  | 0  | 0  | 0  | 0  | 0  ] = 0xCO (no todos los píxeles negros)
```

Donde:
- `1` = negro (en BW, píxel encendido)
- `0` = blanco (en BW, píxel apagado)

Para BWR:
- Frame 1 (0x10): Canal negro
- Frame 2 (0x13): Canal rojo

## Conexión de Flash

El pin Flash_CS está presente en la configuración del pines pero no es utilizado por el driver actual. Está reservado para futuras expansiones con memoria Flash SPI externa.

```
Flash_CS -> GPIO22 (Pin 15)
```

## Secuencia de Encendido

La secuencia de hardware al encender el sistema:

1. **Power stable**: Esperar estabilización de VCC (3.3V)
2. **RESET pulse**: Pulso de reset con temporización:
   - RES# = HIGH por 5ms
   - RES# = LOW por 10ms
   - RES# = HIGH por 5ms
3. **SPI config**: Configurar SPI en modo 0, MSB primero
4. **COG init**: Comandos de inicialización COG
5. **Wait BUSY**: Esperar señal BUSY = HIGH

## Consideraciones de Diseño

### Pull-up/Pull-down

- **RESET**: Internamente pull-up
- **BUSY**: Entrada, sin resistencia interna crítica
- **CS**: Pull-up interno (activo en bajo)
- **D/C**: Pull-up interno recomendado

### Longitud de Cables

Para señales SPI a 1.6 MHz:
- Máximo recomendado: 30 cm
- Usar cables planos o twisted pair para MISO/MOSI
- Agrupar GND junto a cada señal para reducir EMI

### Alimentación

- La pantalla E-Paper consume poca corriente en reposo
- Picos de hasta 50mA durante actualizaciones
- No requiere fuente de alimentación separada en la mayoría de casos
- Para pantallas grandes (3.7"+), considerar fuente externa de 3.3V/500mA

## GPIO en Distintas Arquitecturas

El código distingue automáticamente entre sistemas de 32 y 64 bits:

### 32-bit (armv7l/arm)

GPIO directo (números de hardware):
- 0 a 53

### 64-bit (aarch64)

GPIO con offset BCM 512:
- 512 a 569 (corresponden a GPIO 0 a 57)

Ejemplo de mapeo:
```
GPIO físico 26 -> BCM 7 (32-bit) -> ID 519 (64-bit)
```

Los scripts bash usan `raspi-gpio` que maneja esta abstracción automáticamente.

## Configuración de Pines GPIO por Modelo

### pins_t Definiciones

```cpp
struct pins_t {
    uint16_t panelBusy;       // GPIO para BUSY
    uint16_t panelDC;         // GPIO para Data/Command
    uint16_t panelReset;      // GPIO para RESET
    uint16_t panelCS;         // GPIO para Chip Select
    uint16_t panelON_EXT2;    // GPIO adicional (PANEL_ON)
    uint16_t panelSPI43_EXT2; // GPIO para select de velocidad BS
    uint16_t flashCS;         // GPIO para Flash CS
};
```

### boardRaspberryPiZero2W

```cpp
const pins_t boardRaspberryPiZero2W = {
    .panelBusy       = 25,    // GPIO25 - Pin 22
    .panelDC         = 24,    // GPIO24 - Pin 18
    .panelReset      = 23,    // GPIO23 - Pin 16
    .panelCS         = 8,     // GPIO8  - Pin 24
    .panelON_EXT2    = NOT_CONNECTED,
    .panelSPI43_EXT2 = NOT_CONNECTED,
    .flashCS         = 22     // GPIO22 - Pin 15
};
```
