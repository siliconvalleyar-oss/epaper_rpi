# SKILL: Depuración de SPI y GPIO

## 🔍 Metodología de Debug

Cuando el display E-Paper no funciona, el problema suele estar en una de estas capas (en orden):

```
1. Pines GPIO cableados incorrectamente  ← 80% de los problemas
2. Configuración SPI incorrecta           ← 15% de los problemas
3. Protocolo COG incorrecto               ← 5% de los problemas
```

---

## 1️⃣ Test de Pines GPIO

### Método rápido: LEDs

Conecta un LED con resistencia (220Ω-1kΩ) en serie a cada pin de control:
- Si el LED **no enciende** → el GPIO está mal configurado o el cable está mal
- Si el LED **enciende pero parpadea raro** → posible conflicto de pines

### Test automático (modo TEST_PINS)

En `epaper_success_v1.0.1/src/main.cpp`, descomenta `#define TEST_PINS`:

```cpp
// Descomentar para ejecutar test de pines GPIO
#define TEST_PINS
```

Esto ejecuta 4 pruebas:
1. **Test individual**: Cada pin se pone HIGH/LOW individualmente
2. **Test secuencial**: Los 4 pines se activan uno tras otro
3. **Test simultáneo**: Todos los pines juntos HIGH y LOW
4. **Test carrusel**: Uno se enciende mientras los otros se apagan

### Verificación con multímetro

| Medición | Valor esperado |
|:---------|:--------------:|
| Voltaje pin HIGH (con LED) | ~2.5-3.0V |
| Voltaje pin HIGH (sin carga) | ~3.3V |
| Voltaje pin LOW | ~0V |
| BUSY (display ocioso) | ~3.3V (HIGH) |
| BUSY (display actualizando) | ~0V (LOW) |

---

## 2️⃣ Diagnóstico SPI

### Verificar que SPI está habilitado

```bash
# En la Raspberry Pi:
ls -la /dev/spidev0.0
# Si no existe → habilitar SPI con raspi-config

# O ver módulos cargados:
lsmod | grep spi
```

### Probar SPI con herramienta loopback

```bash
# Conectar MOSI (Pin 19) a MISO (Pin 21) con un jumper
# Luego:
sudo ./bin/epaper_app    # el programa debe ejecutarse sin errores
```

### Errores comunes SPI

| Síntoma | Causa probable |
|:--------|:---------------|
| SPI transfiere pero pantalla no muestra nada | CS no togglea entre bytes |
| La mitad de la imagen se ve bien | SPI Mode incorrecto (MODE1 en vez de MODE0) |
| Imagen corrupta parcialmente | Velocidad SPI demasiado alta |
| No hay comunicación SPI | bcm2835_spi_begin() no se llamó |

### Habilitar debug SPI

En `epaper_success_v1.0.1/libs/epaper/epaper.h`:
```cpp
#define DBG_EPAPER   // Activar trazas del driver
```

O en `libs/spi/spi.h` (para el SPI por ioctl):
```cpp
#define DBG_SPI      // Activar trazas SPI
```

---

## 3️⃣ Diagnóstico del COG

### La señal BUSY

BUSY es la señal más importante para diagnosticar:

| Estado de BUSY | Significado |
|:--------------:|:------------|
| **HIGH** (3.3V) | COG listo para recibir comandos |
| **LOW** (0V) | COG ocupado (procesando) |
| **Pulsa LOW por ms** | Respondiendo a un comando (normal) |
| **Siempre LOW** | COG no inicializado, o RESET mal conectado |
| **Siempre HIGH** | COG no responde, o BUSY mal cableado |

### Secuencia de reset manual

```bash
# Configurar pines como salida
gpioset gpiochip0 23=1   # RESET HIGH
sleep 0.005
gpioset gpiochip0 23=0   # RESET LOW
sleep 0.010
gpioset gpiochip0 23=1   # RESET HIGH
sleep 0.005

# Verificar BUSY
gpioget gpiochip0 25     # Debe ser 1 (HIGH)
```

### Comandos COG básicos (para debug manual)

```bash
# 1. Soft Reset (0x00 con data=0x0E)
# Esto se hace por SPI, no por GPIO

# 2. Verificar que el COG responde:
# Después de soft reset, BUSY debe ir LOW por ~10ms y volver HIGH
```

---

## 4️⃣ Errores Comunes y Soluciones

### 🔴 El display no muestra nada

```
1. ¿bcm2835_init() retornó true?      → Verificar permisos (sudo)
2. ¿SPI habilitado?                     → raspi-config
3. ¿Pines conectados correctamente?    → Verificar tabla de colores
4. ¿CS togglea entre bytes?            → Ver sendIndexData()
5. ¿SPI_MODE0?                         → Ver bcm2835_spi_setDataMode()
```

### 🟡 Muestra imagen corrupta

```
1. ¿Velocidad SPI correcta?            → Divider 256 (~976 KHz)
2. ¿Bit Order MSB First?               → bcm2835_spi_setBitOrder()
3. ¿Frame size correcto?               → Ver image_data_size
4. ¿Formato de buffer correcto?        → Ver organización de bits
```

### 🟢 Muestra imagen previa (no se actualiza)

```
1. ¿Se llama a DCDC_powerOn()?         → Comando 0x04
2. ¿Se llama a displayRefresh()?        → Comando 0x12
3. ¿Se espera BUSY=HIGH entre pasos?   → Timeouts implementados?
```

---

## 5️⃣ Scripts de Diagnóstico

### `bash/verifGpios.sh`
Verifica el estado de todos los GPIOs usados por el display:

```bash
./bash/verifGpios.sh
```

### `bash/spiSettings.sh`
Muestra la configuración actual del SPI:

```bash
./bash/spiSettings.sh
```

### `scripts_tools/check_epaper_gpio.sh`
Diagnóstico completo de GPIOs del display:

```bash
./scripts_tools/check_epaper_gpio.sh
```

---

## 6️⃣ Debug con GDB

```bash
# Ejecutar con GDB
gdb --args ./bin/epaper_app

# Breakpoints útiles:
b EPAPER::EPD_Driver::COG_initial
b EPAPER::EPD_Driver::sendIndexData
b EPAPER::EPD_Driver::globalUpdate

# Ver pines:
p pin_cfg_epaper

# Ver registros COG:
p/x register_data

# Step a través de sendIndexData para ver CS toggle:
n  # next instruction — verificar que CS sube entre bytes
```

También hay reglas GDB preconfiguradas:
```bash
gdb -x rules/rulesEpaper.gdb ./bin/epaper_app
```
