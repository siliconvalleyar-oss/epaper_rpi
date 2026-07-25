# E-Paper Dino Jump Game (v1.0.7)

## Descripción
Juego tipo "Chrome T-Rex" para pantalla e-paper 2.66" (296x152). Un dinosaurio salta sobre cactus que se desplazan de derecha a izquierda.

## Controles
- **Botón GPIO4**: Saltar
- **Ctrl+C**: Salir del juego
- ** gameOver**: Presionar botón para reiniciar

## Características
- Dinosaurio con sprites pixel art
- Cactus de diferentes tamaños
- Score que aumenta con el tiempo
- Velocidad que incrementa progresivamente
- Nubes decorativas
- Transiciones con fast update (~1s)
- Game over con opción de reinicio

## Física del Juego
- Gravedad: 1 px/frame²
- Fuerza de salto: -12 px/frame
- Velocidad inicial: 3 px/frame
- Velocidad máxima: 8 px/frame

## Hardware Requerido
- Raspberry Pi Zero 2W
- E-Paper 2.66" (296x152, BW)
- Botón conectado a GPIO4 (con pull-up)

## Compilación
```bash
make clean
make -j4
```

## Ejecución
```bash
sudo ./bin/dino_game
```

## Estructura
```
epaper_success_v1.0.7/
├── src/main.cpp          # Main loop + input handling
├── libs/
│   ├── game/
│   │   ├── dino_game.h   # Game class definition
│   │   └── dino_game.cpp # Game logic + rendering
│   ├── epaper/           # EPD driver (from v1.0.5)
│   ├── fonts/            # Font rendering
│   ├── tyme/             # Timing utilities
│   └── gpio/             # GPIO control
├── Makefile
└── README.md
```
