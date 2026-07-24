# Instalación y Configuración

## Requisitos del Sistema

### Hardware
- Raspberry Pi 2W, 3B, 4B o Zero 2W
- Pantalla E-Paper compatible (señalada como xE2xxxCSxxx)
- Conexión SPI entre RPi y E-Paper
- Alimentación estable de 3.3V

### Software
- Sistema operativo Raspberry Pi OS (32-bit o 64-bit)
- GCC/G++ con soporte C++20
- Librería bcm2835 (acceso a GPIO y SPI)
- Librería qrencode (generación de QR codes)

## Instalación de Dependencias

### Prerrequisitos del sistema

```bash
sudo apt-get update
sudo apt-get install libraspberrypi-dev -y
```

La librería `libraspberrypi-dev` proporciona:
- Acceso a registros de bajo nivel de la Raspberry Pi
- Control de GPIO, SPI, I2C y UART
- Cabeceras necesarias para compilar drivers personalizados

### Instalación de librerías adicionales

```bash
sudo apt-get install libqrencode-dev -y
```

## Compilación del Proyecto

### Compilación básica

```bash
make
```

Esto generará el binario en `bin/epaper_app`.

### Compilación con objetivo específico

El Makefile soporta dos objetivos adicionales:

```bash
make tx    # Genera bin/epaper_app_tx
make rx    # Genera bin/epaper_app_rx
```

Si no se especifica objetivo, se compila el binario por defecto `epaper_app`.

### Limpieza de artefactos

```bash
make clean
```

Elimina los directorios `obj/`, `bin/` y el contenido de `log/`.

## Permisos y Acceso a GPIO

### Usuario y permisos de superusuario

El programa requiere ejecución con `sudo` porque accede directamente a:

- `/dev/mem` (vía bcm2835)
- `/dev/spidev0.0` (dispositivo SPI)
- Registros de GPIO
- Mapeo de memoria física

### Configuración de GPIO del sistema

Antes de ejecutar la aplicación, se recomienda configurar los pines GPIO del sistema:

```bash
# Configurar todos los GPIO disponibles como salida en bajo
sudo ./bash/menuConfigGpiosSuccess.sh output_low
```

### Verificación de pines

```bash
# Ver estado de todos los GPIO
sudo ./bash/menuConfigGpiosSuccess.sh list

# Información detallada del sistema GPIO
sudo ./bash/menuConfigGpiosSuccess.sh info
```

## Configuración Alternativa: Sysfs GPIO

El proyecto incluye scripts para modificar los permisos de sysfs GPIO:

```bash
# Habilitar acceso a GPIO sin sudo para grupos
sudo ./bash/menuConfigGpiosSuccess.sh enable_gpio
sudo ./bash/menuConfigGpiosSuccess.sh chmod
```

Esto permite que usuarios del grupo `gpio` accedan a:

- `/sys/class/gpio/export`
- `/sys/class/gpio/unexport`
- `/sys/class/gpio/gpio{N}/direction`
- `/sys/class/gpio/gpio{N}/value`

## Verificación de Hardware

### Script de verificación de GPIOs

```bash
sudo ./bash/verifGpios.sh settings
```

Configura los pines críticos del E-Paper:
- 32-bit: 16, 19, 20, 21, 22, 26
- 64-bit: 528, 531, 532, 533, 534, 538

### Configuración de pines individuales

```bash
sudo ./bash/uniquePinGpio.sh <NUMERO_PIN>
```

Este script prueba:
- Dirección IN/OUT
- Valores 0/1
- Edges rising/falling

### Configuración de SPI

```bash
sudo ./bash/spiSettings.sh
```

Configura los pines SPI alternativos (función ALT0):
- 32-bit: GPIO 9, 10, 11 (MISO, MOSI, SCLK)
- 64-bit: GPIO 521, 522, 523

## Solución de Problemas de Instalación

### Error: `bcm2835_init() failed`

Causa: Falta la librería bcm2835 o no se ejecuta con sudo.

Solución:
```bash
sudo apt-get install libraspberrypi-dev -y
sudo ./bin/epaper_app
```

### Error: No se puede abrir `/dev/spidev0.0`

Causa: El módulo spidev no está cargado.

Solución:
```bash
sudo modprobe spi0
sudo dtoverlay spi0-1cs,cs0_pin=8
```

### permisos denegados en `/sys/class/gpio`

Causa: El usuario no tiene permisos de escritura en sysfs.

Solución:
```bash
sudo usermod -aG gpio $USER
```
Luego cerrar sesión y volver a entrar, o ejecutar con sudo.

## Actualización del Sistema

Para actualizar a la última versión del proyecto:

```bash
git pull origin main
make clean
make
sudo make run
```

## Desinstalación

Para eliminar binarios y objetos:

```bash
make clean
```

Para eliminar dependencias del sistema:

```bash
sudo apt-get remove libraspberrypi-dev -y
sudo apt-get remove libqrencode-dev -y
```
