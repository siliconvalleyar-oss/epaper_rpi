# Protocolo SPI

## Visión General

La comunicación entre Raspberry Pi y el controlador E-Paper COG se realiza mediante el bus SPI (Serial Peripheral Interface). El proyecto implementa dos capas de abstracción SPI:

1. **SPI nativo Linux** (`SPI::Spi_t`): Acceso directo a `/dev/spidev0.0` mediante ioctl
2. **bcm2835 wrapper** (`EPAPER::Spi_t`): Acceso mediante la librería bcm2835

Ambas capas coexisten en el código; la capa nativa se encuentra en `libs/spi/`, mientras que la capa bcm2835 se integra en `libs/epaper/`.

## Parámetros de Configuración SPI

### Configuración por Defecto

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| Velocidad (SCLK) | 1,600,000 Hz (1.6 MHz) | `SPI_SPEED` en `spi.h` |
| Modo SPI | Modo 0 (CPOL=0, CPHA=0) | Datos capturados en flanco de subida |
| Orden de bits | MSB primero | bit7 -> bit0 |
| Bits por palabra | 8 | Estándar para comandos y datos |
| CS Change | 0 | CS se mantiene entre transferencias |

### Justificación de Velocidad

El E-Paper COG tiene limitaciones de timing críticas:
- La velocidad máxima del controlador interno es ~2 MHz
- Se eligió 1.6 MHz para margen de seguridad
- Valores mayores pueden causar corrupción de datos en el frame buffer

## Arquitectura de la Clase SPI

### SPI::Spi_t (Implementación Nativa)

```cpp
struct Spi_t {
    explicit Spi_t();
    ~Spi_t();

    void init();
    void settings_spi();
    void spi_close();

    const uint8_t Transfer1bytes(const uint8_t cmd);
    const uint8_t Transfer2bytes(const uint16_t address);
    const uint8_t Transfer3bytes(const uint32_t address);
    uint32_t get_spi_speed();

private:
    uint8_t m_tx_buffer[LARGE_SECTOR_SIZE];  // 256 bytes
    uint8_t m_rx_buffer[LARGE_SECTOR_SIZE];
    const uint32_t m_spi_speed;
    int fs;           // File descriptor de /dev/spidev0.0
    int ret;          // Código de retorno de ioctl
    std::unique_ptr<struct spi_ioc_transfer> spi;
};
```

### EPAPER::Spi_t (Implementación bcm2835)

```cpp
class Spi_t {
public:
    Spi_t();
    ~Spi_t();

    uint8_t Transfer1bytes(uint8_t data);

private:
    // Configuración de registros bcm2835
};
```

## Inicialización

### Flujo de Inicialización Nativo

```cpp
Spi_t::Spi_t()
: m_tx_buffer{0x00}
, m_rx_buffer{0x00}
, m_spi_speed{SPI_SPEED}
, spi{std::make_unique<struct spi_ioc_transfer>()}
{
    settings_spi();  // Configura buffers y parámetros
    init();          // Abre dispositivo y aplica config
}
```

**settings_spi()**:
```cpp
spi->tx_buf = (unsigned long)m_tx_buffer;
spi->rx_buf = (unsigned long)m_rx_buffer;
spi->bits_per_word = 0;          // Usar default (8)
spi->speed_hz = m_spi_speed;
spi->delay_usecs = 1;
spi->len = 3;

m_tx_buffer[0] = 0x00;
m_tx_buffer[1] = 0x00;
m_tx_buffer[2] = 0x00;
m_tx_buffer[3] = 0x00;
m_rx_buffer[0] = 0xFF;
m_rx_buffer[1] = 0xFF;
m_rx_buffer[2] = 0xFF;
m_rx_buffer[3] = 0xFF;
```

**init()**:
```cpp
void Spi_t::init() {
    fs = open(SPI_DEVICE, O_RDWR);        // Abre /dev/spidev0.0
    ret = ioctl(fs, SPI_IOC_RD_MODE, &scratch32);
    scratch32 |= SPI_MODE_0;
    ret = ioctl(fs, SPI_IOC_WR_MODE, &scratch32);
    ret = ioctl(fs, SPI_IOC_RD_MAX_SPEED_HZ, &scratch32);
    scratch32 = m_spi_speed;
    ret = ioctl(fs, SPI_IOC_WR_MAX_SPEED_HZ, &scratch32);
}
```

### Flujo de Inicialización bcm2835

```cpp
Spi_t::Spi_t() {
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // ~3.125MHz
}
```

## Transferencias

### Transfer1bytes (1 byte)

Envía y recibe un solo byte.

