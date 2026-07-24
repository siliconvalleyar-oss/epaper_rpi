# API Reference - Driver E-Paper

## Namespace EPAPER

El namespace `EPAPER` contiene todas las clases y estructuras relacionadas con el control de pantallas E-Paper.

---

## Estructura pins_t

Define la configuración de pines GPIO para una placa específica.

```cpp
struct pins_t {
    uint16_t panelBusy;       ///< GPIO para señal BUSY (entrada)
    uint16_t panelDC;         ///< GPIO para Data/Command (salida)
    uint16_t panelReset;      ///< GPIO para Reset (salida)
    uint16_t panelCS;         ///< GPIO para Chip Select (salida)
    uint16_t panelON_EXT2;    ///< GPIO adicional PANEL_ON (opcional)
    uint16_t panelSPI43_EXT2; ///< GPIO para BS (opcional)
    uint16_t flashCS;         ///< GPIO para Flash CS (opcional)
};
```

**Campos**:

| Campo | Tipo | Dirección | Descripción |
|-------|------|-----------|-------------|
| panelBusy | uint16_t | Input | Indica estado ocupado del COG (HIGH = listo) |
| panelDC | uint16_t | Output | Selecciona comando (LOW) o dato (HIGH) |
| panelReset | uint16_t | Output | Reset hardware del COG (pulso LOW) |
| panelCS | uint16_t | Output | Chip Select del panel (activo en LOW) |
| panelON_EXT2 | uint16_t | Output | Control de encendido extendido |
| panelSPI43_EXT2 | uint16_t | Output | Selector de velocidad SPI |
| flashCS | uint16_t | Output | Chip Select para Flash SPI externa |

---

## Clase Spi_t

Wrapper de bajo nivel para el bus SPI.

### Constructores

```cpp
Spi_t();
```

Inicializa el controlador SPI, abre `/dev/spidev0.0` y configura:
- Modo: SPI_MODE0 (CPOL=0, CPHA=0)
- Velocidad: 1.6 MHz (SPI_SPEED)
- Bits por palabra: 8
- Orden: MSB primero

### Métodos

```cpp
uint8_t Transfer1bytes(uint8_t data);
```
Transfiere un solo byte por SPI.

**Parámetros**:
- `data`: Byte a enviar

**Retorna**: Byte recibido (0 en implementación actual)

```cpp
uint8_t Transfer2bytes(uint16_t cmd);
```
Transfiere 2 bytes por SPI.

**Parámetros**:
- `cmd`: Comando de 16 bits

**Retorna**: Segundo byte recibido

```cpp
uint8_t Transfer3bytes(uint32_t cmd);
```
Transfiere 3 bytes por SPI.

**Parámetros**:
- `cmd`: Comando de 32 bits

**Retorna**: Tercer byte recibido

```cpp
void spi_close();
```
Cierra el descriptor de archivo SPI.

```cpp
uint32_t get_spi_speed();
```
Obtiene la velocidad actual del SPI.

**Retorna**: Velocidad en Hz (1,600,000 por defecto)

---

## Clase Gpio_t

Wrapper de bajo nivel para GPIO usando bcm2835.

### Constructores

```cpp
explicit Gpio_t(bool enable);
```

**Parámetros**:
- `enable`: Flag de inicialización

### Métodos

```cpp
void pinMode(uint16_t pin, uint8_t mode);
```
Configura la dirección de un pin GPIO.

**Parámetros**:
- `pin`: Número de GPIO
- `mode`: `INPUT` (BCM2835_GPIO_FSEL_INPT) o `OUTPUT` (BCM2835_GPIO_FSEL_OUTP)

```cpp
void digitalWrite(uint16_t pin, uint8_t value);
```
Escribe un valor digital en un pin.

**Parámetros**:
- `pin`: Número de GPIO
- `value`: `HIGH` (0x1) o `LOW` (0x0)

```cpp
int digitalRead(uint16_t pin);
```
Lee el valor actual de un pin GPIO.

**Parámetros**:
- `pin`: Número de GPIO

**Retorna**: 1 si HIGH, 0 si LOW

```cpp
void CloseGpios();
```
Cierra la librería bcm2835 y libera recursos.

```cpp
void addGpio(uint16_t gpio_pin, std::string dir, std::string edge, std::string value);
```
Agrega un GPIO a la colección gestionada.

**Parámetros**:
- `gpio_pin`: Número de GPIO
- `dir`: "in" o "out"
- `edge`: "rising", "falling", "none" (reservado)
- `value`: "1" o "0"

