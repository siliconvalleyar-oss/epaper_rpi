# SKILL: Build System y Deploy Remoto

## 🏗️ Sistema de Compilación (Makefile)

### Target por defecto

```bash
make          # Compila todo → bin/epaper_app
make run      # Compila y ejecuta con sudo
make clean    # Limpia obj/ y bin/
```

### Flags de compilación

```makefile
CC = g++
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
LIBRARIES = -pthread -lqrencode -lbcm2835
```

### Dependencias del sistema

```bash
# En Raspberry Pi:
sudo apt-get install libraspberrypi-dev   # para bcm2835.h
sudo apt-get install libqrencode-dev      # para QR codes
```

### Cómo funciona el Makefile

1. **Busca todos los `.cpp`**: en `src/` y recursivamente en `libs/*/`
2. **Compila cada uno** a `obj/<nombre>.o`
3. **Linkea todo** en `bin/epaper_app`
4. **Genera dependencias** automáticas (archivos `.d`) para recompilar solo lo necesario

### Targets adicionales (no usados)

```bash
make tx    # → bin/epaper_app_tx
make rx    # → bin/epaper_app_rx
```

Estos targets no se usan realmente, pero están definidos en el Makefile.

### Compilar con debug

```bash
# En línea de comandos:
g++ -std=c++20 -DDBG_EPAPER -DDBG_SPI -Ilibs -Isrc -c src/main.cpp

# O editando Makefile:
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g -DDBG_EPAPER
```

---

## 🚀 Deploy Remoto a Raspberry Pi

### Prerrequisitos

- Raspberry Pi accesible vía `ssh pi@raspi.local`
- `sshpass` instalado en la máquina de desarrollo:
  ```bash
  sudo apt-get install sshpass
  ```
- Git configurado en la Raspberry Pi:
  ```bash
  # En la Pi:
  cd /home/pi/src/epaper_rpi
  git init
  git remote add origin <url>
  ```

### Usar `compile_remote.sh`

```bash
# En la máquina de desarrollo:
./compile_remote.sh
```

El script:
1. Hace `git pull` en la Raspberry Pi
2. Ejecuta `make clean && make` en la carpeta especificada
3. Ejecuta el binario

### ⚠️ Seguridad

El archivo `compile_remote.sh` contiene la contraseña SSH en texto plano (`sshpass -p zero`).

**Protegido por `.gitignore`** — no se sube al repositorio.

**Para mejorar seguridad**, usar variables de entorno:
```bash
# En ~/.bashrc:
export PI_PASSWORD="tu_password"

# En compile_remote.sh:
sshpass -p "${PI_PASSWORD}" ssh pi@raspi.local "..."
```

### Manual: Copiar archivos a la Pi

```bash
# Opción 1: rsync (solo archivos modificados)
rsync -avz --exclude='bin/' --exclude='obj/' \
  /ruta/local/epaper_success_v1.0.1/ \
  pi@raspi.local:~/src/epaper_rpi/epaper_success_v1.0.1/

# Opción 2: scp (archivos específicos)
scp libs/epaper/epaper.cpp pi@raspi.local:~/src/epaper_rpi/libs/epaper/
scp src/main.cpp pi@raspi.local:~/src/epaper_rpi/src/
```

### Compilar y ejecutar manualmente en la Pi

```bash
ssh pi@raspi.local "cd ~/src/epaper_rpi/epaper_success_v1.0.1 && \
  make clean && make -j4 && sudo ./bin/epaper_app"
```

---

## 📁 Estructura de Directorios (Consistente entre proyectos)

```
proyecto/
├── src/
│   └── main.cpp              # Punto de entrada
├── libs/
│   ├── epaper/
│   │   ├── epaper.h          # Driver EPD (header)
│   │   ├── epaper.cpp        # Driver EPD (implementación)
│   │   ├── epaper_display.h  # Capa de dibujo (opcional)
│   │   ├── epaper_display.cpp
│   │   └── boards.h          # Configuraciones de pines
│   ├── spi/
│   │   └── spi.h/.cpp        # SPI por ioctl (legacy, no usado)
│   ├── gpio/
│   │   └── gpio.h/.cpp       # GPIO por sysfs (legacy)
│   ├── tyme/
│   │   └── tyme.h/.cpp       # Delays
│   ├── qr/
│   │   └── qr_gen.h/.cpp     # Generación QR
│   ├── graphics/
│   │   └── userImageData.h   # Buffers de imagen
│   ├── fonts/                 # Solo en Master/
│   └── app/
│       └── config.h          # Detección CPU 32/64 bits
├── bash/                     # Scripts de utilidad (scripts_tools/)
├── rules/                    # Reglas GDB
├── obj/                      # Objetos compilados (generado)
├── bin/                      # Binarios (generado)
├── Makefile                  # Build system
└── README.md
```

---

## 🧪 Tests

### Test de pines GPIO

En `epaper_success_v1.0.1/src/main.cpp`, descomentar `#define TEST_PINS`:

```cpp
// Descomentar para probar pines GPIO
#define TEST_PINS
```

Luego compilar y ejecutar:
```bash
make clean && make && sudo ./bin/epaper_app
```

### gpio_spi_test (proyecto separado, puede no estar en disco)

Proyecto para test de GPIO y SPI independiente del driver EPD. Si existe en la Raspberry Pi:

```bash
cd gpio_spi_test
make clean && make
sudo ./gpio_spi_test           # Test completo
sudo ./gpio_spi_test --smoke   # Smoke test rápido
```

Si no existe, usar el modo `#define TEST_PINS` en `epaper_success_v1.0.1/` como alternativa.

---

## 🐳 Notas sobre BCM2835

### Inicialización

La librería `bcm2835` requiere:

1. **`bcm2835_init()`** — Mapea `/dev/mem` para acceder a registros GPIO
2. **`bcm2835_spi_begin()`** — Configura pines SPI y mapea registros SPI
3. **`bcm2835_close()`** — Libera memoria y cierra `/dev/mem`
4. **`bcm2835_spi_end()`** — Restaura pines SPI a GPIO

### Orden correcto de cierre

```
Siempre: spi_end() ANTES de close()
Nunca: llamar spi_end() dos veces
```

---

## 📝 Versionado

- `VERSION` en la raíz del proyecto contiene el número de versión
- Debe coincidir con el tag de git (`git tag v1.X.X`)
- Actual: **1.4.3**

```bash
# Verificar coincidencia:
cat VERSION          # → 1.4.3
git describe --tags  # → v1.4.3

# Crear nuevo tag:
echo "1.5.0" > VERSION
git add VERSION && git commit -m "v1.5.0: ..."
git tag v1.5.0
git push origin main --tags
```