```cpp
const uint8_t Spi_t::Transfer1bytes(const uint8_t cmd) {
    if (fs < 0) {
        std::cerr << "SPI device not open." << std::endl;
        return -1;
    }

    std::memset(m_rx_buffer, 0xff, LARGE_SECTOR_SIZE);
    std::memset(m_tx_buffer, 0xff, LARGE_SECTOR_SIZE);
    std::memset(spi.get(), 0, sizeof(struct spi_ioc_transfer));

    spi->len = 1;
    m_tx_buffer[0] = cmd;
    spi->tx_buf = reinterpret_cast<unsigned long>(m_tx_buffer);
    spi->rx_buf = reinterpret_cast<unsigned long>(m_rx_buffer);
    spi->speed_hz = get_spi_speed();
    spi->bits_per_word = 8;
    spi->cs_change = 0;
    spi->delay_usecs = 0;

    int ret = ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    if (ret < 0) {
        std::cerr << "Error en Transfer1bytes: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}
```

**Nota**: La versión actual retorna 0 siempre, ignorando el byte recibido.

### Transfer2bytes (2 bytes)

```cpp
const uint8_t Spi_t::Transfer2bytes(const uint16_t cmd) {
    spi->len = sizeof(cmd);
    m_rx_buffer[0]=m_rx_buffer[1]=0xff;
    m_rx_buffer[2]=m_rx_buffer[3]=0x00;
    memcpy(m_tx_buffer, &cmd, sizeof(cmd));
    ret = ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    return m_rx_buffer[1];
}
```

**Uso**: Comandos extendidos de 16 bits.

### Transfer3bytes (3 bytes)

```cpp
const uint8_t Spi_t::Transfer3bytes(const uint32_t cmd) {
    spi->len = 3;
    m_rx_buffer[0]=m_rx_buffer[1]=m_rx_buffer[2]==0xff;
    m_rx_buffer[3]=0x00;
    memcpy(m_tx_buffer, &cmd, sizeof(cmd));
    ret = ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    return m_rx_buffer[2];
}
```

**Uso**: Comandos extendidos de 24 bits (acceso a memoria Flash).

## Protocolo de Comandos

### Formato General

```
┌─────────┬─────────────────────────────┐
│  DC=0   │ CS=0                        │ -> Selección de comando
├─────────┼─────────────────────────────┤
│  CMD    │ (1 byte, 0x00 - 0xFF)       │ -> Comando por SPI
├─────────┼─────────────────────────────┤
│ CS=1    │ DC=1                        │ -> Fin de comando
├─────────┼─────────────────────────────┤
│ CS=0    │                             │ -> Selección de datos
├─────────┼─────────────────────────────┤
│ DATA... │ (N bytes)                   │ -> Datos por SPI
├─────────┼─────────────────────────────┤
│ CS=1    │                             │ -> Fin de datos
└─────────┴─────────────────────────────┘
```

### Tabla de Comandos

| CMD | Nombre | Descripción | Parámetros |
|-----|--------|-------------|------------|
| 0x00 | Soft Reset | Reinicia el COG | 1 byte: PSR |
| 0x02 | Power OFF | Apaga DC/DC | 0 bytes |
| 0x04 | Power ON | Enciende DC/DC | 0 bytes |
| 0x10 | First Frame | Carga frame negro | N bytes (5,624) |
| 0x12 | Display Refresh | Refresca pantalla | 0 bytes |
| 0x13 | Second Frame | Carga frame rojo | N bytes (5,624) |
| 0xE0 | Active Temp | Temperatura activa | 1 byte |
| 0xE5 | Input Temp | Temperatura entrada | 1 byte |
| 0x00 (PSR) | Panel Setting | Configura panel | 2 bytes |

### Secuencia de Comandos de Inicialización

```cpp
// Soft reset
sendIndexData(0x00, &register_data[1], 1);

// Temperatura
sendIndexData(0xE5, &register_data[2], 1);  // Input Temperature: 25°C
sendIndexData(0xE0, &register_data[3], 1);  // Active Temperature

// Panel Setting Register
sendIndexData(0x00, &register_data[4], 2);  // PSR
```

Registros por defecto (`register_data_sm`):
```
[0x00, 0x0E, 0x19, 0x02, 0xCF, 0x8D]
  |     |     |     |     |     |
  |     |     |     |     +----- PSR bajo
  |     |     |     +----------- PSR alto
  |     |     +----------------- Temperatura activa (25°C)
  |     +----------------------- Temperatura entrada (25°C)
  +----------------------------- Comando soft reset
```

### Secuencia de Actualización Global

Comandos enviados por `globalUpdate()`:

```cpp
// Frame 1 (negro/anterior)
sendIndexData(0x10, data1s, image_data_size);

// Frame 2 (rojo/nuevo)
sendIndexData(0x13, data2s, image_data_size);

// Encender DC/DC
sendIndexData(0x04, &register_data[0], 1);
while (digitalRead(BUSY) != HIGH);

// Refrescar
sendIndexData(0x12, &register_data[0], 1);
while (digitalRead(BUSY) != HIGH);
```

### Secuencia de Apagado

