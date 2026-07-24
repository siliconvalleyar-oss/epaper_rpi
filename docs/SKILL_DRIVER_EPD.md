# SKILL: Driver EPD — Funcionamiento Interno

## 🔬 Arquitectura del Driver

El controlador de pantallas E-Paper (EPD) de Pervasive Displays se comunica vía SPI con el Chip-on-Glass (COG) montado en el flex de la pantalla.

### Capas del Driver

```
┌──────────────────────────────────┐
│         Aplicación (main.cpp)     │
│   Crea EPD_Driver, llama COG_initial,
│   globalUpdate, COG_powerOff     │
├──────────────────────────────────┤
│         EPD_Driver                │
│   - sendIndexData()              │
│   - COG_initial()                │
│   - globalUpdate()               │
│   - DCDC_powerOn()               │
│   - displayRefresh()             │
├──────────────────────────────────┤
│         Spi_t                     │
│   - bcm2835_spi_transfer()       │
│   - CS toggle manual por GPIO    │
├──────────────────────────────────┤
│         Gpio_t                    │
│   - pinMode() / digitalWrite()   │
│   - digitalRead()                │
├──────────────────────────────────┤
│         bcm2835                   │
│   - Acceso a registros del SoC   │
└──────────────────────────────────┘
```

---

## 📡 Protocolo SPI Detallado

### Configuración

| Parámetro | Valor | Razón |
|:----------|:------|:------|
| Modo SPI | **MODE0** (CPOL=0, CPHA=0) | El COG samplea datos en flanco de subida |
| Bit Order | **MSB First** | Estándar SPI |
| Clock Divider | **256** (~976 KHz) | La referencia usa este valor. 128 (~2MHz) puede funcionar pero 256 es más seguro |
| CS | **Manual por GPIO** | `bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE)` — controlamos CS con GPIO dedicado |

### ⚠️ CRÍTICO: CS debe togglear entre cada byte

```cpp
// ✅ CORRECTO (como en EPD_Driver_GU_small):
void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    // Comando
    digitalWrite(DC, LOW);
    digitalWrite(CS, LOW);
    bcm2835_spi_transfer(index);
    digitalWrite(CS, HIGH);     // ← CS sube entre comando y datos

    // Cada byte de dato
    for (uint32_t i = 0; i < len; i++) {
        digitalWrite(DC, HIGH);
        digitalWrite(CS, LOW);
        bcm2835_spi_transfer(data[i]);
        digitalWrite(CS, HIGH); // ← CS sube entre cada byte
    }
}

// ❌ INCORRECTO (CS se queda bajo):
void sendIndexData_bad(uint8_t index, const uint8_t *data, uint32_t len) {
    digitalWrite(DC, LOW);
    digitalWrite(CS, LOW);
    bcm2835_spi_transfer(index);
    // CS sigue LOW aquí — MAL
    digitalWrite(DC, HIGH);
    for (uint32_t i = 0; i < len; i++) {
        bcm2835_spi_transfer(data[i]);
        // CS nunca sube hasta el final
    }
    digitalWrite(CS, HIGH);
}
```

---

## 🔄 Secuencia Completa de Operación

### 1. Inicialización (COG_initial)

```
Timing diagram:
┌─────────────────────────────────────────────────────┐
│ Delay 5ms                                           │
├─────────────────────────────────────────────────────┤
│ RESET pulse: HIGH(5ms) → LOW(10ms) → HIGH(5ms)     │
├─────────────────────────────────────────────────────┤
│ Soft Reset: sendIndexData(0x00, &reg[1], 1)        │
│ Wait BUSY=HIGH                                      │
├─────────────────────────────────────────────────────┤
│ sendIndexData(0xE5, &reg[2], 1)   // Input Temp    │
│ sendIndexData(0xE0, &reg[3], 1)   // Active Temp   │
│ sendIndexData(0x00, &reg[4], 2)   // PSR config    │
└─────────────────────────────────────────────────────┘
```

**Registros de configuración** (array de 6 bytes):

```
register_data_sm = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };
                     [0]    [1]    [2]    [3]    [4]    [5]

Comando 0xE5 (Input Temperature):  reg[2] = 0x19 = 25°C
Comando 0xE0 (Active Temperature): reg[3] = 0x02
Comando 0x00 (PSR):               reg[4..5] = 0xcf, 0x8d
```

**⚠️ Importante**: Los índices `[2]`, `[3]`, `[4..5]` son específicos. Error común: usar `[2]` para 0xE0 y `[3]` para 0x00 (esto envía valores incorrectos al COG).

### 2. Actualización Global (globalUpdate)

