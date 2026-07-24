# Compilación Remota en Raspberry Pi



USER="admin"
HOSTNAME="hostname"



## Prerrequisitos

### En la Raspberry Pi

1. Instalar dependencias:
```bash
sudo apt-get update
sudo apt-get install -y build-essential libbcm2835-dev libqrencode-dev
```

2. Habilitar SSH:
```bash
sudo raspi-config
# Interface Options -> SSH -> Enable
```

3. Configurar usuario y contraseña (ejemplo):
```bash
# Usuario de ejemplo: pi
# Contraseña de ejemplo: raspberry
sudo passwd pi
```

### En tu máquina local

1. Instalar SSH client (si no lo tienes):
```bash
# Linux/Mac
sudo apt-get install openssh-client

# Windows (PowerShell)
Add-WindowsCapability -Online -Name OpenSSH.Client~~~~0.0.1.0
```

2. Configurar acceso sin contraseña (opcional pero recomendado):
```bash
# Generar clave SSH
ssh-keygen -t rsa -b 4096

# Copiar clave a la Raspberry
ssh-copy-id $USER@$HOSTNAME
```





## Compilación y Ejecución Remoto

### Comando básico

```bash
ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && git pull && make clean && make -j4 && sudo make run"
```

### Desglose del comando

| Parte | Descripción |
|-------|-------------|
| `ssh $USER@$HOSTANME` | Conectar a la Raspberry Pi |
| `cd /home/pi/src/epaper_raspberry` | Ir al directorio del proyecto |
| `git pull` | Actualizar código desde el repositorio |
| `make clean` | Limpiar archivos compilados anteriores |
| `make -j4` | Compilar usando 4 cores del procesador |
| `sudo make run` | Ejecutar el programa (necesita root para GPIO/SPI) |

### Variaciones útiles

```bash

# Solo compilar (sin ejecutar)
ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && git pull && make clean && make -j4"

# Ejecutar sin recompilar
ssh  $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && sudo ./bin/epaper_app"

# Compilar con modo debug
ssh  $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && make clean && make -j4 DEBUG=1"

# Ver logs de compilación
ssh  $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && make clean && make -j4 V=1"
```

## Solución de Problemas

### Error: "Permission denied"
```bash
# Asegúrate de usar sudo para ejecutar
sudo ./bin/epaper_app
```

### Error: "bcm2835: Unable to open /dev/mem"
```bash
# Necesitas permisos de root
sudo ./bin/epaper_app
```

### Error: "Segmentation fault"
Causas comunes:
1. Falta `bcm2835_spi_begin()` - Verificar que el código incluya esta llamada
2. Panel e-Paper no conectado - Verificar conexiones
3. Permisos insuficientes - Ejecutar con `sudo`

### Error: "git pull" falla
```bash
# Verificar conexión a internet
ping google.com

# Verificar que el repositorio existe
ls -la /home/pi/src/epaper_raspberry
```

## Estructura del Proyecto

```
epaper_raspberry/
├── src/
│   └── main.cpp          # Punto de entrada
├── libs/
│   ├── epaper/           # Driver e-Paper
│   ├── fonts/            # Sistema de fuentes
│   ├── gpio/             # Manejo de GPIO
│   ├── spi/              # Comunicación SPI
│   └── qr/               # Generación de QR
├── docs/                 # Documentación
├── Makefile              # Sistema de compilación
└── README.md             # Información general
```

## Ejemplo de Flujo Completo

```bash
# 1. Clonar el repositorio (primera vez)
git clone https://github.com/siliconvalleyar-oss/epaper_raspberry.git
cd epaper_raspberry

# 2. Compilar y ejecutar remotamente
ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_raspberry && git pull && make clean && make -j4 && sudo make run"

# 3. Ver resultados en la pantalla e-Paper
```

## Notas Importantes

- **Siempre ejecutar con `sudo`** para acceder a GPIO y SPI
- **Verificar conexiones físicas** antes de ejecutar
- **El programa espera 5 segundos** para mostrar resultados
- **Usar `bcm2835_spi_begin()`** después de `bcm2835_init()` para inicializar SPI




compilacion remota
PASSWORD=passowrd

# Para compilar origin/
FOLDER="origin"
sshpass -p $PASSWORD ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_rpi && git pull && make -C ${FOLDER} clean && make -C ${FOLDER} && make -C ${FOLDER} run"

# Para compilar epaper_success/
FOLDER="epaper_success"
sshpass -p $PASSWORD ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_rpi && git pull && make -C ${FOLDER} clean && make -C ${FOLDER} && make -C ${FOLDER} run"

# Para compilar epaper_success_2026_1.0.1/
FOLDER="epaper_success_2026_1.0.1"
sshpass -p $PASSWORD ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_rpi && git pull && make -C ${FOLDER} clean && make -C ${FOLDER} && make -C ${FOLDER} run"

# Para compilar gpio_spi_test/
FOLDER="gpio_spi_test"
sshpass -p $PASSWORD ssh $USER@$HOSTNAME "cd /home/pi/src/epaper_rpi && git pull && make -C ${FOLDER} clean && make -C ${FOLDER} && sudo ./gpio_spi_test --smoke"
