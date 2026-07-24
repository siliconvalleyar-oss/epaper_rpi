# Generación de Códigos QR

## Visión General

El módulo de generación de QR codes permite crear códigos QR dinámicamente y renderizarlos directamente en el buffer de la pantalla E-Paper. La implementación usa la librería `libqrencode` para la codificación.

## Clase Qr_gen_t

```cpp
struct Qr_gen_t {
    void setPixel(int x, int y, bool isBlack);
    void drawQRCode(const char* data, int scaleFactor);
    int qr_generator();

private:
    unsigned char imageBuffer[BYTES_PER_ROW * IMAGE_HEIGHT];
};
```

## Constantes

```cpp
const int IMAGE_WIDTH = 152;
const int IMAGE_HEIGHT = 296;
const int BYTES_PER_ROW = { IMAGE_WIDTH / 8 };   // 19 bytes
const int SCALE = 5;                              // Factor de escalado
```

## Algoritmo de Generación

### Flujo Completo

```cpp
int Qr_gen_t::qr_generator() {
    // 1. Datos a codificar (WiFi WPA)
    const char* networkData = "WIFI:T:WPA;S:SSID;P:Password;;";

    // 2. Limpiar buffer (pantalla blanca)
    std::memset(imageBuffer, 0x00, sizeof(imageBuffer));

    // 3. Definir factor de escala
    int scaleFactor = SCALE;  // 5

    // 4. Generar y dibujar QR
    drawQRCode(networkData, scaleFactor);

    // 5. (Opcional) Imprimir en consola para debugging
    std::cout << "[[maybe_unused]]unsigned char const image_213_212x104_qr[]=\n";
    std::cout << "{\n";
    for (int i = 0; i < IMAGE_HEIGHT; ++i) {
        for (int j = 0; j < BYTES_PER_ROW; ++j) {
            std::cout << "0x" << std::hex << (int)imageBuffer[i * BYTES_PER_ROW + j] << ",";
        }
        std::cout << std::endl;
    }
    std::cout << "\n};\n";

    return 0;
}
```

### Paso 1: Configuración de Formato (libqrencode)

```cpp
QRcode* qrcode = QRcode_encodeString(
    data,           // String de datos
    0,              // Longitud (0 = detectar automáticamente)
    QR_ECLEVEL_L,   // Nivel de corrección de error: Bajo (7%)
    QR_MODE_8,      // Modo: 8-bit bytes
    1               // Case-sensitive
);
```

**Parámetros de QRcode_encodeString**:

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| str | Cadena WiFi | Datos a codificar |
| version | 0 | Auto (ajusta tamaño del QR) |
| level | QR_ECLEVEL_L | Corrección de error baja (7%) |
| hint | QR_MODE_8 | Modo byte 8-bit |
| casesensitive | 1 | Distingue mayúsculas/minúsculas |

**Nota**: `QR_ECLEVEL_L` permite más datos pero menor redundancia. Para información crítica usar `QR_ECLEVEL_H` (30%).

### Paso 2: Centrado del QR

```cpp
int qrSize = qrcode->width;
int scaledQrSize = qrSize * scaleFactor;
int xOffset = (IMAGE_WIDTH - scaledQrSize) / 2;
int yOffset = (IMAGE_HEIGHT - scaledQrSize) / 2;
```

**Ejemplo**:
- QR nativo: 21x21 módulos (versión 1)
- Escala 5: 105x105 píxeles
- Buffer E-Paper: 152x296
- xOffset = (152 - 105) / 2 = 23
- yOffset = (296 - 105) / 2 = 95

### Paso 3: Renderizado por Píxeles

```cpp
for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
        bool isBlack = qrcode->data[y * qrSize + x] & 0x01;
        for (int dy = 0; dy < scaleFactor; ++dy) {
            for (int dx = 0; dx < scaleFactor; ++dx) {
                setPixel(x * scaleFactor + dx + xOffset,
                         y * scaleFactor + dy + yOffset, isBlack);
            }
        }
    }
}
```

