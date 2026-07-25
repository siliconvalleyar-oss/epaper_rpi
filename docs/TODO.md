# TODO — E-Paper Display para Raspberry Pi

## Corto plazo

- [ ] Agregar `main.cpp` como demonio con OBD2 (reutilizar ELM327 de obd2_rpi)
- [ ] Soporte de doble buffer para evitar flicker en actualizaciones parciales
- [ ] Manejo de señal SIGTERM/SIGINT para apagado limpio del display
- [ ] Script de instalación como servicio systemd

## Mediano plazo

- [ ] Soporte para más tamaños de pantalla (2.9", 4.2", 7.5")
- [ ] Fuentes adicionales (negrita, cursiva, iconos)
- [ ] Modo de bajo consumo (deep sleep del display)
- [ ] Soporte de imagen BMP desde archivo
- [ ] Generación de QR en display (ya hay `qr_gen.cpp`)
- [ ] Historial de renderizado con timestamp

## Largo plazo

- [ ] Dashboard OBD2 completo en e-paper (RPM, velocidad, temperatura)
- [ ] Actualización por WiFi (socket server en RPi)
- [ ] App companion Flutter para enviar contenido al display
- [ ] Soporte multitarea con varios displays en cadena SPI
- [ ] Interfaz web de configuración

## Bugs conocidos

- [ ] El constructor no verifica si `new` falla (sin excepciones habilitadas)
- [ ] `drawCenteredString` no funciona correctamente con todas las fuentes (ancho variable no soportado)
- [ ] La detección `CPU_32_BITS` en `config.h` puede fallar en compiladores cruzados

## Arquitectura

- [ ] Separar `epaper_display.cpp` en módulos: `drawing.cpp`, `text.cpp`, `fonts.cpp`
- [ ] Agregar tests unitarios para coordenadas de `drawPixel`
- [ ] CI/CD con GitHub Actions para compilación ARM64
- [ ] Refactorizar Makefile a CMake para mejor portabilidad

# TODO — E-Paper Display para Raspberry Pi

## Corto plazo

- [ ] Agregar `main.cpp` como demonio con OBD2 (reutilizar ELM327 de obd2_rpi)
- [ ] Soporte de doble buffer para evitar flicker en actualizaciones parciales
- [ ] Manejo de señal SIGTERM/SIGINT para apagado limpio del display
- [ ] Script de instalación como servicio systemd

## Mediano plazo

- [ ] Soporte para más tamaños de pantalla (2.9", 4.2", 7.5")
- [ ] Fuentes adicionales (negrita, cursiva, iconos)
- [ ] Modo de bajo consumo (deep sleep del display)
- [ ] Soporte de imagen BMP desde archivo
- [ ] Generación de QR en display (ya hay `qr_gen.cpp`)
- [ ] Historial de renderizado con timestamp

## Largo plazo

- [ ] Dashboard OBD2 completo en e-paper (RPM, velocidad, temperatura)
- [ ] Actualización por WiFi (socket server en RPi)
- [ ] App companion Flutter para enviar contenido al display
- [ ] Soporte multitarea con varios displays en cadena SPI
- [ ] Interfaz web de configuración

## Bugs conocidos

- [ ] El constructor no verifica si `new` falla (sin excepciones habilitadas)
- [ ] `drawCenteredString` no funciona correctamente con todas las fuentes (ancho variable no soportado)
- [ ] La detección `CPU_32_BITS` en `config.h` puede fallar en compiladores cruzados

## Arquitectura

- [ ] Separar `epaper_display.cpp` en módulos: `drawing.cpp`, `text.cpp`, `fonts.cpp`
- [ ] Agregar tests unitarios para coordenadas de `drawPixel`
- [ ] CI/CD con GitHub Actions para compilación ARM64
- [ ] Refactorizar Makefile a CMake para mejor portabilidad
