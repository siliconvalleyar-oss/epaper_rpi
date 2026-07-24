# SKILL: Reparaciones — Bugs Encontrados y Corregidos

Este documento registra todos los bugs descubiertos durante el desarrollo y las correcciones aplicadas, para referencia futura.

---

## Bug #1: SPI Mode Incorrecto (MODE1 vs MODE0)

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `epaper_success/libs/epaper.cpp` → `Spi_t` |
| **Síntoma** | Display no respondía a comandos SPI |
| **Causa** | `bcm2835_spi_setDataMode(BCM2835_SPI_MODE1)` — CPOL=0, CPHA=1 |
| **Solución** | Cambiar a `BCM2835_SPI_MODE0` (CPOL=0, CPHA=0) |
| **Referencia** | `EPD_Driver_GU_small` usa MODE0 |
| **Commit** | Aplicado en `epaper_success_v1.0.0` y `epaper_success_v1.0.1` |

### Por qué MODE0:

```
SPI MODE0:   ___     ___     ___     ___
SCLK          | |___| |___| |___| |___|
              
MOSI         X<bit7>X<bit6>X<bit5>X     ← sampleado en flanco de subida
             
El COG samplea datos en el flanco de subida (CPHA=0) con clock idle en LOW (CPOL=0)
```

---

## Bug #2: CS sin Toggle entre Bytes

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `sendIndexData()` en todas las versiones excepto `EPD_Driver_GU_small` |
| **Síntoma** | Display no mostraba nada, o mostraba datos corruptos |
| **Causa** | CS se mantenía LOW durante toda la transacción de múltiples bytes |
| **Solución** | Toggle CS = HIGH entre cada byte |
| **Referencia** | `EPD_Driver_GU_small` hace CS=HIGH después de cada byte |

### El cambio:

```cpp
// ❌ ANTES (no funciona):
void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    digitalWrite(DC, LOW);
    digitalWrite(CS, LOW);           // CS baja una vez
    hV_HAL_SPI_transfer(index);
    for (uint32_t i = 0; i < len; i++) {
        digitalWrite(DC, HIGH);
        hV_HAL_SPI_transfer(data[i]); // CS sigue LOW aquí
    }
    digitalWrite(CS, HIGH);          // CS sube al final
}

// ✅ DESPUÉS (funciona):
void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    digitalWrite(DC, LOW);
    digitalWrite(CS, LOW);
    hV_HAL_SPI_transfer(index);
    digitalWrite(CS, HIGH);          // CS sube después del comando

    for (uint32_t i = 0; i < len; i++) {
        digitalWrite(DC, HIGH);
        digitalWrite(CS, LOW);
        hV_HAL_SPI_transfer(data[i]);
        digitalWrite(CS, HIGH);      // CS sube después de cada byte
    }
}
```

**Razón**: El COG espera un flanco de CS para sincronizarse. Sin el toggle, el COG pierde la cuenta de los bytes y corrompe el frame buffer interno.

---

## Bug #3: Índices Incorrectos de `register_data`

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `COG_initial()` en varias versiones |
| **Síntoma** | Display se inicializaba pero mostraba comportamiento errático |
| **Causa** | Usar `register_data[2]` para el comando 0xE0 y `register_data[3]` para 0x00 |
| **Solución** | Usar `register_data[3]` para 0xE0 y `register_data[4..5]` para 0x00 |
| **Referencia** | `EPD_Driver_GU_small` usa `[2]`, `[3]`, `[4..5]` |

### El cambio:

```cpp
// register_data_sm = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d }
//                      [0]    [1]    [2]    [3]    [4]    [5]

// ❌ ANTES (incorrecto):
sendIndexData(0xe5, &register_data[2], 1);  // [2]=0x19 ✅ (Input Temp)
sendIndexData(0xe0, &register_data[2], 1);  // [2]=0x19 ❌ (debe ser 0x02)
sendIndexData(0x00, &register_data[3], 2);  // [3]=0x02, [4]=0xcf ❌ (debe ser 0xcf,0x8d)

// ✅ DESPUÉS (correcto):
sendIndexData(0xe5, &register_data[2], 1);  // [2]=0x19 ✅ Input Temperature
sendIndexData(0xe0, &register_data[3], 1);  // [3]=0x02 ✅ Active Temperature
sendIndexData(0x00, &register_data[4], 2);  // [4..5]=0xcf,0x8d ✅ PSR
```

