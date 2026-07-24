# SKILL: Proyectos del Repositorio

## 📋 Índice de Proyectos

Este repositorio contiene múltiples proyectos relacionados con el control de pantallas E-Paper (EPD) en Raspberry Pi. Cada uno representa una iteración o enfoque diferente.

---

## 1. `EPD_Driver_GU_small/` — Driver de Referencia ✅ (funciona)

| Aspecto | Descripción |
|:--------|:------------|
| **Propósito** | Driver mínimo que funciona correctamente. Usado como referencia de oro. |
| **Estado** | ✅ Funcional — probado en Raspberry Pi |
| **Driver** | `EPD_Driver` (clase única, sin namespace) |
| **SPI** | `bcm2835_spi_transfer()` con CS manual toggle entre bytes |
| **GPIO** | `bcm2835_gpio_fsel/write/lev` directo |
| **Registros** | `register_data_sm = {0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d}` |
| **Pines** | BUSY=25, DC=24, RESET=23, CS=27, flashCS=22 |
| **CS toggle** | ✅ Sí — CS sube entre cada byte |
| **SPI Mode** | MODE0 (CPOL=0, CPHA=0) |
| **Clock** | Divider 256 (~976 KHz) |
| **Dependencias** | `libbcm2835-dev` |
| **Archivos clave** | `src/EPD_Driver.cpp`, `src/EPD_Driver.h`, `src/EPD_Configuration.h` |

### Lecciones de `EPD_Driver_GU_small/`

- `sendIndexData()` debe hacer **CS=HIGH entre cada byte**, no mantener CS bajo todo el envío
- `register_data[2..5]` se usan con índices específicos: `[2]`=temperatura, `[3]`=temp activa, `[4..5]`=PSR
- `DCDC_powerOn()` y `displayRefresh()` envían **solo comando, sin datos** (`len=0`)
- `globalUpdate()` **ignora** `data2s` y siempre envía ceros al canal rojo (0x13)

---

## 2. `epaper_success_v1.0.0/` — Primera Versión Limpiada 🟡

| Aspecto | Descripción |
|:--------|:------------|
| **Propósito** | Versión original del proyecto, recién limpiada y corregida |
| **Estado** | 🟡 Corregida pero no probada en Pi |
| **Driver** | `EPAPER::EPD_Driver` (namespace, hereda de `Gpio_t`) |
| **SPI** | `bcm2835_spi_transfer()` con CS toggle |
| **GPIO** | `EPAPER::Gpio_t` (wrapper sobre bcm2835) |
| **Cambios aplicados** | CS toggle, timeouts, constructor simplificado, registros corregidos |

### Archivos eliminados/limpiados

- `Gpio_t::m_gpio_in_fd`, `m_gpio_out` — sysfs legacy
- `v_screenSizeV`, `v_screenSizeH` — no se usaban
- `bool enable{1}` — reemplazado por `bool enabled` recibido en constructor
- Comentarios muertos, #ifdef ENERGIA, etc.

---

## 3. `epaper_success_v1.0.1/` — Versión Actualizada 🟢

| Aspecto | Descripción |
|:--------|:------------|
| **Propósito** | Versión más moderna del driver, con todas las correcciones |
| **Estado** | 🟢 Más completo — incluye `COG_initial()` con índices correctos |
| **Driver** | `EPAPER::EPD_Driver` con `Spi_t`, `Gpio_t` separados |
| **Pines** | BUSY=25, DC=24, RESET=23, CS=27 |
| **main.cpp** | Incluye modo TEST_PINS para verificar cableado GPIO |
| **Característica extra** | `printGpios()` con colores de cables |
| **Nota** | Este es el directorio real en disco. Antes se llamaba `epaper_success_2026_1.0.1/` |

---

## 4. `Master/` — Versión con Fuentes 🟢

| Aspecto | Descripción |
|:--------|:------------|
| **Propósito** | Driver + sistema de fuentes para dibujar texto |
| **Estado** | 🟢 Completo, incluye `EpaperDisplay` y `FontManager` |
| **Capas** | `EPD_Driver` (bajo) → `EpaperDisplay` (alto, dibujo+texto) |
| **Fuentes** | 7 fuentes diferentes integradas |
| **Características** | `drawString()`, `drawCenteredString()`, `clearScreen()` |
| **main.cpp** | Demostración de todas las fuentes en pantalla |

