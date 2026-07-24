# Debugging y Depuración

## Visión General

El proyecto incluye múltiples herramientas de depuración: macros condicionales, scripts GDB, verificación de GPIOs y logging estructurado.

## Macros de Debug en Código

### Macros Disponibles

| Macro | Módulo | Función |
|-------|--------|---------|
| `DBG_EPAPER` | epaper.cpp | Trazas del driver EPD |
| `DBG_SPI` | spi.cpp | Trazas de transferencias SPI |
| `DBG_GPIO` | gpio.cpp | Trazas de operaciones GPIO |
| `DEBUG` | epaper.cpp | Mensajes de inicialización |

### Activación

```bash
g++ -std=c++20 ... -DDBG_EPAPER -DDBG_SPI -DDBG_GPIO
```

### Ejemplos de Uso

#### epaper.cpp

```cpp
#ifdef DBG_EPAPER
std::cout << "Enviando dato: " << static_cast<int>(data[i]) << std::endl;
#endif
```

#### spi.cpp

```cpp
#ifdef DBG_SPI
printf("Could not open the Spi device...\r\n");
#endif
```

#### gpio.cpp

```cpp
#ifdef DBG_GPIO
std::cout << "ID: " << gpio.ID 
          << ", GPIO: " << gpio.gpio 
          << ", Direction: " << gpio.dir 
          << ", Value: " << gpio.value << "\n";
#endif
```

## Scripts de Utilidad

### runGdb.sh

Script que ejecuta GDB con reglas preconfiguradas.

```bash
sudo gdb -x $HOME/src/epaper/rules/rulesEpaper.gdb ./bin/epaper_app
```

**Nota**: La ruta `$HOME/src/epaper/rules/` debe ajustarse a la ubicación real del proyecto.

**Uso**:
```bash
cd /path/to/epaper_success_2026_1.0.0
sudo ./bash/runGdb.sh
```

### rulesEpaper.gdb

Archivo de configuración de GDB.

**Breakpoints preconfigurados**:

```gdb
break GPIO::Gpio_t::file_open_and_write_value
break GPIO::Gpio_t::gpio_export
break GPIO::Gpio_t::gpio_unexport
break GPIO::Gpio_t::gpio_set_direction
break GPIO::Gpio_t::gpio_set_value
break GPIO::Gpio_t::gpio_set_edge
break GPIO::Gpio_t::gpio_get_fd_to_value

break GPIO::Gpio_t::CloseGpios
break GPIO::Gpio_t::pinMode
break GPIO::Gpio_t::digitalWrite
break GPIO::Gpio_t::digitalRead

break EPAPER::EPD_Driver::COG_initial
break EPAPER::EPD_Driver::sendIndexData

break SPI::Spi_t::Spi_t
break SPI::Spi_t::init
break SPI::Spi_t::Transfer1bytes
break SPI::Spi_t::settings_spi

break GPIO::Gpio_t::settings
```

**Comandos automáticos**:

```gdb
command 1
    print pin
    print str_v
    print fileTmp.is_open()
end

commands
    continue
end

set listsize 10
```

**Notas**:
- `command 1`: Se ejecuta en el primer breakpoint, imprime variables locales
- `commands`: Se ejecuta tras cualquier breakpoint, continúa automáticamente
- `set listsize 10`: Muestra 10 líneas de código fuente al detenerse

### verifGpios.sh

Verifica la configuración de GPIOs específicos.

```bash
sudo ./bash/verifGpios.sh settings
```

Configura:
- 32-bit: pines 16, 19, 20, 21, 22, 26
- 64-bit: pines 528, 531, 532, 533, 534, 538

### menuConfigGpiosSuccess.sh

Menú interactivo para gestión de GPIOs.

**Comandos**:

| Comando | Función |
|---------|---------|
| `enable_gpio` | Habilita permisos de GPIO para grupo `gpio` |
| `set` | Exporta todos los GPIOs al sistema |
| `chmod` | Cambia propietario a grupo `gpio`, permisos 660 |
| `debbug` | Muestra información de depuración de GPIOs |
| `info` | Muestra gpioinfo y lista /sys/class/gpio |
| `list` | Lista estado de GPIOs con raspi-gpio |
| `output_low` | Configura GPIOs 0-27 como salida en bajo (excluye reservados) |

**Ejemplos**:

```bash
# Listar estado de GPIOs
sudo ./bash/menuConfigGpiosSuccess.sh list

# Configurar GPIOs como salida baja
sudo ./bash/menuConfigGpiosSuccess.sh output_low
```

## Depuración de Hardware

### Verificación de Pines SPI

```bash
# Configurar pines SPI en función alternativa ALT0
sudo ./bash/spiSettings.sh
```

Verifica:
- GPIO 9,10,11 (32-bit) en función SPI0

### Verificación Individual de Pines

```bash
sudo ./bash/uniquePinGpio.sh 22
```

Tests realizados:
1. Configura como entrada
2. Configura como salida
3. Escribe 1
4. Escribe 0
5. Lee valor
6. Configura edge rising
7. Configura edge falling

## Tracing con strace/ltrace

### strace

```bash
sudo strace -e trace=ioctl ./bin/epaper_app
```

Muestra todas las llamadas `ioctl()` al dispositivo SPI.

### ltrace

```bash
sudo ltrace ./bin/epaper_app
```

Muestra llamadas a librerías (no muy útil por uso directo de syscalls).

## Análisis de Memoria

### Valgrind (si aplica)

