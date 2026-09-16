# Sistema de Fuentes

## Fuentes disponibles

| Enum | Nombre | Tamaño | Bytes/char | Rango |
|------|--------|--------|-----------|-------|
| `FONT_5x8` | Standard | 5x8 | 5 | 32-127 |
| `FONT_7x8_THICK` | Thick | 7x8 | 7 | 32-127 |
| `FONT_7x8_HOMESPUN` | Homespun | 7x8 | 7 | 32-127 |
| `FONT_3x8_TINY` | Tiny | 3x8 | 3 | 32-127 |
| `FONT_4x8_SEG` | Seven Segment | 4x8 | 4 | 32-127 |
| `FONT_8x8` | Original | 8x8 | 8 | 32-127 |
| `FONT_8x8_WIDE` | Wide | 8x8 | 8 | 32-127 |
| `FONT_16x32_BIGNUM` | Big Numbers | 16x32 | 64 | 48-58 (0-9, :) |
| `FONT_16x16_MEDNUM` | Medium Numbers | 16x16 | 32 | 48-58 (0-9, :) |

## Formato de bitmap

### Fuentes regulares (5x8, 7x8, etc.)

Cada carácter es un array de `width` bytes. Cada byte representa una **columna** donde:
- Bit 0 = fila superior
- Bit 7 = fila inferior

```
Carácter 'A' (5x8):
  Byte 0: 0x7E  →  01111110
  Byte 1: 0x09  →  00001001
  Byte 2: 0x09  →  00001001
  Byte 3: 0x09  →  00001001
  Byte 4: 0x7E  →  01111110
```

### Fuentes de números grandes (16x32, 16x16)

Bitmap en formato row-major. Cada byte contiene 8 píxeles horizontales.

## Uso con EpaperDisplay

```cpp
#include <epaper/epaper_display.h>
#include <fonts/fonts_manager.h>

// Opción 1: API directa
display.drawString(10, 10, "Hola", FONT_5x8, true);
display.drawCenteredString(50, "CENTRADO", FONT_7x8_THICK, true);

// Opción 2: FontManager para debug
FontManager fm;
fm.setFont(FONT_7x8_THICK);
fm.printChar('A');    // Imprime en consola
fm.printString("Hello");
fm.printFontInfo();
```

## Agregar una fuente nueva

1. Definir datos del bitmap en `libs/fonts/fonts.cpp`
2. Declarar en `libs/fonts/fonts.h`
3. Agregar enum en `FontType` (`fonts_manager.h`)
4. Agregar caso en `FontManager::setFont()` y `getFontData()`
5. Actualizar `getCharSize()` con el tamaño en bytes