### Arquitectura de Master/

```
main.cpp
    │
    ▼
EpaperDisplay  ← capa ALTA (dibujo, fuentes, buffer de píxeles)
    │
    ▼
EPD_Driver     ← capa BAJA (SPI, GPIO, protocolo COG)
    │
    ▼
bcm2835        ← librería hardware (GPIO + SPI)
```

---

## 5. `scripts_tools/` — Scripts de Utilidad 🛠️

| Script | Propósito |
|:-------|:----------|
| `setGpioPinsOutput.sh` | Configura pines GPIO como salida |
| `uniquePinGpio.sh` | Verifica que no haya pines duplicados |
| `spiSettings.sh` | Muestra/configura parámetros SPI |
| `verifGpios.sh` | Verifica estado de los GPIOs |
| `runGdb.sh` | Ejecuta el binario con GDB |
| `install_libs.sh` | Instala dependencias (bcm2835, qrencode) |
| `menuConfigGpiosSuccess.sh` | Menú interactivo para configurar GPIOs |
| `check_epaper_gpio.sh` | Diagnóstico de GPIOs del display |
| `read_cat_header.sh` | Lee headers de archivos |

---

## 6. `docs/` — Documentación del Proyecto 📚

| Documento | Contenido |
|:----------|:----------|
| `README.md` | Descripción general del proyecto |
| `instalacion.md` | Guía de instalación y dependencias |
| `hardware.md` | Conexiones físicas, pinout y tablas de wiring |
| `arquitectura.md` | Diseño arquitectural, capas y flujo de datos |
| `api.md` | Referencia completa de API y clases |
| `spi-protocolo.md` | Especificación del protocolo SPI |
| `gpio.md` | Sistema de gestión de GPIO |
| `build.md` | Sistema de compilación Makefile |
| `debugging.md` | Depuración con GDB y scripts auxiliares |
| `pinout-display.md` | Pinout del conector FPC del display |
| `qr-codes.md` | Generación de códigos QR |
| `datasheets.md` | Referencia a datasheets y documentos técnicos |
| `aprendizaje-pdf-vs-implementacion.md` | Lecciones aprendidas |
| `CHANGELOG.md` | Historial de cambios |
| `TODO.md` | Tareas pendientes |
| `REMOTE_BUILD.md` | Compilación remota |

---

## 7. `compile_remote.sh` — Deploy Remoto 🚀

Script para compilar y ejecutar en Raspberry Pi remota vía SSH:

```bash
# Secciones disponibles (descomentar la que corresponda):
# origin/                  → compila origin/
# epaper_success/          → compila epaper_success/
# epaper_success_v1.0.1/   → compila epaper_success_v1.0.1/
# gpio_spi_test/           → compila gpio_spi_test/ y ejecuta smoke test

sshpass -p zero ssh pi@raspi.local "cd /home/pi/src/epaper_rpi && \
  git pull && \
  make -C ${FOLDER} clean && \
  make -C ${FOLDER} && \
  make -C ${FOLDER} run"
```

⚠️ **Nota**: `origin/`, `epaper_success/` y `gpio_spi_test/` pueden no existir en el estado actual del disco. Están documentados por referencia histórica.

⚠️ **Seguridad**: La contraseña está en texto plano. El `.gitignore` excluye este archivo.

---

## Mapa de Rutas (Disk State)

```
/mnt/disk/src/raspberry_src/epaper_raspberry/
├── EPD_Driver_GU_small/          # Driver de referencia ✅
├── epaper_success_v1.0.0/        # Versión 1.0.0 limpiada 🟡
├── epaper_success_v1.0.1/        # Versión 1.0.1 actualizada 🟢
├── Master/                       # Versión con fuentes 🟢
├── scripts_tools/                # Scripts de utilidad
├── docs/                         # Documentación
│   ├── epaper_pdf/               # PDFs y datasheets
│   └── ... (skill docs)
├── VERSION                       # 1.4.3
└── compile_remote.sh             # Deploy remoto (gitignored)
```
