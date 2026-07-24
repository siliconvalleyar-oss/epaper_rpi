# Sistema de Compilación

## Visión General

El proyecto utiliza **GNU Make** como sistema de compilación. El archivo `Makefile` principal reside en la raíz del proyecto y gestiona la compilación, linking y limpieza de artefactos.

## Compilador y Estándar

```makefile
CC = g++
# Alternativas:
# CC = clang++
# CC = arm-none-eabi-g++

CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
```

| Flag | Valor | Descripción |
|------|-------|-------------|
| `-std=c++20` | C++20 | Estándar de C++ |
| `-Ilibs` | Ruta include | Cabeceras de librerías |
| `-Isrc` | Ruta include | Cabeceras de aplicación |
| `-Wall` | Warnings | Todos los warnings estándar |
| `-pedantic` | Warnings | Conformance estricta al estándar |
| `-g` | Debug | Información de depuración |

## Librerías Vinculadas

```makefile
LIBRARIES = -pthread -lqrencode -lbcm2835
```

| Librería | Propósito |
|----------|-----------|
| pthread | Threads POSIX |
| qrencode | Generación de códigos QR |
| bcm2835 | Acceso a GPIO y SPI |

## Estructura de Directorios

```
SRC_DIR = src
LIB_DIR = libs
OBJ_DIR = obj
BIN_DIR = bin
```

```makefile
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
LIB_DIRS = $(wildcard $(LIB_DIR)/*)
LIB_SRCS = $(foreach dir,$(LIB_DIRS),$(wildcard $(dir)/*.cpp))
```

## Variables de Origen

### main.cpp

Compilado desde `src/main.cpp`.

### Librerías

Todos los `.cpp` en subdirectorios de `libs/`:

```
libs/tyme/tyme.cpp
libs/epaper/epaper.cpp
libs/gpio/gpio.cpp
libs/spi/spi.cpp
libs/qr/qr_gen.cpp
libs/graphics/*.cpp
```

## Objetos de Compilación

### Generación de Nombres

```makefile
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.cpp=.o) $(notdir $(LIB_SRCS:.cpp=.o))))
```

**Ejemplo**:
```
src/main.cpp            -> obj/main.o
libs/tyme/tyme.cpp      -> obj/tyme.o
libs/epaper/epaper.cpp  -> obj/epaper.o
```

### Regla de Compilación Principal

```makefile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
    $(CC) $(CXXFLAGS) -c $< -o $@ -MMD -MP
    @echo "Compiled: $< -> $@"
```

**Flags de dependencia**:
- `-MMD`: Genera dependencias `.d` excluyendo system headers
- `-MP`: Genera reglas phony para headers eliminados

### Template de Compilación de Librerías

```makefile
define compile_template
$(OBJ_DIR)/%.o: $(LIB_DIR)/$(1)/%.cpp | $(OBJ_DIR)
    $(CC) $(CXXFLAGS) -c $$< -o $$@ -MMD -MP
    @echo "Compiled library: $$< -> $$@"
endef
```

**Evaluación dinámica**:
```makefile
$(foreach libdir,$(LIB_DIRS),$(eval $(call compile_template,$(notdir $(libdir)))))
```

Genera automáticamente reglas para:
- `libs/tyme/` -> patron `$(OBJ_DIR)/%.o: $(LIB_DIR)/tyme/%.cpp`
- `libs/epaper/` -> patron `$(OBJ_DIR)/%.o: $(LIB_DIR)/epaper/%.cpp`

## Nombres de Binarios

### Target por Defecto

```makefile
N_APP = epaper_app
DEFAULT_TARGET = ${N_APP}
```

### Target desde CLI

```makefile
TARGET ?= $(DEFAULT_TARGET)

ifeq ($(TARGET), tx)
    APP = $(BIN_DIR)/${N_APP}_tx
else ifeq ($(TARGET), rx)
    APP = $(BIN_DIR)/${N_APP}_rx
else
    APP = $(BIN_DIR)/$(DEFAULT_TARGET)
endif
```

**Binarios generados**:

| Comando | Binario |
|---------|---------|
| `make` | bin/epaper_app |
| `make tx` | bin/epaper_app_tx |
| `make rx` | bin/epaper_app_rx |

## Reglas de Compilación

### Regla Principal

```makefile
all: $(APP)

$(APP): $(OBJS) | $(BIN_DIR)
    $(CC) $(CXXFLAGS) -o $@ $^ $(LIBRARIES)
    @echo "Binary created: $@"
```

**Variables especiales**:
- `$@`: Nombre del target (`bin/epaper_app`)
- `$^`: Todos los prerequisitos (`obj/main.o obj/tyme.o ...`)

## Directorios de Salida

```makefile
$(BIN_DIR) $(OBJ_DIR):
    @test -d $@ || mkdir -p $@
```

