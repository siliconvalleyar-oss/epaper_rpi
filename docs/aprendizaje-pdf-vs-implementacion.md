# Análisis: PDF Resumen SPI vs Implementación Raspberry Pi

## Referencias

- **Documento PDF**: `docs/ResumenSpiEpaperRaspberryPiZ.pdf` (Rev. 02, 2022/06/06)
- **Código implementado**: `libs/epaper/epaper.cpp`
- **Proyecto**: `epaper_success_2026_1.0.1`

---

## 1. Flujo de Actualización del EPD

### PDF (Página 13) — Driving Flow Chart

```
Start → Get Temperature → Power On → Input Temperature
  → Send Image Data → Send Power Settings → Turn on DC/DC
  → Send Update Command → Wait BUSY=HIGH → Turn off DC/DC
```

### Implementado en `epaper.cpp` — `globalUpdate()`

```cpp
void EPD_Driver::globalUpdate(const uint8_t *data1s, const uint8_t *data2s) {
    sendIndexData(0x10, data1s, image_data_size);  // Frame 1: imagen
    sendIndexData(0x13, data2s, image_data_size);  // Frame 2: placeholder
    DCDC_powerOn();                                  // Encender DC/DC
    displayRefresh();                                // Refrescar display
}
```

### Tabla de Correspondencia

| Paso PDF | Comando | Código RPi | Estado |
|:---------|:-------:|------------|:------:|
| Get Temperature | — | No implementado (se asume 25°C) | ⚠️ |
| Input Temperature | `0xE5` | `sendIndexData(0xe5, &register_data[2], 1)` | ✅ Corregido |
| Power On (COG) | Secuencia reset | `COG_initial()` → `reset()` + `softReset()` | ✅ |
| Send Image Data (Frame 1) | `0x10` | `sendIndexData(0x10, data1s, image_data_size)` | ✅ |
| Send Placeholder (Frame 2) | `0x13` | `sendIndexData(0x13, data2s, image_data_size)` | ✅ Corregido |
| Turn on DC/DC | `0x04` | `sendIndexData(0x04, &dummy, 0)` | ✅ |
| Display Refresh | `0x12` | `sendIndexData(0x12, &dummy, 0)` | ✅ |
| Wait BUSY=HIGH | Polling | `while (... != HIGH)` con timeout | ✅ Corregido |
| Turn off DC/DC | `0x02` | `sendIndexData(0x02, ...)` en `COG_powerOff()` | ✅ |

---

## 2. SPI Timing y Protocolo (PDF Páginas 15-16)

### Especificación del PDF

| Parámetro | Mínimo | Máximo |
|-----------|:------:|:------:|
| Serial Clock Cycle (Write) | 100 ns | — |
| Serial Clock Cycle (Read) | 200 ns | — |
| Chip Select Setup Time | 60 ns | — |
| Clock High Time | 40 ns | — |
| Clock Low Time | 40 ns | — |
| Frecuencia SPI | — | **10 MHz** |

### Configuración en Raspberry Pi

```cpp
bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);       // CPOL=0, CPHA=1
bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);  // ~1.95 MHz
```

| Aspecto | PDF | RPi | Verificación |
|:--------|:---:|:---:|:-------------|
| Frecuencia máx. | 10 MHz | 1.95 MHz | ✅ Muy por debajo del límite |
| MSB primero | Sí | `MSBFIRST` | ✅ |
| Modo SPI | No especificado explícitamente | **Mode 1** (CPOL=0, CPHA=1) | ✅ Determinado empíricamente |

### SPI Mode 1: Determinación Experimental

El PDF no especifica explícitamente el modo SPI. La configuración se determinó por:

1. **Referencia PIC32**: El código funcional del PIC32 usa `CKP=0, CKE=1` que corresponde a SPI Mode 1
2. **Prueba empírica**: Con SPI Mode 0 el display **no respondía** (BUSY nunca HIGH). Con SPI Mode 1 el display **funcionó correctamente**