```cpp
void printGpios() const;
```
Imprime la lista de GPIOs configurados.

```cpp
int gpio_get_fd_to_value(int gpio_num);
```
Lee el valor de un GPIO via sysfs.

**Parámetros**:
- `gpio_num`: Número de GPIO

**Retorna**: 0 o 1, -1 en error

```cpp
int getNextId();
```
Obtiene el siguiente ID disponible para nuevos GPIOs.

**Retorna**: ID entero (auto-incremental)

---

## Clase EPD_Driver

Controlador principal de pantallas E-Paper. Hereda de `Gpio_t`.

### Constructores

```cpp
explicit EPD_Driver(uint32_t eScreen_EPD, const pins_t& board);
```

**Parámetros**:
- `eScreen_EPD`: Código de pantalla (ver tabla de defines)
- `board`: Configuración de pines de la placa

**Excepciones**: Ninguna

**Ejemplo**:
```cpp
auto epaper = std::make_unique<EPAPER::EPD_Driver>(
    EPAPER::eScreen_EPD_266,
    EPAPER::boardRaspberryPiZero2W
);
```

**Códigos de pantalla soportados**:

| Define | Valor | Diagonal | Resolución |
|--------|-------|----------|------------|
| eScreen_EPD_154 | 0x1509 | 1.54" | 152x152 |
| eScreen_EPD_213 | 0x2100 | 2.13" | 212x104 |
| eScreen_EPD_266 | 0x2600 | 2.66" | 296x152 |
| eScreen_EPD_271 | 0x2700 | 2.71" | 264x176 |
| eScreen_EPD_287 | 0x2800 | 2.87" | 296x128 |
| eScreen_EPD_370 | 0x3700 | 3.70" | 416x240 |
| eScreen_EPD_417 | 0x4100 | 4.17" | 300x400 |
| eScreen_EPD_437 | 0x430C | 4.37" | 480x176 |

---

### Métodos Públicos

#### void COG_initial()

Inicializa el COG (Chip-on-Glass) de la pantalla E-Paper.

**Secuencia**:
1. Configura pines GPIO (BUSY: INPUT, DC/RESET/CS: OUTPUT)
2. Pulso de reset (5ms HIGH, 10ms LOW, 5ms HIGH)
3. Soft reset (comando 0x00)
4. Configuración de temperatura (0xE5, 0xE0)
5. Configuración PSR (0x00)
6. Establece dimensiones de pantalla

**Ejemplo**:
```cpp
epaper->COG_initial();
```

#### void globalUpdate(const uint8_t* data1s, const uint8_t* data2s)

Realiza una actualización global de la pantalla.

**Parámetros**:
- `data1s`: Puntero al primer frame (canal negro/previo)
- `data2s`: Puntero al segundo frame (canal rojo/nuevo)

**Secuencia**:
1. Envía frame 1 (comando 0x10, 5,624 bytes)
2. Envía frame 2 (comando 0x13, 5,624 bytes)
3. Enciende DC/DC (comando 0x04)
4. Refresco de pantalla (comando 0x12)
5. Espera BUSY=HIGH en cada etapa

**Ejemplo**:
```cpp
epaper->globalUpdate(BW_monoBuffer, BW_0x00Buffer);
epaper->globalUpdate(BWR_blackBuffer, BWR_redBuffer);
```

#### void COG_powerOff()

Apaga el COG y el conversor DC/DC.

**Secuencia**:
1. Comando Power OFF (0x02)
2. Espera BUSY=HIGH
3. CS=LOW, DC=LOW
4. Delay 150ms
5. RESET=LOW

**Ejemplo**:
```cpp
epaper->COG_powerOff();
```

#### void printGpios()

Imprime la configuración actual de GPIOs.

**Salida**:
```
========================================
Configuración de GPIOs para E-Paper:
----------------------------------------
Panel BUSY  : GPIO25
Panel DC    : GPIO24
Panel RESET : GPIO23
Panel CS    : GPIO8
Panel ON_EXT2: NOT CONNECTED
Panel SPI43 : NOT CONNECTED
Flash CS    : NOT CONNECTED
========================================
```

#### uint8_t hV_HAL_SPI_transfer(uint8_t data)

Transferencia SPI de bajo nivel.

**Parámetros**:
- `data`: Byte a enviar

**Retorna**: Byte recibido

---

### Métodos Protegidos

#### void sendIndexData(uint8_t index, const uint8_t* data, uint32_t len)

Envía un comando (index) seguido de datos por SPI.

**Parámetros**:
- `index`: Código de comando (1 byte)
- `data`: Puntero a datos
- `len`: Longitud de datos en bytes

