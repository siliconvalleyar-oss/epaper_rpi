# Datasheet y Referencia de Comandos EPD

## Visión General

Este documento contiene la referencia completa de comandos, registros y secuencias del controlador E-Paper COG, extraída directamente del código fuente y documentación del proyecto.

## Familias de Pantallas

### Pantallas Soportadas

El driver soporta pantallas de la serie `xE2xxxCSxxx` (E Ink Carta):

| Código | Diagonal | Resolución | Driver Interno |
|--------|----------|------------|----------------|
| xE2154CSxxx | 1.54" | 152x152 | Unknown |
| xE2213CSxxx | 2.13" | 212x104 | Unknown |
| xE2266CSxxx | 2.66" | 296x152 | Unknown |
| xE2271CSxxx | 2.71" | 264x176 | Unknown |
| xE2287CSxxx | 2.87" | 296x128 | Unknown |
| xE2370CSxxx | 3.70" | 416x240 | Unknown |
| xE2417CSxxx | 4.17" | 300x400 | Unknown |
| xE2437CSxxx | 4.37" | 480x176 | Unknown |

## Códigos de Definición

```cpp
#define eScreen_EPD_154 (uint32_t)0x1509
#define eScreen_EPD_213 (uint32_t)0x2100
#define eScreen_EPD_266 (uint32_t)0x2600
#define eScreen_EPD_271 (uint32_t)0x2700
#define eScreen_EPD_287 (uint32_t)0x2800
#define eScreen_EPD_370 (uint32_t)0x3700
#define eScreen_EPD_417 (uint32_t)0x4100
#define eScreen_EPD_437 (uint32_t)0x430C
```

