# Changelog

## [1.0.0] - 2026-06-15

### Added
- Primer release estructurado con documentación
- Sistema de fuentes: 5x8, 7x8 THICK, 7x8 HOMESPUN, 3x8 TINY, 4x8 SEG, 16x32 BIGNUM, 16x16 MEDNUM
- Controlador SPI para E-Paper SSD1306 (213 / 266 / 154)
- Detección automática de arquitectura ARM (32 vs 64-bit) via `config.h`
- Funciones de dibujo: pixel, línea, rectángulo, texto, texto centrado
- `testPattern()` para verificar funcionamiento del display
- Scripts de instalación y herramientas GPIO

### Fixed
- `drawPixel` con espejo horizontal y organización column-major (corrige renderizado en pantallas e-paper Pervasive Displays)
- Inicialización COG con try/catch para manejo de errores

### Hardware soportado
- Raspberry Pi Zero 2W / Pi 4
- Pantallas e-Paper Pervasive Displays (2.13", 2.66", 1.54")
- SPI + GPIO con librería bcm2835