**Secuencia**:
1. DC=LOW, CS=LOW
2. Enviar index por SPI
3. CS=HIGH, DC=HIGH
4. CS=LOW
5. Enviar len datos por SPI
6. CS=HIGH

#### void softReset()

Ejecuta un soft reset del COG (comando 0x00).

#### void reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5)

Pulso de reset hardware con delays configurables.

**Parámetros**:
- `ms1`: Delay inicial (5ms)
- `ms2`: Delay con RES#=HIGH (5ms)
- `ms3`: Delay con RES#=LOW (10ms)
- `ms4`: Delay con RES#=HIGH (5ms)
- `ms5`: Delay final (5ms)

#### void DCDC_powerOn()

Enciende el conversor DC/DC (comando 0x04).

#### void displayRefresh()

Inicia el refresco de pantalla (comando 0x12).

#### int digitalRead(int gpio)

Lee un GPIO convertido a entero.

---

### Variables Miembro

| Variable | Tipo | Descripción |
|----------|------|-------------|
| pdi_brd | const char* | Nombre de la placa |
| pdi_size | uint16_t | Tamaño de pantalla (0xXX) |
| pdi_cp | uint16_t | Código completo de pantalla |
| image_data_size | uint32_t | Tamaño del buffer de imagen (bytes) |
| register_data | uint8_t[6] | Registros de configuración |
| v_screenSizeV | uint16_t | Resolución vertical (296 para 2.66") |
| v_screenSizeH | uint16_t | Resolución horizontal (152 para 2.66") |

---

## Configuraciones de Placa Predefinidas

### boardRaspberryPi

Configuración para Raspberry Pi estándar.

```cpp
const pins_t boardRaspberryPi = {
    .panelBusy        = 27  // GPIO27 - Pin 13
    .panelDC          = 18  // GPIO18 - Pin 12
    .panelReset       = 17  // GPIO17 - Pin 11
    .panelCS          = 8   // GPIO8  - Pin 24 (CE0)
    .panelON_EXT2     = NOT_CONNECTED
    .panelSPI43_EXT2  = NOT_CONNECTED
    .flashCS          = 22  // GPIO22 - Pin 15
};
```

### boardRaspberryPiZero2W

Configuración para Pi Zero 2W.

```cpp
const pins_t boardRaspberryPiZero2W = {
    .panelBusy        = 25  // GPIO25 - Pin 22
    .panelDC          = 24  // GPIO24 - Pin 18
    .panelReset       = 23  // GPIO23 - Pin 16
    .panelCS          = 8   // GPIO8  - Pin 24 (CE0)
    .panelON_EXT2     = NOT_CONNECTED
    .panelSPI43_EXT2  = NOT_CONNECTED
    .flashCS          = 22  // GPIO22 - Pin 15
};
```

### boardRaspberryPi4

Configuración para Pi 4/5.

```cpp
const pins_t boardRaspberryPi4 = {
    .panelBusy        = 27
    .panelDC          = 18
    .panelReset       = 17
    .panelCS          = 8
    .panelON_EXT2     = NOT_CONNECTED
    .panelSPI43_EXT2  = NOT_CONNECTED
    .flashCS          = 22
};
```

---

## Códigos de Error

| Código | Condición | Acción |
|--------|-----------|--------|
| 0 | Éxito | Ninguna |
| -1 | Error en bcm2835_init | Verificar permisos |
| -1 | Error en gpio_get_fd | Verificar sysfs GPIO |

## Ejemplo de Uso Completo

```cpp
#include <iostream>
#include <memory>
#include <epaper/epaper.h>
#include <epaper/boards.h>
#include <graphics/userImageData.h>
#include <tyme/tyme.h>

int main() {
    // 1. Inicializar hardware
    if (!bcm2835_init()) {
        std::cerr << "Error bcm2835_init()" << std::endl;
        return 1;
    }

    // 2. Crear driver
    auto epaper = std::make_unique<EPAPER::EPD_Driver>(
        EPAPER::eScreen_EPD_266,
        EPAPER::boardRaspberryPiZero2W
    );

    // 3. Inicializar COG
    epaper->COG_initial();
    epaper->printGpios();

    // 4. Actualizaciones
    epaper->globalUpdate(BW_monoBuffer, BW_0x00Buffer);
    TYME::delay(900);
    epaper->globalUpdate(BWR_blackBuffer, BWR_redBuffer);

    // 5. Apagar
    epaper->COG_powerOff();
    bcm2835_close();

    return 0;
}
```
