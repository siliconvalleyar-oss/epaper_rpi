# Gestión de GPIO

## Infraestructura GPIO

El proyecto proporciona dos capas de abstracción para GPIO:

1. **GPIO::Gpio_t**: Clase genérica con gestión de colecciones
2. **EPAPER::Gpio_t**: Clase base del driver EPD, wrapper directo de bcm2835

## Clase GPIO::Gpio_t

### Descripción

Gestor de GPIO que wrapper tanto la librería bcm2835 como sysfs. Soporta colecciones de pines con IDs auto-incrementales.

### Constructores

```cpp
explicit Gpio_t(bool& status);
```

**Parámetros**:
- `status`: Referencia a bool que indica estado de inicialización bcm2835

**Comportamiento**:
- Si `bcm2835_init()` falla: `status = false`, error en stderr
- Si `bcm2835_init()` éxito: `status = true`

### Gestión de GPIOs

#### addGpio()

```cpp
void addGpio(uint16_t gpio_pin, std::string dir, std::string edge, std::string value);
```

Agrega un GPIO a la colección gestionada.

**Parámetros**:
- `gpio_pin`: Número de GPIO BCM
- `dir`: `DIR_IN` ("in") o `DIR_OUT` ("out")
- `edge`: "rising", "falling", "none" (para interrupciones futuras)
- `value`: `VALUE_HIGH` ("1") o `VALUE_LOW` ("0")

**Ejemplo**:
```cpp
gpio.addGpio(25, GPIO::DIR_IN, "none", "0");
gpio.addGpio(24, GPIO::DIR_OUT, "none", "1");
```

#### pinMode()

```cpp
int pinMode(uint16_t gpio_pin, std::string_view direction);
```

Configura la dirección de un GPIO individual.

**Parámetros**:
- `gpio_pin`: Número de GPIO
- `direction`: `DIR_OUT` ("out") o `DIR_IN` ("in")

**Retorna**: 0 en éxito, -1 en error o si bcm2835 no está inicializado

#### digitalWrite()

```cpp
int digitalWrite(uint16_t pin, std::string_view st);
```

Escribe un valor digital.

**Parámetros**:
- `pin`: Número de GPIO
- `st`: `VALUE_HIGH` ("1") o `VALUE_LOW` ("0")

**Retorna**: 0 en éxito, -1 en error

#### digitalRead()

```cpp
int digitalRead(int pin);
```

Lee el valor actual de un GPIO.

**Parámetros**:
- `pin`: Número de GPIO

**Retorna**: 1 (HIGH), 0 (LOW), -1 (error)

#### gpio_get_fd_to_value()

```cpp
int gpio_get_fd_to_value(int gpio_num);
```

Lectura alternativa mediante sysfs (no usada actualmente en el driver EPD).

**Parámetros**:
- `gpio_num`: Número de GPIO

**Retorna**: 0, 1 o -1

**Nota**: Abre el archivo sysfs, lee un character, cierra inmediatamente.

### Gestión de Colecciones

#### getNextId()

```cpp
int getNextId();
```

Genera IDs auto-incrementales para nuevos GPIOs.

**Lógica**:
```cpp
int max_id = -1;
for (const auto& gpioPtr : m_gpio_cfg) {
    if (gpioPtr->ID > max_id) max_id = gpioPtr->ID;
}
return max_id + 1;
```

#### printGpios()

```cpp
void printGpios() const;
```

Imprime todos los GPIOs en formato:
```
ID: 0, GPIO: 25, Direction: in, Edge: none, Value: 0
ID: 1, GPIO: 24, Direction: out, Edge: none, Value: 1
```

#### updateGpioMaps()

```cpp
void updateGpioMaps();
```

Reconstruye los mapas internos `gpioById` y `gpioByPin` después de modificaciones en la colección.

#### CloseGpios()

```cpp
void CloseGpios();
```

Cierra bcm2835 y libera recursos.

## Estructura GpioConform_t

