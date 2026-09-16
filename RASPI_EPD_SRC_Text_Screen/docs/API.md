# API de Dibujo

## EpaperDisplay

Clase principal para interactuar con el display e-Paper.

### Constructor

```cpp
EPAPER_DISPLAY::EpaperDisplay display(screen_type, board);
```

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `screen_type` | `uint32_t` | `eScreen_EPD_266` (2.66"), `eScreen_EPD_213` (2.13"), `eScreen_EPD_154` (1.54") |
| `board` | `const EPAPER::pins_t&` | Configuración de pines (`boardRaspberryPiZero2W`, etc.) |

### Inicialización

```cpp
display.init();  // Inicializa COG, reset hardware, configura registros
```

Retorna `true` si éxito, `false` si error.

### Dibujo

```cpp
// Pixel
display.drawPixel(x, y, black);  // black = true (negro), false (blanco)

// Línea (Bresenham)
display.drawLine(x0, y0, x1, y1, black);

// Rectángulo
display.drawRect(x, y, w, h, fill, black);  // fill = true (relleno), false (solo borde)

// Texto
display.drawString(x, y, "texto", FONT_5x8, black);
display.drawCenteredString(y, "centrado", FONT_7x8_THICK, black);
```

### Buffer

```cpp
display.clearScreen(true);   // Limpiar a blanco
display.update();            // Enviar buffer al display (globalUpdate)

uint8_t* buf = display.getBuffer();  // Acceso directo al buffer
int w = display.getWidth();          // Ancho en píxeles
int h = display.getHeight();         // Alto en píxeles
```

### Patrón de prueba

```cpp
display.testPattern();  // Marco + diagonal + rectángulo + texto "TEST"
```

### Dimensiones por pantalla

| Tipo | Ancho | Alto | Buffer (bytes) |
|------|-------|------|----------------|
| `eScreen_EPD_154` | 200 | 200 | 5000 |
| `eScreen_EPD_213` | 212 | 104 | 2756 |
| `eScreen_EPD_266` | 296 | 152 | 5624 |

### Sistema de coordenadas

El buffer usa organización **column-major** con **espejo horizontal**:

```cpp
// Coordenadas internas del buffer
int mirroredX = width - 1 - x;
int byteIndex = (mirroredX * height + y) / 8;
int bitIndex  = 7 - (y % 8);
```

Esto es requerido por el hardware EPD de Pervasive Displays.

### Ciclo de vida

```cpp
{
    EPAPER_DISPLAY::EpaperDisplay display(screen_type, board);
    display.init();
    // ... usar display ...
}  // display se destruye aquí
bcm2835_close();  // Seguro después de que display se destruyó
```

**Importante**: El display debe destruirse ANTES de `bcm2835_close()`.
El bloque `{}` controla esto automáticamente.
