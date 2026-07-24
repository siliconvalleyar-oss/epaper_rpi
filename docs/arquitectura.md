# Arquitectura del Software

## Visión General

El proyecto sigue una arquitectura en capas orientada a objetos con namespaces, diseñada para abstraer el hardware específico de la Raspberry Pi y proporcionar una API limpia para el control de pantallas E-Paper.

## Diagrama de Capas

```
┌─────────────────────────────────────────────┐
│          main.cpp (Aplicación)              │
│    Inicializa bcm2835, coordina updates     │
├─────────────────────────────────────────────┤
│        EPAPER::EPD_Driver                   │
│   Control de alto nivel: COG, frames, power │
├─────────────┬─────────────┬─────────────────┤
│  SPI::Spi_t │ GPIO::Gpio_t│ TYME::delay     │
│  (spi.cpp)  │ (gpio.cpp)  │ (tyme.cpp)      │
├─────────────┴─────────────┴─────────────────┤
│          bcm2835 (C)                        │
│  Acceso directo a registros del SoC        │
├─────────────────────────────────────────────┤
│   Hardware: RPi SPI0 + GPIO + E-Paper COG  │
└─────────────────────────────────────────────┘

Capa de Datos:
├── graphics/userImageData.h (Buffers de imagen)
├── qr/qr_gen.cpp (Generación QR)
└── app/config.h (Configuración de compilación)
```

## Módulos Principales

### 1. EPAPER (Driver Principal)

**Archivos**: `epaper.h`, `epaper.cpp`, `boards.h`

Responsabilidades:
- Inicialización del COG (Chip-on-Glass)
- Secuencias de power-on y power-off
- Transferencia de frames de imagen
- Control de refresco (DCDC power-on, Display Refresh)
- Gestión de registros de control del EPD

Clases y estructuras clave:

```cpp
namespace EPAPER {
    struct pins_t;               // Configuración de pines GPIO
    class Spi_t;                  // Wrapper SPI para bcm2835
    class Gpio_t;                 // Wrapper GPIO para bcm2835
    struct EPD_Driver;            // Driver principal
}
```

### 2. SPI (Bus de Comunicación)

**Archivos**: `spi.h`, `spi.cpp`

Responsabilidades:
- Inicialización del dispositivo SPI `/dev/spidev0.0`
- Transferencias de 1, 2 y 3 bytes
- Configuración de velocidad, modo y bits por palabra
- Gestión de buffers TX/RX

Características:
- Implementación dual: bcm2835 y spidev nativo
- Buffer de 256 bytes (LARGE_SECTOR_SIZE)
- Velocidad configurable: 1.6 MHz por defecto

### 3. GPIO (Entrada/Salida Digital)

**Archivos**: `gpio.h`, `gpio.cpp`

Responsabilidades:
- Configuración de dirección de pines
- Lectura/escritura digital
- Gestión de colecciones de GPIOs
- Abstracción sysfs y bcm2835

Características:
- Mapa por ID y por número de pin
- Soporte para NOT_CONNECTED (0xFF)
- Cierre automático al destruir el objeto

### 4. TYME (Tiempo y Delays)

**Archivos**: `tyme.h`, `tyme.cpp`

Responsabilidades:
- Delays en milisegundos, microsegundos y segundos
- Wrapper de `bcm2835_delayMicroseconds`
- Abstracción POSIX (usleep/sleep)

### 5. QR (Generación de Códigos QR)

**Archivos**: `qr_gen.h`, `qr_gen.cpp`

Responsabilidades:
- Generación de códigos QR mediante libqrencode
- Escalado de píxeles
- Centrado en buffer de imagen
- Codificación de datos de red WiFi

### 6. Graphics (Datos de Imagen)

**Archivos**: `userImageData.h`, múltiples buffers

Responsabilidades:
- Buffers de imagen precompilados
- Distinctivos para 2.13" y 2.66"
- Modos BW (blanco/negro) y BWR (blanco/negro/rojo)

### 7. App (Configuración)

**Archivos**: `config.h`

Responsabilidades:
- Detección de arquitectura 32/64 bits
- Definición de CPU_32_BITS / CPU_64_BITS
- Selección de configuración de placa en compile-time

## Flujo de Ejecución

### Ciclo de Vida Principal