```cpp
struct GpioConform_t {
    int ID;              // ID auto-incremental
    uint16_t gpio;       // Número GPIO BCM
    std::string dir;     // "in" o "out"
    std::string edge;    // Interrupción (reservado)
    std::string value;   // Estado actual ("0" o "1")
    bool status;         // Estado de configuración
};
```

**Nota**: Las operaciones de copia están eliminadas. Solo se permiten moves.

## Clase EPAPER::Gpio_t

### Descripción

Clase base de `EPD_Driver`. Proporciona acceso directo a GPIOs sin colección.

### Constructores

```cpp
Gpio_t(bool enable);
```

**Parámetros**:
- `enable`: Flag de habilitación

### Métodos

#### pinMode()

```cpp
void pinMode(uint16_t pin, uint8_t mode);
```

Configura un pin como entrada o salida usando constantes bcm2835.

**Parámetros**:
- `pin`: GPIO BCM
- `mode`: `INPUT` (BCM2835_GPIO_FSEL_INPT) o `OUTPUT` (BCM2835_GPIO_FSEL_OUTP)

**Ejemplo**:
```cpp
pinMode(25, INPUT);  // GPIO25 como entrada
pinMode(24, OUTPUT); // GPIO24 como salida
```

#### digitalWrite()

```cpp
void digitalWrite(uint16_t pin, uint8_t value);
```

Escribe un valor digital.

**Parámetros**:
- `pin`: GPIO BCM
- `value`: `HIGH` (0x1) o `LOW` (0x0)

#### digitalRead()

```cpp
int digitalRead(uint16_t pin);
```

Lee un pin GPIO.

**Parámetros**:
- `pin`: GPIO BCM

**Retorna**: 0 o 1

## Uso en el Driver EPD

### Configuración en COG_initial()

```cpp
void EPD_Driver::COG_initial() {
    // Pines de entrada
    pinMode(pin_cfg_epaper.panelBusy, INPUT);

    // Pines de salida
    pinMode(pin_cfg_epaper.panelDC, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);

    pinMode(pin_cfg_epaper.panelReset, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);

    pinMode(pin_cfg_epaper.panelCS, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);

    // Pines opcionales (EXT2)
    if (pin_cfg_epaper.panelON_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelON_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelON_EXT2, HIGH);
    }

    if (pin_cfg_epaper.panelSPI43_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelSPI43_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelSPI43_EXT2, LOW);
    }
}
```

### Lectura de BUSY

```cpp
// En displayRefresh()
while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);

// En softReset()
while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
```

**Mecanismo**: Polling activo hasta que BUSY = HIGH (COG listo).

## Arquitectura de Pines en Hardware

### Flujo de Señales COG

```
RPi GPIO                    COG Interno
     |                            |
     |--- panelDC --------------> | D/C (Data/Command)
     |                            |     LOW  = Comando
     |                            |     HIGH = Dato
     |
     |--- panelCS --------------> | CS (Chip Select)
     |                            |     LOW  = Seleccionado
     |                            |     HIGH = Deseleccionado
     |
     |--- panelReset -----------> | RES#
     |                            |     Pulso LOW = Reset
     |
     |--- panelBusy <------------ | BUSY
     |                            |     HIGH = Listo
     |                            |     LOW  = Ocupado
     |
     |--- SPI MOSI --------------> | SDIN
     |--- SPI SCK --------------> | SCLK
     |<-- SPI MISO -------------- | SDOUT
```

### Estados de Pines

| Estado | panelDC | panelCS | panelReset | panelBusy |
|--------|---------|---------|------------|-----------|
| Reposo | HIGH | HIGH | HIGH | LOW |
| Comando SPI | LOW | LOW | HIGH | X |
| Dato SPI | HIGH | LOW | HIGH | X |
| Reset activo | X | HIGH | LOW | X |
| COG ocupado | X | X | X | LOW |
| COG listo | X | X | X | HIGH |

