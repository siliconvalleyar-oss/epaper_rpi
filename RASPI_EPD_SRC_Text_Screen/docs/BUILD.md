# Compilación e Instalación

## Dependencias

```bash
# bcm2835 (librería de acceso directo a GPIO/SPI del BCM2835)
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
tar xzf bcm2835-1.71.tar.gz
cd bcm2835-1.71
./configure
make
sudo make install
sudo ldconfig

# qrencode (generación de códigos QR)
sudo apt-get install libqrencode-dev
```

## Compilar

```bash
make            # Compilar
make clean      # Limpiar obj/ y bin/
make run        # Compilar y ejecutar (sudo)
```

## Compilación remota (desde otra máquina)

```bash
sshpass -p zero ssh pi@raspi.local \
  "cd /home/pi/src/epaper_rpi && git pull && \
   make -C Master clean && make -C Master && \
   make -C Master run"
```

## Salida

```
bin/epaper_app    # Binario compilado
obj/              # Archivos objeto
```

## Flags de compilación

- C++20 (`-std=c++20`)
- Warnings: `-Wall -pedantic`
- Debug: `-g`
- Include paths: `-Ilibs -Isrc`
- Librerías: `-lbcm2835 -lqrencode -pthread`

## Verificar bcm2835

```bash
# Verificar que bcm2835 está instalado
ldconfig -p | grep bcm2835

# Compilar un test mínimo
echo '#include <bcm2835.h>
int main() { bcm2835_init(); bcm2835_close(); return 0; }' > test.c
gcc test.c -lbcm2835 -o test && sudo ./test && echo "OK"
```