Crea `bin/` y `obj/` solo si no existen.

## Regla de Ejecución

```makefile
run: $(APP)
    sudo $<
```

Ejecuta el binario con `sudo`.

**Uso**:
```bash
make run
```

Equivale a:
```bash
sudo ./bin/epaper_app
```

## Regla de Limpieza

```makefile
clean:
    rm -rf $(OBJ_DIR) $(BIN_DIR)
    rm -f log/*
```

Elimina:
- `obj/` (todos los objetos)
- `bin/` (todos los binarios)
- `log/*` (archivos de log)

## Dependencias Automáticas

```makefile
-include $(OBJS:.o=.d)
```

Incluye archivos `.d` generados por `-MMD -MP`.

**Ejemplo de archivo .d**:
```
obj/main.o: src/main.cpp libs/epaper/epaper.h libs/gpio/gpio.h ...
```

Esto causa recompilación automática cuando cambian las cabeceras.

## Variables Make

### Variables Definidas

```makefile
CC = g++
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g
LIBRARIES = -pthread -lqrencode -lbcm2835
SRC_DIR = src
LIB_DIR = libs
OBJ_DIR = obj
BIN_DIR = bin
N_APP = epaper_app
```

### Variables Automáticas

| Variable | Significado | Ejemplo |
|----------|-------------|---------|
| `$@` | Target | `bin/epaper_app` |
| `$<` | Primer prerequisito | `src/main.cpp` |
| `$^` | Todos los prerequisitos | `obj/main.o obj/epaper.o ...` |
| `$*` | Stem del target | `main` en `obj/main.o` |
| `$?` | Prerequisitos más nuevos que target | Lista de `.o` modificados |

## Flujo de Compilación

```
make
  |
  |-- Directorios (bin/, obj/)
  |     +-- mkdir -p bin/ obj/
  |
  |-- Compilación
  |     +-- g++ -std=c++20 -c src/main.cpp -o obj/main.o
  |     +-- g++ -std=c++20 -c libs/tyme/tyme.cpp -o obj/tyme.o
  |     +-- g++ -std=c++20 -c libs/epaper/epaper.cpp -o obj/epaper.o
  |     +-- ... (todos los .cpp)
  |
  |-- Linking
  |     +-- g++ -std=c++20 -o bin/epaper_app obj/*.o -pthread -lqrencode -lbcm2835
  |
  +-- "Binary created: bin/epaper_app"
```

## Personalización

### Cambiar Compilador

```makefile
CC = clang++    # o arm-none-eabi-g++
```

### Agregar Flags

Editar `CXXFLAGS`:

```makefile
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g -O2
```

**Opciones comunes**:
- `-O0`: Sin optimización (debug)
- `-O2`: Optimización estándar
- `-O3`: Optimización agresiva
- `-fsanitize=address`: AddressSanitizer para detectar memory leaks

### Agregar Includes

```makefile
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Ithird_party/include -Wall -g
```

### Agregar Librerías

```makefile
LIBRARIES = -pthread -lqrencode -lbcm2835 -lrt -lm
```

### Cambiar Directorios

```makefile
SRC_DIR = src
LIB_DIR = libs
OBJ_DIR = build/obj
BIN_DIR = build/bin
```

## Depuración con Build System

### Habilitar Modo Debug

```makefile
# En lugar de -g usar:
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g3 -O0
```

`-g3` incluye macros de preprocesador.

### Compilar con Tracing

```bash
g++ -std=c++20 -DDBG_EPAPER -DDBG_SPI -DDBG_GPIO ...
```

O modificar Makefile:

```makefile
CXXFLAGS = -std=c++20 -Ilibs -Isrc -Wall -pedantic -g -DDBG_EPAPER -DDBG_SPI
```

## Notas del Diseño

1. **No hay regla `install`**: El proyecto no define instalación al sistema
2. **Targets phony**: `all`, `clean`, `run` están declarados como `.PHONY`
3. **Ejecución forzada con sudo**: La regla `run` siempre requiere sudo
4. **Dependencias débiles**: Los archivos `.d` están incluidos con `-include` (ignora si no existe)
5. **No hay soporte multi-config**: Solo modo Release/Debug mezclado

## Troubleshooting de Compilación

### Error: bcm2835.h not found

```bash
sudo apt-get install libraspberrypi-dev -y
```

### Error: qrencode.h not found

```bash
sudo apt-get install libqrencode-dev -y
```

### Warnings por unused

```bash
g++ -std=c++20 ... -Wno-unused-variable
```

O en código fuente:
```cpp
[[maybe_unused]] const uint8_t register_data_mid[] = { ... };
```

### Error: undefined reference to QRcode_encodeString

Verificar:
```bash
dpkg -L libqrencode-dev | grep .so
```

Y agregar manualmente:
```makefile
LIBRARIES = -pthread -lqrencode-dev -lbcm2835
```