**Complejidad**:
- QR versión 1: 21x21 = 441 módulos
- Escala 5: 441 × 25 = 11,025 iteraciones
- Rendimiento: < 10ms en Raspberry Pi

### Paso 4: Seteo de Píxel

```cpp
void Qr_gen_t::setPixel(int x, int y, bool isBlack) {
    if (x < 0 || x >= IMAGE_WIDTH || y < 0 || y >= IMAGE_HEIGHT) return;

    int byteIndex = (y * BYTES_PER_ROW) + (x / 8);
    int bitIndex = 7 - (x % 8);

    if (isBlack) {
        imageBuffer[byteIndex] |= (1 << bitIndex);  // Negro = 1
    } else {
        imageBuffer[byteIndex] &= ~(1 << bitIndex); // Blanco = 0
    }
}
```

**Validación**: Se verifican límites del buffer antes de escribir.

## Formato de Datos WiFi

### Estructura

```
WIFI:T:<auth_type>;S:<SSID>;P:<password>;H:<hidden>;;
```

| Campo | Ejemplo | Descripción |
|-------|---------|-------------|
| T | WPA | Tipo de autenticación (WPA, WEP, nopass) |
| S | MiRedWiFi | Nombre de la red (SSID) |
| P | MiClave123 | Contraseña |
| H | true/false | Red oculta |

### Ejemplos

```cpp
// WPA estándar
"WIFI:T:WPA;S:MiRedWiFi;P:MiClave123;;"

// WEP (inseguro)
"WIFI:T:WEP;S:RedAntigua;P:ClaveWEP;;"

// Red abierta
"WIFI:T:nopass;S:RedAbierta;;"

// Red oculta
"WIFI:T:WPA;S:RedOculta;P:Clave;;H:true;"
```

## Factor de Escala

### Valores Recomendados

| ESCALA | QR 21px | QR 29px | QR 37px |
|--------|---------|---------|---------|
| 3 | 63x63 | 87x87 | 111x111 |
| 4 | 84x84 | 116x116 | 148x148 |
| 5 | 105x105 | **145x145** | **185x185** |
| 6 | 126x126 | 174x174 | 222x222 |