## Manejo de Errores

### Detección de Fallos

```cpp
Gpio_t::Gpio_t(bool& status) {
    if (!bcm2835_init()) {
        std::cerr << "Error iniciando bcm2835" << std::endl;
        m_state = false;
    } else {
        m_state = true;
    }
}
```

### Validación de Pines

```cpp
void Gpio_t::digitalWrite(uint16_t pin, std::string_view st) {
    if (!m_state) return -1;  // bcm2835 no inicializado
    if (st == VALUE_HIGH) bcm2835_gpio_write(pin, HIGH);
    else bcm2835_gpio_write(pin, LOW);
}
```

### Recuperación

```cpp
void Gpio_t::CloseGpios() {
    if (m_state) {
        bcm2835_close();
        m_state = false;
    }
}
```

## Sysfs GPIO (Alternativo)

El proyecto incluye alternativas de acceso por sysfs:

```cpp
std::string path = std::string(SYSFS_GPIO_PATH)
    + "/gpio" + std::to_string(gpio_num)
    + SYSFS_GPIO_VALUE;

int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
char buf[2] = {0};
ssize_t n = read(fd, buf, 1);
close(fd);
```

**Limitaciones de sysfs**:
- Más lento que acceso directo a memoria (bcm2835)
- Requiere export previo (`/sys/class/gpio/export`)
- Menos adecuado para tiempo real

## Optimización de Acceso

### Minimización de syscalls

```cpp
// Ineficiente (cambio uno por uno)
bcm2835_gpio_write(pin, HIGH);
bcm2835_gpio_write(pin, LOW);

// Eficiente (agrupado)
// En sendIndexData():
digitalWrite(pin_cfg_epaper.panelDC, LOW);
digitalWrite(pin_cfg_epaper.panelCS, LOW);
hV_HAL_SPI_transfer(index);
digitalWrite(pin_cfg_epaper.panelCS, HIGH);
digitalWrite(pin_cfg_epaper.panelDC, HIGH);
```

### Cache de Pines

```cpp
// Guardar configuración en struct pins_t
const pins_t pin_cfg_epaper = board;  // Copiado al constructor
```

Acceso O(1) por campo estático, sin búsquedas en mapa.

## Seguridad

### Permisos Requeridos

```
Acceso a /dev/mem (bcm2835):           root
Acceso a /dev/spidev0.0:               root/dialout
Escritura a /sys/class/gpio:          root
```

### Configuración de Usuario

```bash
# Agregar usuario a grupo dialout (acceso SPI)
sudo usermod -aG dialout $USER

# Agregar usuario a grupo gpio (acceso sysfs)
sudo usermod -aG gpio $USER
```

### Restricciones de Pines Reservados

Algunos GPIOs están reservados para funcionalidad del sistema:

| GPIO | Función del Sistema | Reservado |
|------|---------------------|-----------|
| 0-1  | I2C (SDA, SCL) | Si |
| 2-3  | I2C/I2S | Si |
| 7-11 | SPI/UART | Si |
| 14-15 | UART | Si |
| 28-45 | EEPROM/I2C | Si |

El script `menuConfigGpiosSuccess.sh` incluye una lista de pines reservados.

## Pruebas

### Verificación Manual

```bash
# Listar GPIOs
sudo raspi-gpio get 25

# Configurar temporalmente
sudo raspi-gpio set 25 op dh  # Salida, HIGH
sudo raspi-gpio set 25 ip pu  # Entrada, pull-up
```

### Scripts del Proyecto

```bash
# Verificar pines específicos del E-Paper
sudo ./bash/verifGpios.sh settings

# Configurar todos los GPIO como entrada
sudo ./bash/menuConfigGpiosSuccess.sh set

# Probar un pin individual
sudo ./bash/uniquePinGpio.sh 22
```

## Variables de Entorno

No hay variables de entorno específicas para el subsistema GPIO. La configuración es totalmente estática mediante el struct `pins_t`.