```bash
sudo valgrind --leak-check=full ./bin/epaper_app
```

**Limitaciones**:
- bcm2835 usa memoria mapeada directamente
- Valgrind puede reportar falsos positivos en acceso a registros

### /proc/pid/maps

```bash
# Durante ejecución del programa en otra terminal
sudo cat /proc/$(pidof epaper_app)/maps
```

## Depuración de SPI

### Logic Analyzer

Usar un analizador lógico (Saleae, DSView) en:
- SCLK (GPIO11, Pin 23)
- MOSI (GPIO10, Pin 19)
- CS (GPIO8, Pin 24)
- DC (GPIO24, Pin 18)

**Decodificación esperada (Modo 0, MSB)**:

```
Comando 0x10:
  CS=LOW, DC=LOW
  SCLK: ───┐ ┌── ┌── ┌── ┌── ┌── ┌── ┌──
          └─┘   └─┘   └─┘   └─┘   └─┘   └─┘
  MOSI:  1  0  0  0  1  0  0  0  (0x10)

Datos: 5,624 bytes a 1.6 MHz (~28 ms)
```

### spidev_test

```bash
# Prueba SPI nativo
sudo /opt/vc/src/hello_pi/hello_spi/hello_spi.bin
```

O compilar spidev_test:
```bash
gcc -o spidev_test spidev_test.c
sudo ./spidev_test
```

## Depuración de GPIO

### raspi-gpio

```bash
# Leer pines específicos
sudo raspi-gpio get 25
sudo raspi-gpio get 24

# Configurar temporalmente
sudo raspi-gpio set 25 op dh  # Salida, HIGH
sudo raspi-gpio set 24 ip pu  # Entrada, pull-up

# Leer todos los GPIOs
sudo raspi-gpio get
```

Salida esperada:
```
GPIO 25: level=1 fsel=1 alt=0 func=OUTPUT
GPIO 24: level=0 fsel=0 alt=0 func=INPUT
```

### gpioinfo (libgpiod)

```bash
sudo apt-get install gpiod -y
gpioinfo
```

Muestra:
- Número de línea
- Nombre
- Dirección (in/out)
- Estado (active-low/high)

### /sys/kernel/debug/gpio

```bash
# Ver estado en tiempo real
sudo cat /sys/kernel/debug/gpio
```

Salida:
```
gpiochip0: GPIOs 0-53, parent: platform/3f200000.gpio, 3f200000.gpio:
 gpio-25 (sysfs)            out lo
 gpio-24 (sysfs)            in  hi
```

## Logging Estructurado

### Redirección a Archivo

```bash
sudo ./bin/epaper_app > log/epaper.log 2>&1
```

### Timestamp

Agregar en main.cpp:

```cpp
#include <chrono>
#include <iomanip>

std::cout << "[" << std::chrono::system_clock::now().time_since_epoch().count() 
          << "] Iniciando E-Paper..." << std::endl;
```

## Tests de Integración

### Verificación Post-Compilación

```bash
# Verificar binario
file bin/epaper_app

# Verificar linking
ldd bin/epaper_app

# Verificar tamaño
ls -lh bin/epaper_app
```

Salida esperada:
```
bin/epaper_app: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV), dynamically linked
	interpreter /lib/ld-linux-aarch64.so.1
	...
```

### Verificación de GPIOs en Runtime

```bash
# Antes de ejecutar
sudo raspi-gpio get 25,24,23,8,22

# Ejecutar aplicación
sudo ./bin/epaper_app

# Después de ejecutar
sudo raspi-gpio get 25,24,23,8,22
```

Verificar que los pines estén en los estados correctos durante la operación.

## Common Issues

### Programa cuelga en BUSY

**Causa**: COG no responde (hardware desconectado o alimentación insuficiente)

**Solución**:
1. Verificar conexiones físicas
2. Medir voltaje de alimentación (debe ser 3.3V ±10%)
3. Verificar señal RESET con osciloscopio
4. Agregar timeout en código:

```cpp
uint32_t timeout = 0;
while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH) {
    if (++timeout > 100000) {
        std::cerr << "Timeout esperando BUSY" << std::endl;
        return;
    }
    delay_ms(1);
}
```

### SPI no responde

**Causa**: /dev/spidev0.0 no existe

**Diagnóstico**:
```bash
ls -la /dev/spidev*
dmesg | grep spi
```

**Solución**:
```bash
sudo modprobe spi0
sudo dtoverlay spi0-1cs
```

### Corrupción de Datos

**Causa**: Velocidad SPI excesiva o ruido eléctrico

**Solución**:
1. Reducir velocidad en `spi.h`: `SPI_SPEED 800000`
2. Usar cables más cortos (< 15 cm)
3. Agregar capacitor de desacople (100nF) cerca del COG

### bcm2835_init() falla

**Causa**: Permisos insuficientes o librería no instalada

**Diagnóstico**:
```bash
ldconfig -p | grep bcm2835
```

**Solución**:
```bash
sudo apt-get install --reinstall libraspberrypi-dev
```

## Mejoras Futuras

1. **Log levels**: Implementar niveles DEBUG, INFO, WARN, ERROR
2. **Structured logging**: JSON o syslog para integración con sistemas de monitoreo
3. **Unit tests**: Tests con Google Test o Catch2 para GPIO y SPI
4. **Watchdog**: Timeout automático en BUSY polling
5. **Remote debugging**: SSH forwarding de GDB para depuración remota