**Convención**: `0xDDSS` donde:
- `DD` = diagonal en centímetros (15 = 1.5", 21 = 2.1", 26 = 2.6"...)
- `SS` = serie/submodelo (00, 09, 0C)

## Tabla de Comandos Maestra

| CMD HEX | CMD DEC | Nombre | Parámetros | Dirección | Descripción |
|---------|---------|--------|------------|-----------|-------------|
| 0x00 | 0 | Soft Reset | 1-2 bytes | COG -> Host | Reinicia COG, ejecuta LUT interno |
| 0x02 | 2 | Power OFF | 0 bytes | COG -> Host | Apaga DC/DC, espera estabilización |
| 0x04 | 4 | Power ON | 0 bytes | COG -> Host | Enciende DC/DC, espera BUSY=HIGH |
| 0x10 | 16 | First Frame | N bytes | Host -> COG | Carga primer frame (Black/Previous) |
| 0x12 | 18 | Display Refresh | 0 bytes | COG -> Host | Inicia refresco de pantalla |
| 0x13 | 19 | Second Frame | N bytes | Host -> COG | Carga segundo frame (Red/New) |
| 0xE0 | 224 | Active Temperature | 1 byte | Host -> COG | Temperatura operativa actual |
| 0xE5 | 229 | Input Temperature | 1 byte | Host -> COG | Temperatura referencia COG |

## Registros Internos

### Register Data Array

```cpp
// Pantallas pequeñas (sm)
const uint8_t register_data_sm[] = { 
    0x00,  // register_data[0]: Comando default para Power ON/OFF
    0x0E,  // register_data[1]: PSR para Soft Reset (default)
    0x19,  // register_data[2]: Temperatura entrada (25°C)
    0x02,  // register_data[3]: Temperatura activa (25°C)
    0xCF,  // register_data[4]: PSR alto
    0x8D   // register_data[5]: PSR bajo
};

// Pantallas medianas (mid) - para 4.2"
const uint8_t register_data_mid[] = { 
    0x00, 0x0E, 0x19, 0x02, 0x0F, 0x89 
};
```

### Significado de Registros

#### register_data[0] (0x00)
Comando enviado en operaciones Power ON/OFF:
- Power ON: `sendIndexData(0x04, &register_data[0], 1)`
- Power OFF: `sendIndexData(0x02, &register_data[0], 0)`

**Valor por defecto**: 0x00 (comando vacío de 1 byte)

#### register_data[1] (0x0E)
Parámetro para Soft Reset (comando 0x00):
- Enviado como segundo byte en `sendIndexData(0x00, &register_data[1], 1)`
- Configuración de Panel Setting Register (PSR) tras reset

**Valor por defecto**: 0x0E
- Bit 0: No definido
- Bit 1: No definido
- Bit 2: No definido
- Bit 3: No definido

#### register_data[2] (0x19)
Temperatura interna para comando Input Temperature (0xE5):
```cpp
sendIndexData(0xE5, &register_data[2], 1);  // 0x19 = 25°C
```

**Mapeo de temperatura**:
```
0x19 = 25°C
0x1A = 26°C
...
0x0F = 15°C
```

**Nota**: En código actual está hardcodeado a 25°C.

#### register_data[3] (0x02)
Temperatura activa para comando Active Temperature (0xE0):
```cpp
sendIndexData(0xE0, &register_data[3], 1);  // 0x02 = 25°C
```

#### register_data[4] y register_data[5] (0xCF, 0x8D)
Panel Setting Register (PSR) completo:
```cpp
sendIndexData(0x00, &register_data[4], 2);  // Envía 0xCF, 0x8D
```

**Desglose de PSR (register_data sm)**:

| Bit | register_data[4] (0xCF) | register_data[5] (0x8D) | Descripción |
|-----|-------------------------|-------------------------|-------------|
| 7 | 1 | 1 | Resolución alta (HYST = 3) |
| 6 | 1 | 0 | Resolución media (HYST = 2) |
| 5 | 1 | 0 | Sin definición |
| 4 | 1 | 1 | Sin definición |
| 3 | 1 | 0 | Sin definición |
| 2 | 1 | 0 | Sin definición |
| 1 | 1 | 0 | Sin definición |
| 0 | 1 | 1 | Sin definición |

**Nota**: La documentación oficial de E Ink no publica el significado exacto de cada bit para chips COG específicos. Los valores son configuración por defecto del fabricante.

## Formato de Comandos SPI

### Formato General

```
┌─────────┬──────────────────────────────────────┐
│ DC = 0  │ CS = 0 (Inicio comando)             │
├─────────┼──────────────────────────────────────┤
│ CMD     │ 1 byte (0x00 - 0xFF)                │
├─────────┼──────────────────────────────────────┤
│ DC = 1  │ CS = 1 (Fin comando)                │
├─────────┼──────────────────────────────────────┤
│ CS = 0  │ (Inicio datos)                      │
├─────────┼──────────────────────────────────────┤
│ DATA... │ N bytes                             │
├─────────┼──────────────────────────────────────┤
│ CS = 1  │ (Fin datos)                         │
└─────────┴──────────────────────────────────────┘
```

### Secuencia de Comandos

```cpp
void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    // Fase comando
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(index);  // Enviar comando
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);

    // Fase datos
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    for (uint32_t i = 0; i < len; i++) {
        hV_HAL_SPI_transfer(data[i]);  // Enviar datos
    }
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
}
```

## Secuencias Completas

### Secuencia de Inicialización COG

```cpp
// 1. Configuración inicial de pines
COG_initial();

// 2. Comandos de inicialización
sendIndexData(0x00, &register_data[1], 1);  // Soft Reset (0x00, 0x0E)
sendIndexData(0xE5, &register_data[2], 1);  // Input Temp (0xE5, 0x19)
sendIndexData(0xE0, &register_data[3], 1);  // Active Temp (0xE0, 0x02)
sendIndexData(0x00, &register_data[4], 2);  // PSR (0x00, 0xCF, 0x8D)

// 3. Dimensiones
v_screenSizeV = 296;
v_screenSizeH = 152;
image_data_size = 296 * 152 / 8 = 5,624 bytes;
```

### Secuencia de Actualización Global

```cpp
// 1. Cargar primer frame (5,624 bytes)
sendIndexData(0x10, data1, 5624);

// 2. Cargar segundo frame (5,624 bytes)
sendIndexData(0x13, data2, 5624);

// 3. Encender DC/DC
sendIndexData(0x04, &register_data[0], 1);  // (0x04, 0x00)
while (digitalRead(BUSY) != HIGH);           // Esperar busy=HIGH

// 4. Refrescar pantalla
sendIndexData(0x12, &register_data[0], 1);  // (0x12, 0x00)
while (digitalRead(BUSY) != HIGH);           // Esperar busy=HIGH
```

### Secuencia de Apagado

```cpp
// 1. Apagar DC/DC
sendIndexData(0x02, &register_data[0], 0);  // (0x02)
while (digitalRead(BUSY) != HIGH);           // Esperar busy=HIGH

// 2. Limpiar señales
digitalWrite(DC, LOW);
digitalWrite(CS, LOW);

// 3. Esperar descarga
delay_ms(150);

// 4. Reset final
digitalWrite(RESET, LOW);
```

### Secuencia de Reset Hardware

```cpp
void reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5) {
    delay_ms(ms1);                     // 5ms: espera inicial
    digitalWrite(RESET, HIGH);         // RES# = HIGH
    delay_ms(ms2);                     // 5ms
    digitalWrite(RESET, LOW);          // RES# = LOW
    delay_ms(ms3);                     // 10ms: reset interno
    digitalWrite(RESET, HIGH);         // RES# = HIGH
    delay_ms(ms4);                     // 5ms
    digitalWrite(CS, HIGH);            // CS# = HIGH
    delay_ms(ms5);                     // 5ms: estabilización
}
```

**Tiempos**:
```
T1 (5ms)
RESET: LOW -----+                    HIGH
                |<-- T2 (5ms) -->|
                |<--- T3 (10ms) ---->|
                |<-- T4 (5ms) -->|
CS:     HIGH -------------------------+---- HIGH
                  |<-- T5 (5ms) -->|
```

## Mapa de Bits de Pantalla

### Estructura de Buffer 2.66" (296x152)

```cpp
image_data_size = 296 * (152 / 8) = 5,624 bytes
```

**Organización**:
```
+-------------------------------------------------+
| Fila 0: bytes 0 - 36  (37 bytes = 296 bits)    |
| Fila 1: bytes 37 - 73 (37 bytes = 296 bits)    |
| ...                                             |
| Fila 151: bytes 554... (37 bytes = 296 bits)   |
+-------------------------------------------------+
Total: 152 filas x 37 bytes = 5,624 bytes
```

**Ejemplo de byte**:
```
Byte 0:    b7 b6 b5 b4 b3 b2 b1 b0
Pixel:     P0 P1 P2 P3 P4 P5 P6 P7
Color:     0  1  1  0  0  0  1  0 (0x62)

Donde:
- P0 = pixel columna 0 (izquierdo)
- P7 = pixel columna 7 (derecho)
- 1 = negro, 0 = blanco
```

## Mecanismos de Sincronización

### Señal BUSY

```cpp
// Patrón de uso
while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
```

**Significado**:
- `BUSY = LOW`: COG procesando, no aceptar comandos
- `BUSY = HIGH`: COG listo, aceptar comandos

**Tiempos típicos de espera**:
- Power ON: 500-900 ms
- Display Refresh: 300-900 ms (depende de temperatura y contenido)

**Nota**: No hay timeout implementado. Si COG no responde, el programa cuelga indefinidamente.

## Valores de Referencia

### Tiempos

| Parametro | Valor | Unidad | Notas |
|-----------|-------|--------|-------|
| VCC rise time | < 10 | ms | Alimentación 3.3V |
| RESET pulse (HIGH) | 5 | ms | Antes de pulso LOW |
| RESET pulse (LOW) | 10 | ms | Mínimo para reset interno |
| Post-reset CS delay | 5 | ms | CS debe estar HIGH |
| CS switching delay | 1 | ms | Entre comandos |
| DC/DC OFF delay | 150 | ms | Descarga capacitores |
| BUSY timeout | ∞ (sin límite) | ms | Implementación actual |

### Eléctricos

| Parametro | Valor | Unidad | Condiciones |
|-----------|-------|--------|-------------|
| VCC | 3.3 | V | ±10% |
| VCC rise time | < 10 | ms | |
| VIH (entradas) | 0.7 x VCC | V | HIGH |
| VIL (entradas) | 0.3 x VCC | V | LOW |
| VOH (salidas) | 0.8 x VCC | V | HIGH |
| VOL (salidas) | 0.2 x VCC | V | LOW |
| IIL (entradas) | ±1 | µA | Pull-down |
| IIH (entradas) | ±1 | µA | Pull-up |

### Operativas

| Parametro | Valor | Unidad | Notas |
|-----------|-------|--------|-------|
| fSCLK máximo | 2 | MHz | SPI clock |
| fSCLK operativo | 1.6 | MHz | Configurado en driver |
| tSP (SPI clock period) | 625 | ns | A 1.6 MHz |
| tDS (data setup) | 50 | ns | Antes de SCLK rising |
| tDH (data hold) | 50 | ns | Después de SCLK rising |
| T_update (completo) | ~1 | s | 2 frames + DC/DC + refresh |

## Errores en Hardware

### Glitches Comunes

| Síntoma | Causa Probable | Solución |
|---------|----------------|----------|
| Pantalla parpadea | BUSY no respetado | Agregar `while(BUSY)` después de cada comando |
| Imagen corrupta | SPI muy rápido | Reducir SPI_SPEED a 800 kHz |
| COG no responde | RESET no pulsado | Verificar temporización de reset |
| COG recalienta | Alimentación inestable | Usar capacitor 100µF en VCC |
| Píxeles fantasma | Actualizaciones muy rápidas | Esperar > 180s entre actualizaciones completas |

## Extended Information

### Onda Típica COG

```
VCC:    3.3V ----+-----------------------> (siempre)
RES#:           +--+  +------+
               |  |  |      |
               LOW 10us  (resto)

BUSY:           +----------------------+
               |                       |
               LOW (ocupado)    HIGH (listo)

CS#:            ----+    +--+    +---
                   |    |  |    |
                   CS=HIGH  CS=LOW (para comando/dato)

DC#:            ----+    +----------+
                   |                 |
                   Comando    Dato
```

### Duración de Batería (Estimado)

| Tamaño | Actualizacion/día | Corriente | Batería 2000mAh |
|--------|-------------------|-----------|-----------------|
| 2.13" | 4 | 30 mA | 66 días |
| 2.66" | 4 | 40 mA | 50 días |
| 3.70" | 4 | 60 mA | 33 días |

**Nota**: E-Paper consume energía solo durante actualización. En modo estático consume <1µA.