```
1. bcm2835_init()
   └── Inicializa librería de bajo nivel
       └── Mapea memoria física de GPIO/SPI

2. EPD_Driver::EPD_Driver(screen, board)
   ├── Configura tipo de pantalla (size, diagonal)
   ├── Calcula image_data_size
   └── Carga registros de configuración

3. EPD_Driver::COG_initial()
   ├── Configura pines GPIO (modo, estado inicial)
   ├── Secuencia de reset hardware
   ├── Soft reset COG
   ├── Configuración de temperatura y PSR
   └── Establece dimensiones de pantalla

4. EPD_Driver::globalUpdate(data1, data2)
   ├── sendIndexData(0x10, data1) - Frame 1
   ├── sendIndexData(0x13, data2) - Frame 2
   ├── DCDC_powerOn() - Enciende conversor DC/DC
   └── displayRefresh() - Refresco de pantalla

5. EPD_Driver::COG_powerOff()
   ├── Apaga DC/DC
   ├── Espera BUSY
   └── Secuencia de apagado

6. bcm2835_close()
   └── Libera memoria y recursos
```

## Secuencia de Comandos SPI

### Inicialización COG

```
1. 0x00, 0x0E  -> Soft Reset
2. 0xE5, <temp> -> Configuración de temperatura
3. 0xE0, <temp> -> Temperatura activa
4. 0x00, <PSR>  -> Configuración de Panel Setting Register
```

### Actualización Global

```
1. 0x10, <5,624 bytes> -> First Frame (Black/previous)
2. 0x13, <5,624 bytes> -> Second Frame (Red/new)
3. 0x04, 0x00          -> Power ON (DC/DC)
4. Esperar BUSY=HIGH
5. 0x12, 0x00          -> Display Refresh
6. Esperar BUSY=HIGH
```

### Apagado

```
1. 0x02, 0x00          -> Power OFF (DC/DC)
2. Esperar BUSY=HIGH
3. CS = LOW, DC = LOW
4. Delay 150ms
5. RESET = LOW
```

## Gestión de Memoria

### Buffers de Imagen

Los buffers están definidos como `unsigned char const` en archivos `.cpp` separados:

```cpp
// image_266_296x152_BW.cpp
unsigned char const image_266_296x152_BW_mono[5624];
unsigned char const image_266_296x152_BW_0x00[5624];
```

Estos son linkeados vía `userImageData.h`:

```cpp
#define BW_monoBuffer (uint8_t *)& image_266_296x152_BW_mono
#define BW_0x00Buffer (uint8_t *)& image_266_296x152_BW_0x00
```

### Stack vs Heap

- `EPD_Driver` se crea en heap (`std::unique_ptr`)
- SPi_t y Gpio_t se delegan a miembros `std::unique_ptr`
- Buffers de imagen residen en data segment (estático)

## Concurrencia

El diseño es **single-threaded** y **síncrono**:

- No hay multithreading
- Las operaciones SPI son bloqueantes
- El estado BUSY se consulta de forma polling

## Depuración y Tracing

El código incluye macros condicionales:

- `DBG_EPAPER`: Trazas del driver EPD
- `DBG_SPI`: Trazas de transferencias SPI
- `DBG_GPIO`: Trazas de operaciones GPIO

Activar con:
```bash
g++ -std=c++20 ... -DDBG_EPAPER -DDBG_SPI -DDBG_GPIO
```

## Patrones de Diseño Utilizados

### RAII (Resource Acquisition Is Initialization)

```cpp
Gpio_t::Gpio_t(bool& status) {
    if (!bcm2835_init()) m_state = false;
}
~Gpio_t() {
    bcm2835_close();
}
```

### Strategy Pattern (Configuración de Placa)

```cpp
const pins_t& board = boardRaspberryPiZero2W;
auto epaper = std::make_unique<EPD_Driver>(eScreen_EPD_266, board);
```

### Template Method (Compilación de Librerías)

El Makefile usa `define/endef` y `eval/call` para generar reglas de compilación genéricas:

```makefile
define compile_template
$(OBJ_DIR)/%.o: $(LIB_DIR)/$(1)/%.cpp | $(OBJ_DIR)
    ...
endef
```

## Extensibilidad

### Agregar Nueva Pantalla

1. Agregar define en `epaper.h`:
   ```cpp
   #define eScreen_EPD_XXX (uint32_t)0xXX00
   ```

2. Agregar caso en `epaper.cpp`:
   ```cpp
   case 0xXX:
       screenSizeV = ...;
       screenSizeH = ...;
       break;
   ```

3. Crear buffers de imagen en `graphics/`

### Agregar Nueva Placa

1. Agregar configuración en `boards.h`:
   ```cpp
   const pins_t boardMiPlaca = { ... };
   ```

## Limitaciones Conocidas

1. **Solo pantalla 2.66" activa por defecto**: El código tiene hardcodeo de `v_screenSizeV = 296; v_screenSizeH = 152;`
2. **No hay interrupciones**: El polling de BUSY consume CPU
3. **Buffer único**: No soporta double-buffering ni partial updates
4. **Temperatura fija**: Registro 0xE5 hardcodeado a 25°C
5. **No hay manejo de errores robusto**: Muchas funciones retornan void y asumen éxito
