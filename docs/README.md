# E-Paper Success 2026 - Documentación

## Descripción del Proyecto

**E-Paper Success** es una aplicación de control de pantallas electrónicas (e-Paper/EPD) para Raspberry Pi, desarrollada en C++20. El proyecto implementa un driver completo para pantallas E-Paper de la serie xE2xxxCSxxx, compatible con múltiples tamaños, tanto monocromáticas (BW) como bicolor blanco/rojo/negro (BWR).

- **Versión**: 1.0.0 (codificación en código: 1.3.0)
- **Autor**: Lio
- **Licencia**: GNU
- **Hardware objetivo**: Raspberry Pi 2W, Pi 4, Pi Zero 2W
- **Compilador**: g++
- **Estandar**: C++20
- **Dependencias**: librería bcm2835, libqrencode

## Tabla de Contenidos

1. [Introducción](#introducción)
2. [Características](#características)
3. [Compatibilidad de Hardware](#compatibilidad-de-hardware)
4. [Rápida Inicio](#rápida-inicio)
5. [Estructura del Proyecto](#estructura-del-proyecto)
6. [Documentación Disponible](#documentación-disponible)

## Introducción

Este software permite controlar pantallas E-Paper (también conocidas como E Ink o EPD) conectadas a través del bus SPI de Raspberry Pi. La aplicación proporciona:

- Inicialización y control del driver COG (Chip-on-Glass)
- Actualizaciones globales de pantalla (Global Update)
- Soporte para pantallas monocromáticas (BW) y bicolor (BWR)
- Generación dinámica de códigos QR
- Gestión de GPIO y SPI mediante la librería bcm2835
- Herramientas de depuración integradas con GDB

## Características

- **Multi-pantalla**: Soporta resoluciones desde 1.54" hasta 4.37"
- **Actualización Global**: Protocolo completo de refresco COG (DC/DC, Display Refresh)
- **Imágenes predefinidas**: Buffers de imagen incluidos para 2.13" y 2.66"
- **QR Code Generator**: Generador de códigos QR integrado con escalado configurable
- **Abstracción de GPIO**: Wrapper completo de GPIO con detección de arquitectura (32/64 bits)
- **SPI nativo**: Acceso directo a `/dev/spidev0.0` y modo compatible bcm2835
- **Debugging**: Scripts GDB con breakpoints preconfigurados y visualización de registros

## Compatibilidad de Hardware

### Modelos soportados

| Modelo | Arquitectura | GPIO BCM utilizados |
|--------|-------------|---------------------|
| Raspberry Pi 2W | 64-bit (ARMv7/aarch64) | 25, 24, 23, 8, 22 |
| Raspberry Pi 4B | 64-bit | 27, 18, 17, 8, 22 |
| Raspberry Pi Zero 2W | 64-bit | 25, 24, 23, 8, 22 |

### Pantallas soportadas

| Diagonal | Resolución (VxH) | Código | Modo |
|----------|------------------|--------|------|
| 1.54" | 152 x 152 | 0x1509 | BW/BWR |
| 2.13" | 212 x 104 | 0x2100 | BW/BWR |
| 2.66" | 296 x 152 | 0x2600 | BW/BWR |
| 2.71" | 264 x 176 | 0x2700 | BW/BWR |
| 2.87" | 296 x 128 | 0x2800 | BW/BWR |
| 3.70" | 416 x 240 | 0x3700 | BW/BWR |
| 4.17" | 300 x 400 | 0x4100 | BW/BWR |
| 4.37" | 480 x 176 | 0x430C | BW/BWR |

## Rápida Inicio

### Prerrequisitos

```bash
sudo apt-get install libraspberrypi-dev -y
```

### Compilación

```bash
make
```

Para objetivo específico:

```bash
make tx    # Compila como transmisor (epaper_app_tx)
make rx    # Compila como receptor (epaper_app_rx)
```

### Ejecución

```bash
sudo ./bin/epaper_app
```

El binario requiere permisos de superusuario para acceder a los registros del sistema y a la memoria mapeada de bcm2835.

## Estructura del Proyecto

```
epaper_success_2026_1.0.0/
├── src/
│   └── main.cpp                 # Punto de entrada de la aplicación
├── libs/
│   ├── epaper/
│   │   ├── epaper.h             # Driver principal EPD
│   │   └── epaper.cpp
│   ├── boards.h                 # Configuraciones de placa
│   ├── gpio/
│   │   └── gpio.h/.cpp          # Abstracción GPIO
│   ├── spi/
│   │   └── spi.h/.cpp           # Controlador SPI
│   ├── tyme/
│   │   └── tyme.h/.cpp          # Delays y timing
│   ├── graphics/
│   │   └── userImageData.h      # Buffers de imagen compilados
│   ├── qr/
│   │   └── qr_gen.h/.cpp        # Generador QR Code
│   ├── app/
│   │   └── config.h             # Configuración de arquitectura
│   └── work/
│       └── work.h               # Clase base abstracta Work_t
├── bash/                        # Scripts de utilidad
├── rules/                       # Reglas GDB
├── Makefile                     # Sistema de compilación
├── .gitignore
└── README.md
```

## Documentación Disponível

| Documento | Descripción |
|-----------|-------------|
| [instalacion.md](instalacion.md) | Guía completa de instalación y dependencias |
| [hardware.md](hardware.md) | Conexiones físicas, pinout y tablas de wiring |
| [arquitectura.md](arquitectura.md) | Diseño arquitectural, capas y flujo de datos |
| [api.md](api.md) | Referencia completa de API y clases |
| [spi-protocolo.md](spi-protocolo.md) | Especificación del protocolo SPI |
| [gpio.md](gpio.md) | Sistema de gestión de GPIO |
| [graficos.md](graficos.md) | Sistema de gráficos y buffers de imagen |
| [qr-codes.md](qr-codes.md) | Generación de códigos QR |
| [build.md](build.md) | Sistema de compilación Makefile |
| [debugging.md](debugging.md) | Depuración con GDB y scripts auxiliares |