```cpp
// Apagar DC/DC
sendIndexData(0x02, &register_data[0], 0);
while (digitalRead(BUSY) != HIGH);

// Limpiar líneas
digitalWrite(DC, LOW);
digitalWrite(CS, LOW);
digitalWrite(BUSY, LOW);

// Esperar estabilización
TYME::delay_ms(150);

// Reset final
digitalWrite(RESET, LOW);
```

## Estructura de Datos

### Frame Buffer

Para pantalla 2.66" (296x152):

```
Tamaño total: 5,624 bytes
Estructura:   152 filas x 37 bytes/fila

Byte layout:
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│ b7   │ b6   │ b5   │ b4   │ b3   │ b2   │ b1   │ b0   │
├──────┼──────┼──────┼──────┼──────┼──────┼──────┼──────┤
│ P+7  │ P+6  │ P+5  │ P+4  │ P+3  │ P+2  │ P+1  │ P+0  │
└──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
```

Donde:
- P = pixel position (0 a 295)
- b7 = pixel más significativo (izquierda)
- b0 = pixel menos significativo (derecha)

### Buffer Doble (BWR)

Los modos bicolor requieren dos frames:

```cpp
// Frame 1: Canal negro
uint8_t frame_black[5624];

// Frame 2: Canal rojo
uint8_t frame_red[5624];

// Actualización completa
epaper->globalUpdate(frame_black, frame_red);
```

## Modos de Operación

### Modo SPI_MODE0

```
CPOL = 0 (Clock inactivo en bajo)
CPHA = 0 (Datos capturados en primer flanco)

SCLK:    ───┐   ┌───┐   ┌───┐   ┌───
             └───┘   └───┘   └───┘

MOSI:    ───<───────><───────><─────
               D0      D1      D2

MISO:    ───<───────><───────><─────
               R0      R1      R2
```

### Control CS

El control de Chip Select es manual en la implementación:

```cpp
// Comando
digitalWrite(DC, LOW);
digitalWrite(CS, LOW);
hV_HAL_SPI_transfer(cmd);
digitalWrite(CS, HIGH);
digitalWrite(DC, HIGH);
digitalWrite(CS, LOW);

// Datos
for (uint32_t i = 0; i < len; i++) {
    hV_HAL_SPI_transfer(data[i]);
}
digitalWrite(CS, HIGH);
```

## Timeouts y Delays

### Delays Críticos

| Etapa | Duración | Causa |
|-------|----------|-------|
| RESET pulse HIGH | 5 ms | Estabilización VCC |
| RESET pulse LOW | 10 ms | Reset interno COG |
| CS después de reset | 5 ms | Estabilización SPI |
| CS entre frames | 1 ms | Tiempo de procesamiento COG |
| DC/DC OFF delay | 150 ms | Descarga de capacitores |
| BUSY polling | Variable | Hasta 900 ms en prácticas |

### Cálculo de Tiempo de Actualización

Para pantalla 2.66":
```
T_total = T_init + T_frame1 + T_frame2 + T_dcdc + T_refresh

Donde:
- T_init = 5 + 5 + 10 + 5 + 5 = 30 ms (reset) + soft_reset
- T_frame1 = 5624 bytes / (1.6 MHz / 8) ≈ 28 ms
- T_frame2 = 28 ms
- T_dcdc ≈ 32 ms (espera BUSY)
- T_refresh ≈ 300-900 ms (depende de temperatura y contenido)
```

## Dispositivos SPI en Linux

### /dev/spidev0.0

El proyecto utiliza el bus SPI0 con Chip Enable 0:

```bash
# Verificar existencia
ls -la /dev/spidev*

# Configuración overlay en /boot/config.txt
dtoverlay=spi0-1cs,cs0_pin=8
```

### ioctl Utilizados

```
SPI_IOC_RD_MODE        Lee modo SPI
SPI_IOC_WR_MODE        Escribe modo SPI
SPI_IOC_RD_MAX_SPEED_HZ   Lee velocidad máxima
SPI_IOC_WR_MAX_SPEED_HZ   Escribe velocidad máxima
SPI_IOC_MESSAGE(1)     Transfiere mensaje SPI
```

### Modos de Error

```cpp
// Errores al abrir dispositivo
if (fs < 0) {
    printf("Could not open the Spi device...\r\n");
    exit(EXIT_FAILURE);
}

// Errores en ioctl
if (ret != 0) {
    close(fs);
    exit(EXIT_FAILURE);
}
```

El diseño actual usa `exit(EXIT_FAILURE)` para errores SPI fatales. Esto es apropiado para sistemas embebidos donde el SPI es crítico.

## Mejoras Potenciales

1. **Retry logic**: En lugar de exit(), reintentar en caso de error transitorio
2. **DMA transfers**: Usar DMA para transfers grandes (5,624 bytes)
3. **Configurable SPI speed**: Permitir cambiar velocidad dinámicamente por pantalla
4. **Timeout BUSY**: Agregar timeout a la espera de BUSY para evitar bucles infinitos