```
sendIndexData(0x10, data_black, image_data_size)   // Frame negro
sendIndexData(0x13, data_red,   image_data_size)   // Frame rojo
      ↓
DCDC_powerOn()   → sendIndexData(0x04, dummy, 0) → Wait BUSY=HIGH
      ↓
displayRefresh() → sendIndexData(0x12, dummy, 0) → Wait BUSY=HIGH
```

**Tamaño de frame**: `image_data_size = screenSizeV * (screenSizeH / 8)`

| Pantalla | Resolución | Frame size (bytes) |
|:---------|:-----------|:------------------:|
| 1.54" | 152×152 | 2,888 |
| 2.13" | 212×104 | 2,756 |
| 2.66" | 296×152 | 5,624 |
| 2.71" | 264×176 | 5,808 |
| 2.87" | 296×128 | 4,736 |
| 3.70" | 416×240 | 12,480 |
| 4.17" | 300×400 | 15,000 |
| 4.37" | 480×176 | 10,560 |

### 3. Apagado (COG_powerOff)

```
sendIndexData(0x02, dummy, 0)   // Power OFF command
Wait BUSY=HIGH
DC = LOW
CS = LOW
Delay 150ms
RESET = LOW
```

---

## ⏱️ Timeouts (protección contra cuelgues)

El driver de referencia usa **bucles infinitos** (`while(...) ;`). Las versiones corregidas agregan timeouts:

| Operación | Timeout | Por qué |
|:----------|:-------:|:--------|
| `softReset()` | 5 seg | El reset debe ser rápido |
| `DCDC_powerOn()` | 5-10 seg | Encender DC/DC toma ~1-2 seg |
| `displayRefresh()` | 20-60 seg | El refresh completo puede tomar hasta 20 seg |
| `COG_powerOff()` | 5 seg | Apagado rápido |

---

## 📐 Mapa de Memoria del Buffer de Imagen

### Organización de píxeles

```
Para pantalla 2.66" (296×152):

152 filas × 296 columnas = 44,992 píxeles
Cada fila: 296 bits = 37 bytes
Total: 152 × 37 = 5,624 bytes

Byte layout:
Byte 0:   [b7 b6 b5 b4 b3 b2 b1 b0]  → píxeles 0-7 de fila 0
Byte 1:   [b7 b6 b5 b4 b3 b2 b1 b0]  → píxeles 8-15 de fila 0
...
Byte 36:  [b7 b6 b5 b4 b3 b2 b1 b0]  → píxeles 288-295 de fila 0
Byte 37:  [b7 b6 b5 b4 b3 b2 b1 b0]  → píxeles 0-7 de fila 1
```

1 = negro/blanco, 0 = blanco/rojo (depende del canal)

### Para BWR (blanco-negro-rojo)

Se requieren **dos frames**:
- **Frame 1** (0x10): Define qué píxeles son **negros**
- **Frame 2** (0x13): Define qué píxeles son **rojos**
- Un píxel con bit=0 en ambos frames = **blanco**

---

## 🧩 Inicialización de bcm2835

### Orden correcto

```cpp
// 1. PRIMERO: inicializar bcm2835
if (!bcm2835_init()) {
    std::cerr << "Error bcm2835_init()" << std::endl;
    return 1;
}

// 2. El constructor de EPD_Driver llama a Spi_t → bcm2835_spi_begin()
//    que configura los pines SPI (GPIO 9-11) como ALT0

// 3. Usar el display...

// 4. AL FINAL: liberar recursos EN ORDEN
epaper.reset();          // destructor → ~Spi_t → bcm2835_spi_end()
bcm2835_close();         // cierra /dev/mem
```

### ⚠️ Error común: doble cierre

Si `Spi_t::~Spi_t()` llama a `bcm2835_spi_end()`, NO debes llamarlo también en `main()`. Usa un bloque `{}` para controlar el ciclo de vida:

```cpp
int main() {
    if (!bcm2835_init()) return 1;
    
    {
        auto epaper = std::make_unique<EPD_Driver>(...);
        epaper->COG_initial();
        // ... usar display ...
    }  // epaper se destruye aquí → bcm2835_spi_end()
    
    bcm2835_close();  // ahora es seguro
    return 0;
}
```

---

## 🎯 Pines GPIO (configuración correcta)

| Señal | GPIO | Pin RPi | Color cable | Dirección |
|:------|:----:|:-------:|:-----------:|:---------:|
| BUSY | 25 | 22 | Rojo | Entrada |
| D/C | 24 | 18 | Naranja | Salida |
| RESET | 23 | 16 | Amarillo | Salida |
| CS | 27 | 13 | Gris | Salida |
| flashCS | 22 | 15 | Violeta | Salida (no usado) |
| MOSI | 10 | 19 | Azul | SPI HW |
| SCLK | 11 | 23 | Marrón | SPI HW |
| MISO | 9 | 21 | Verde | SPI HW (no usado) |
