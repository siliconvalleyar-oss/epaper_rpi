# E-Paper Fast Update - Documentación Técnica

## Fuente
- `Small Size Fast Update EPD.pdf` (PDI Rev.02, 2022/09/07)
- `ApplicationNote_Small_Size_Mono_v02_220606.pdf` (PDI Rev.02, 2022/06/06)
- `PDLS_EXT3_Basic_Global-main/` (Rei Vilo, v804)
- `PDLS_EXT3_Basic_Fast/` (Rei Vilo)

---

## Modos de Actualización

### Normal Update (lo que hacemos actualmente)
- Realiza la waveform completa: inverse → shaking → imaging
- **Tarda 5-20 segundos** (displayRefresh 0x12)
- Mejor calidad de imagen, sin ghosting

### Fast Update (lo que necesitamos implementar)
- Ejecuta una waveform corta
- **COG compara pixel por pixel** la imagen actual vs la nueva
- Solo drivea los píxeles que cambiaron
- **Tarda ~1 segundo** (según especificación)

---

## Diferencias Clave: Normal vs Fast

### 1. Inicialización (Capítulo 4 del appnote Fast)

**Normal Update:**
```
SPI(0xe5, TSSET)          # Temperatura ej: 0x19 = 25°C
SPI(0xe0, 0x02)           # Active Temperature
SPI(0x00, PSR[0], PSR[1]) # Panel Settings
```

**Fast Update:**
```
SPI(0xe5, TSSET | 0x40)           # Temperatura + flag 0x40!
SPI(0xe0, 0x02)                    # Active Temperature
SPI(0x00, PSR[0] | 0x10, PSR[1] | 0x02)  # PSR + flags!
SPI(0x50, 0x07)                    # CDI = 0x07 (Vcom data interval)
```

**Cambios en bits:**
- Temperatura: OR con `0x40` (bit 6 = fast mode flag)
- PSR byte 0: OR con `0x10` (bit 4)
- PSR byte 1: OR con `0x02` (bit 1)
- CDI: fijo en `0x07`

### 2. Envío de Imagen (Capítulo 5 del appnote Fast)

**Normal Update:**
```
SPI(0x10, DTM1)  # DTM1 = NUEVA imagen (la que quiero mostrar)
SPI(0x50, 0x27)  # Border setting (solo para 1.54", 2.13", 2.66", 3.7", 4.37")
SPI(0x13, DTM2)  # DTM2 = Dummy (0x00)
SPI(0x50, CDI)   # CDI = 0x07
```

**Fast Update:**
```
SPI(0x10, DTM1)  # DTM1 = IMAGEN VIEJA (la que está en pantalla ahora)
SPI(0x13, DTM2)  # DTM2 = IMAGEN NUEVA (la que quiero mostrar)
```

**¡CRÍTICO!** En Fast Update los frames están **INVERTIDOS**:
- `0x10` = OLD (antes)
- `0x13` = NEW (ahora)

### 3. Definición de DTM1/DTM2

| Modo | DTM1 (0x10) | DTM2 (0x13) |
|------|-------------|-------------|
| Normal | NEW image | Dummy (0x00) |
| Fast | OLD image (en pantalla) | NEW image (a mostrar) |

### 4. Update Command (Capítulo 6)

Ambos modos usan el mismo comando:
```
SPI(0x04)  # Power on
# wait BUSY = HIGH
SPI(0x12)  # Display Refresh
# wait BUSY = HIGH
```

---

## Valores para 2.66" (E266)

### PSR (Panel Setting Register)
- **Normal:** `0xCF, 0x8D`
- **Fast:** `0xCF | 0x10 = 0xDF`, `0x8D | 0x02 = 0x8F`

### Tamaño de frame
- N = 152, M = 296
- Total bytes/frame = 5,624

### Temperatura
- Ejemplo a 25°C: `0x19`
- Fast mode: `0x19 | 0x40 = 0x59`

---

## Flujo Completo Fast Update

```
1. Reset + Power on COG (igual que normal)
2. Soft-reset: SPI(0x00, 0x0E)
3. delay(5)
4. Input Temperature: SPI(0xE5, temp | 0x40)  ← ¡FLAG!
5. Active Temperature: SPI(0xE0, 0x02)
6. Panel Settings: SPI(0x00, 0xDF, 0x8F)      ← ¡FLAGS!
7. CDI: SPI(0x50, 0x07)
8. SPI(0x10, oldImage, 5624)                   ← ¡IMAGEN VIEJA!
9. SPI(0x13, newImage, 5624)                   ← ¡IMAGEN NUEVA!
10. Power on: SPI(0x04)
11. wait BUSY = HIGH
12. Display Refresh: SPI(0x12)
13. wait BUSY = HIGH
14. (Opcional) Power off: SPI(0x02)
```

---

## Notas del App Note Normal (Mono)

### SPI Format
- CS# se pulla HIGH entre cada byte de datos multi-byte
- "If register data is more than one byte, the CS# pulse is necessary between each data byte"
- **PERO** la referencia PDLS hace burst (CS LOW continuo) y funciona

### Frame Data
- First Frame (0x10): 1 = black, 0 = white
- Second Frame (0x13): debe ser 0x00 (dummy) en Normal
- En Fast: 0x13 = nueva imagen

### SPI Timing
- Max clock: 10 MHz
- CS Setup: 60ns min
- CS Hold: 65ns min

---

## Tabla Resumen de Cambios en Código

| Componente | Normal (actual) | Fast (necesario) |
|-----------|-----------------|-------------------|
| Temp init | `0xe5, temp` | `0xe5, temp \| 0x40` |
| PSR init | `0x00, 0xCF, 0x8D` | `0x00, 0xDF, 0x8F` |
| CDI | No se envía | `0x50, 0x07` |
| Frame 0x10 | New image | **Old image** |
| Frame 0x13 | 0x00 (dummy) | **New image** |
| Refresh | 0x12 | 0x12 (igual) |
| Tiempo refresh | 5-20s | ~1s |

---

## Referencias
- `Small Size Fast Update EPD.pdf` → Capítulos 4, 5, 6
- `ApplicationNote_Small_Size_Mono_v02_220606.pdf` → Capítulos 3, 4, 5
- `PDLS_EXT3_Basic_Fast/src/Screen_EPD_EXT3.cpp` → `COG_SmallP_initial()`, `COG_SmallP_sendImageData()`