---

## 3. Formato de Comando SPI (PDF Página 15)

### PDF

``` 
CS# \___/                       \_____/ 
         |   Index  |   D0  |   D1  | ... |   Dn  |
```

Formato: 1 byte de índice (registro) seguido de N bytes de datos.

### Implementación en `sendIndexData()`

```cpp
void EPD_Driver::sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    digitalWrite(pin_cfg_epaper.panelDC, LOW);   // Modo comando
    digitalWrite(pin_cfg_epaper.panelCS, LOW);   // CS activo
    hV_HAL_SPI_transfer(index);                  // Envía índice (1 byte)

    if (len > 0) {
        digitalWrite(pin_cfg_epaper.panelDC, HIGH);  // Modo datos
        for (uint32_t i = 0; i < len; i++) {
            hV_HAL_SPI_transfer(data[i]);            // Envía datos
        }
    }

    digitalWrite(pin_cfg_epaper.panelCS, HIGH);  // CS inactivo
}
```

### Aprendizaje del PDF vs Código Original

| Aspecto | PDF | Código Original (bug) | Código Corregido |
|:--------|:---:|:---------------------:|:----------------:|
| CS durante transacción | Debe estar LOW | Se pulsaba HIGH entre bytes | ✅ **Corregido**: CS LOW todo el comando+datos |

---

## 4. Temperatura (PDF Página 18)

### PDF

```
SPI(0xe5, TSSET)   → Input temperature
SPI(0xe0, 0x19)    → Active temperature (activate internal sensor)
```

- `TSSET`: Temperatura en complemento a 2. 25°C = 0x19

### Implementación

```cpp
sendIndexData(0xe5, &register_data[2], 1);  // 0x19 = 25°C
sendIndexData(0xe0, &register_data[2], 1);  // 0x19 (activate internal)
```

**Aprendizaje**: Ambos comandos deben apuntar al mismo valor (`register_data[2]` = 0x19). El primer comando (0xE5) carga la temperatura, el segundo (0xE0) la activa. Inicialmente el código usaba `register_data[3]` para 0xE0, lo cual era incorrecto.

---

## 5. PSR — Panel Setting Register (PDF Página 18)

### PDF

```
SPI(0x00, PSR0, PSR1)   → Panel Setting Register
```

PSR es específico para cada tamaño de pantalla.

### Implementación

```cpp
sendIndexData(0x00, &register_data[3], 2);  // [0x02, 0xcf]
```

Donde `register_data[3] = 0x02` y `register_data[4] = 0xcf`.

**Aprendizaje**: Originalmente el código usaba `register_data[4]` como inicio, lo que enviaba `[0xcf, 0x8d]` — valores incorrectos que podían impedir la inicialización del COG.

---

## 6. Formato de Imagen (PDF Páginas 19-20)

### PDF

```
Frame 1 (0x10): Imagen principal   → 1=negro, 0=blanco
Frame 2 (0x13): Placeholder        → Todo 0x00
```

### Tabla de Tamaños de Frame

| Display | Resolución | Bytes/Frame (PDF) | Bytes/Frame (RPi) | Coincide |
|:--------|:----------:|:------------------:|:------------------:|:--------:|
| 1.54" | 152×152 | 2,888 | 2,888 | ✅ |
| 2.13" | 212×104 | 2,756 | 2,756 | ✅ |
| 2.66" | 296×152 | 5,624 | 5,624 | ✅ |
| 2.71" | 264×176 | 5,808 | 5,808 | ✅ |
| 2.87" | 296×128 | 4,736 | 4,736 | ✅ |
| 3.70" | 416×240 | 12,480 | 12,480 | ✅ |
| 4.17" | 300×400 | 15,000 | 15,000 | ✅ |
| 4.37" | 480×176 | 10,560 | 10,560 | ✅ |