### Tabla de valores correctos:

| Comando | Índice | Valor | Significado |
|:-------:|:------:|:-----:|:------------|
| 0xE5 | `[2]` | 0x19 | Temperatura de entrada (25°C) |
| 0xE0 | `[3]` | 0x02 | Temperatura activa |
| 0x00 (PSR) | `[4..5]` | 0xcf, 0x8d | Panel Setting Register |

---

## Bug #4: Pines GPIO Incorrectos en `boards.h`

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `boards.h` en `origin/` y `epaper_success/` |
| **Síntoma** | Display no respondía porque los pines no coincidían con el cableado |
| **Causa** | `boardRaspberryPiZero2W` tenía BUSY=7 (GPIO7=Pin26), DC=8, RESET=25 |
| **Solución** | Corregir a BUSY=25 (Pin22), DC=24 (Pin18), RESET=23 (Pin16), CS=27 (Pin13) |

### Tabla de pines CORRECTA (todas las configuraciones funcionales):

| Señal | GPIO | Pin Físico | Color Cable | Dirección |
|:------|:----:|:----------:|:-----------:|:---------:|
| BUSY | 25 | 22 | Rojo | INPUT |
| D/C | 24 | 18 | Naranja | OUTPUT |
| RESET | 23 | 16 | Amarillo | OUTPUT |
| CS | 27 | 13 | Gris | OUTPUT |
| flashCS | 22 | 15 | Violeta | OUTPUT |
| MOSI | 10 | 19 | Azul | SPI HW |
| SCLK | 11 | 23 | Marrón | SPI HW |
| MISO | 9 | 21 | Verde | SPI HW |

---

## Bug #5: `bcm2835_spi_begin()` Faltante

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `origin/libs/epaper/epaper.cpp` → `Spi_t` |
| **Síntoma** | SPI no transfería datos (bcm2835 no inicializado para SPI) |
| **Causa** | `Spi_t` configuraba data mode y clock pero nunca llamaba a `bcm2835_spi_begin()` |
| **Solución** | Agregar `bcm2835_spi_begin()` en el constructor de `Spi_t` |

```cpp
// ✅ CORRECTO:
Spi_t::Spi_t() {
    bcm2835_spi_begin();                                    // ← FALTABA
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);           // ← CS manual
}
```

**Nota**: `bcm2835_spi_begin()` configura los pines GPIO 9, 10, 11 como función alternativa SPI (ALT0). Sin esto, los pines quedan como GPIO normales y SPI no funciona.

---

## Bug #6: CS Configurado como SPI Hardware en vez de GPIO Manual

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | Varios |
| **Síntoma** | CS no se podía controlar porque bcm2835 lo manejaba automáticamente |
| **Causa** | No llamar a `bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE)` |
| **Solución** | Deshabilitar CS hardware y usar GPIO manual para CS |

```cpp
Spi_t::Spi_t() {
    // ...
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);  // CS manual por GPIO
}
```

---

## Bug #7: `DCDC_powerOn()` y `displayRefresh()` con Datos Extra

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `epaper.cpp` en algunas versiones |
| **Síntoma** | Comandos enviados con byte extra, COG los interpretaba mal |
| **Causa** | `sendIndexData(0x04, &register_data[0], 1)` — enviaba 1 byte de datos |
| **Solución** | `sendIndexData(0x04, &dummy, 0)` — solo comando, sin datos |

```cpp
// ❌ ANTES:
sendIndexData(0x04, &register_data[0], 1);  // Enviaba 0x00 como dato

// ✅ DESPUÉS:
uint8_t dummy = 0x00;
sendIndexData(0x04, &dummy, 0);             // Solo comando, len=0
```