**Mejor valor**: 5
- QR 21px: 105x105 (cómodo en 152x296)
- QR 29px: 145x145 (óptimo para 2.66")
- QR 37px: 185x185 (requiere pantalla ≥ 2.87")

### Cálculo de Versión QR por Capacidad

| Versión | Módulos | Capacidad (L%) | Caracteres alfanuméricos |
|---------|---------|----------------|--------------------------|
| 1 | 21x21 | 17 | 25 |
| 2 | 25x25 | 32 | 47 |
| 3 | 29x29 | 53 | 77 |
| 4 | 33x33 | 78 | 114 |
| 5 | 37x37 | 106 | 154 |
| 6 | 41x41 | 134 | 195 |
| 7 | 45x45 | 154 | 224 |
| 8 | 49x49 | 192 | 279 |

Un string WiFi típico (`WIFI:T:WPA;S:SSID;P:Password;;`) tiene ~40 caracteres, por lo que versión 2+ es suficiente.

## Buffer de Imagen

### Acceso desde main.cpp

```cpp
// Si SCREEN == 266, BW_QrBuffer apunta al QR code generado
// Nota: El QR parece estar vinculado a image_213_212x104_qr
// pero se usa en buffer de 2.66" (5624 bytes)
epaper->globalUpdate(BW_QrBuffer, BW_0x00Buffer);
```

**Nota de implementación**: El macro `BW_QrBuffer` en `userImageData.h` apunta a `image_213_212x104_qr` incluso cuando se compila para SCREEN==266. Esto parece ser un bug o simplificación temporal.

### Tamaño del Buffer QR

```cpp
// imageBuffer[BYTES_PER_ROW * IMAGE_HEIGHT]
// = 19 * 296 = 5,624 bytes
unsigned char imageBuffer[5624] = {0};
```

## Integración con E-Paper

### Secuencia de Actualización con QR

```cpp
// 1. Inicializar COG
epaper->COG_initial();

// 2. Mostrar QR (primer frame)
epaper->globalUpdate(BW_QrBuffer, BW_0x00Buffer);
TYME::delay(900);  // Esperar estabilización

// 3. Limpiar pantalla
epaper->globalUpdate(BW_0x00Buffer, BW_0x00Buffer);

// 4. Mostrar otro contenido
epaper->globalUpdate(BW_monoBuffer, BW_0x00Buffer);
TYME::delay(900);

// 5. Limpiar
epaper->globalUpdate(BW_0x00Buffer, BW_0x00Buffer);

// 6. Mostrar BWR
epaper->globalUpdate(BWR_blackBuffer, BWR_redBuffer);

// 7. Apagar
epaper->COG_powerOff();
```

## Modo Debug

### Impresión en Consola

```cpp
std::cout << "[[maybe_unused]]unsigned char const image_213_212x104_qr[]=\n";
std::cout << "{\n";
for (int i = 0; i < IMAGE_HEIGHT; ++i) {
    for (int j = 0; j < BYTES_PER_ROW; ++j) {
        std::cout << "0x" << std::hex << (int)imageBuffer[i * BYTES_PER_ROW + j] << ",";
    }
    std::cout << std::endl;
}
std::cout << "\n};\n";
```

**Salida**: Array C++ para copiar directamente en `qr_code.cpp`.

**Uso**:
```bash
sudo ./bin/epaper_app > qr_debug.txt
```

## Dependencia Externa

### libqrencode

```bash
sudo apt-get install libqrencode-dev -y
```

**Cabecera**:
```cpp
#include <qrencode.h>
```

**Funciones utilizadas**:
- `QRcode_encodeString()`: Genera código QR desde string
- `QRcode_free()`: Libera memoria del QR generado

## Personalización

### Cambiar SSID/Password

Modificar en `qr_gen.cpp`:
```cpp
const char* networkData = "WIFI:T:WPA;S:MiRedWiFi;P:MiClave123;;";
```

### Cambiar Nivel de Corrección de Error

```cpp
QR_ECLEVEL_L  // 7% - Mínimo
QR_ECLEVEL_M  // 15% - Medio
QR_ECLEVEL_Q  // 25% - Cuartil
QR_ECLEVEL_H  // 30% - Máximo (mejor para daño/ensuciamiento)
```

### Cambiar Modo de Codificación

```cpp
QR_MODE_8     // 8-bit bytes (por defecto)
QR_MODE_NUM   // Solo números
QR_MODE_ALPH  // Alfanumérico
```

## Limitaciones

1. **Solo WiFi**: El ejemplo hardcodea formato WiFi. Puede extenderse a URLs, texto, vCards.
2. **Versión fija**: No especifica versión máxima, pero limitada por IMAGE_WIDTH/HEIGHT.
3. **Single-threaded**: Generación bloquea el hilo principal.
4. **No cache**: Genera nuevo QR en cada ejecución.

## Ejemplo de Uso Avanzado

```cpp
#include <qr/qr_gen.h>

int main() {
    QR::Qr_gen_t qrGen;

    // Configurar red personalizada
    const char* miRed = "WIFI:T:WPA;S:OtraRed;P:ClaveSegura;;";
    
    // Generar QR con escala 4
    qrGen.drawQRCode(miRed, 4);

    // Obtener puntero al buffer
    unsigned char* qrBuffer = qrGen.imageBuffer;

    // Enviar a E-Paper
    epaper->globalUpdate(qrBuffer, BW_0x00Buffer);

    return 0;
}
```

## Escaneo desde Dispositivos Móviles

iOS (Cámara nativa):
- Abrir Cámara
- Apuntar al código QR
- Tocar la notificación

Android (Google Lens):
- Abrir Google Lens o Cámara
- Apuntar al código QR
- Tocar "Unirse a la red"