**Aprendizaje**: El Frame 2 con 0x00 es correcto para modo monocromático (BW). Para BWR, el Frame 2 debe contener los datos del canal rojo. El código original forzaba 0x00 ignorando `data2s`; fue corregido para respetar el parámetro.

---

## 7. Secuencia de Actualización Completa (PDF Página 21)

### PDF

```
1. SPI(0x04)      → Power On (DC/DC)
2. Wait BUSY=HIGH
3. SPI(0x12)      → Display Refresh
4. Wait BUSY=HIGH
```

### Implementación

```cpp
void EPD_Driver::DCDC_powerOn() {
    sendIndexData(0x04, &dummy, 0);   // Power On
    // Wait BUSY con timeout 5s
}

void EPD_Driver::displayRefresh() {
    sendIndexData(0x12, &dummy, 0);   // Display Refresh
    // Wait BUSY con timeout 20s
}
```

---

## 8. Resumen de Correcciones Realizadas

| # | Problema | Síntoma | Corrección |
|:-:|:---------|:--------|:-----------|
| 1 | `bcm2835_spi_begin()` no llamado | SPI no funcional (pines en modo GPIO) | Agregado en constructor `Spi_t` |
| 2 | CS pulsado entre bytes | Display ignoraba comandos | CS LOW durante toda la transacción |
| 3 | Índices de `register_data` incorrectos | Configuración temperatura/PSR errónea | Corregidos índices (0xE0, 0x00) |
| 4 | `globalUpdate()` ignoraba `data2s` | Frame rojo siempre cero | Ahora usa el buffer recibido |
| 5 | Sin timeouts en BUSY-wait | Programa se colgaba si display no responde | Timeouts de 5s y 20s con mensaje de error |
| 6 | SPI Mode 0 | Display no respondía | Cambiado a **SPI Mode 1** |
| 7 | `exit()` en destructor SPI | Muerte súbita del proceso | Eliminado (era código no usado) |

---

## 9. Flujo Completo Verificado

```
main()
  ├── bcm2835_init()
  ├── EPD_Driver(screen, board)
  │     └── Spi_t() → bcm2835_spi_begin(), SPI Mode 1, ~1.95 MHz
  │
  ├── COG_initial()
  │     ├── pinMode() / digitalWrite() → Configura GPIOs
  │     ├── reset(5,5,10,5,5)          → Pulso RST
  │     ├── softReset()                → SPI(0x00, 0x0E) + wait BUSY timeout 5s
  │     ├── SPI(0xE5, 0x19)            → Input temperatura 25°C
  │     ├── SPI(0xE0, 0x19)            → Activar temp. interna
  │     └── SPI(0x00, [0x02, 0xCF])    → PSR
  │
  ├── globalUpdate(data1, data2)
  │     ├── SPI(0x10, frame1, 5624)    → Frame imagen
  │     ├── SPI(0x13, frame2, 5624)    → Frame placeholder/rojo
  │     ├── DCDC_powerOn()             → SPI(0x04) + wait BUSY timeout 5s
  │     └── displayRefresh()           → SPI(0x12) + wait BUSY timeout 20s
  │
  ├── COG_powerOff()
  │     ├── SPI(0x02)                  → Power off + wait BUSY timeout 5s
  │     └── GPIO cleanup
  │
  ├── epaper.reset()                   → ~Spi_t() → bcm2835_spi_end()
  └── bcm2835_close()
```

---

## Conclusiones

1. **SPI Mode 1 es el correcto** para este display, determinado empíricamente por comparación con el PIC32
2. **CS debe permanecer LOW** durante toda la transacción comando+datos, a pesar de que el PDF muestre toggles entre clocks
3. **Los valores de registro (temperatura, PSR)** deben usar los índices correctos del array `register_data[]`
4. **Los timeouts** evitan que el programa se cuelgue si el display no responde
5. **La velocidad SPI de 1.95 MHz** está muy por debajo del límite de 10 MHz del display
6. **El código actual sigue fielmente el flujo** descrito en el PDF y ha sido validado: el display responde correctamente sin timeouts