**Referencia**: `EPD_Driver_GU_small` usa `sendIndexData(0x04, &dummy, 0)`.

---

## Bug #8: `COG_powerOff()` Escribiendo en BUSY

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `epaper_success_v1.0.0/libs/epaper/epaper.cpp` |
| **Síntoma** | Potencial conflicto — escribir en un pin configurado como entrada |
| **Causa** | `digitalWrite(pin_cfg_epaper.panelBusy, LOW)` — BUSY es INPUT |
| **Solución** | Eliminar la línea — BUSY solo se lee, nunca se escribe |

---

## Bug #9: `~Spi_t()` llamando a `exit(EXIT_SUCCESS)`

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `epaper_success/libs/spi/spi.cpp` (SPI por ioctl, código muerto) |
| **Síntoma** | El programa terminaba abruptamente al destruir Spi_t |
| **Causa** | `exit(EXIT_SUCCESS)` en el destructor |
| **Solución** | Código no usado. Eliminar o no incluir en el build. |

---

## Bug #10: `bcm2835_spi_end()` llamado dos veces

| Aspecto | Detalle |
|:--------|:--------|
| **Archivo** | `main.cpp` en `origin/` y otras versiones |
| **Síntoma** | Posible doble liberación de recursos SPI |
| **Causa** | `~Spi_t()` llamaba a `bcm2835_spi_end()`, y `main.cpp` también lo llamaba explícitamente |
| **Solución** | Encerrar el objeto display en bloque `{}` y llamar `bcm2835_close()` después |

```cpp
// ✅ CORRECTO:
int main() {
    if (!bcm2835_init()) return 1;
    
    {
        auto epaper = std::make_unique<EPD_Driver>(...);
        // usar display...
    }  // ~EPD_Driver → ~Spi_t → bcm2835_spi_end()
    
    bcm2835_close();  // seguro — SPI ya terminó
    return 0;
}
```

---

## Resumen de Correcciones por Proyecto

### Detalle: Bug #3 en Master/

`Master/` usa `sendIndexData(0xe0, &register_data[2], 1)` y `sendIndexData(0x00, &register_data[3], 2)` — **índices desplazados un paso**. Esto envía `0x19` (temperatura) en lugar de `0x02` (activa), y `0x02,0xcf` en lugar de `0xcf,0x8d` para PSR. `Master/` necesita la misma corrección que las otras versiones.

### Detalle: Bug #9 — `exit()` en destructor

Este bug está en `libs/spi/spi.cpp` (implementación SPI por ioctl de Linux). **Este archivo no se compila** en los proyectos que usan bcm2835 para SPI. El bug existe en el código fuente pero no afecta el binario final. Se marca como ❌ por precaución documental.

| Bug | `origin/` | `epaper_success_v1.0.0/` | `epaper_success_v1.0.1/` | `Master/` |
|:---:|:---------:|:-------------------------:|:-------------------------:|:---------:|
| #1 SPI Mode | ❌ | ✅ | ✅ | ✅ |
| #2 CS Toggle | ❌ (corregido) | ✅ | ✅ | ✅ |
| #3 Register indices | ❌ | ✅ | ✅ | ⚠️ (usa [2],[3]) |
| #4 Pines boards.h | ❌ (corregido) | ✅ (Zero2W) | ✅ | ✅ |
| #5 spi_begin() | ❌ (corregido) | ✅ | ✅ | ✅ |
| #6 CS_NONE | ❌ (corregido) | ✅ | ✅ | ✅ |
| #7 Datos extra en cmds | ❌ | ✅ | ✅ | ✅ |
| #8 BUSY write | ❌ | ✅ | ✅ | ✅ |
| #9 exit() en destructor | ❌ | ❌ (spi legacy, no compilado) | ❌ (spi legacy, no compilado) | ❌ (spi legacy, no compilado) |
| #10 Doble spi_end | ❌ (corregido) | ✅ | ✅ | ✅ |

✅ = Corregido  |  ❌ = Pendiente  |  ⚠️ = Parcial
