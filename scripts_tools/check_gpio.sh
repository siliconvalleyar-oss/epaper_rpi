#!/bin/bash
# check_gpio.sh - Verificacion de GPIO/SPI para e-Paper Display

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

show_menu() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  Verificacion E-Paper Display${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo "  1) Verificar pines GPIO"
    echo "  2) Verificar SPI"
    echo "  3) Verificar permisos"
    echo "  4) Verificar todo"
    echo "  5) Info del sistema"
    echo ""
    echo "  0) Salir"
    echo ""
    echo -n "  Opcion: "
}

# --- Pines GPIO ---
check_pins() {
    echo ""
    echo -e "${CYAN}=== PINES E-PAPER (Zero 2W) ===${NC}"
    echo ""

    declare -A NAMES=( [25]="BUSY" [24]="DC" [23]="RESET" [27]="CS" [22]="FLASH_CS" [11]="SCLK" [10]="MOSI" [9]="MISO" )
    declare -A COLORS=( [25]="Rojo" [24]="Naranja" [23]="Amarillo" [27]="Gris" [22]="Violeta" [11]="Marron" [10]="Azul" [9]="Verde" )

    printf "  %-12s %-6s %-10s %-10s %-6s\n" "Pin" "GPIO" "Color" "Estado" "Valor"
    printf "  %-12s %-6s %-10s %-10s %-6s\n" "--------" "----" "-----" "------" "-----"

    for gpio in 25 24 23 27 22 11 10 9; do
        name=${NAMES[$gpio]}
        color=${COLORS[$gpio]}
        gpio_path="/sys/class/gpio/gpio$gpio"

        if [ -d "$gpio_path" ]; then
            direction=$(cat "$gpio_path/direction" 2>/dev/null)
            value=$(cat "$gpio_path/value" 2>/dev/null)
            printf "  %-12s %-6s %-10s ${GREEN}%-10s${NC} %s\n" "$name" "GPIO$gpio" "$color" "EXPORTED" "dir=$direction val=$value"
        else
            printf "  %-12s %-6s %-10s ${YELLOW}%-10s${NC} %s\n" "$name" "GPIO$gpio" "$color" "sin export" "(bcm2835 /dev/mem)"
        fi
    done
}

# --- SPI ---
check_spi() {
    echo ""
    echo -e "${CYAN}=== SPI ===${NC}"
    echo ""

    if [ -e /dev/spidev0.0 ]; then
        echo -e "  spidev0.0: ${GREEN}DISPONIBLE${NC}"
    else
        echo -e "  spidev0.0: ${RED}NO DISPONIBLE${NC}  (sudo raspi-config)"
    fi

    if [ -e /dev/spidev0.1 ]; then
        echo -e "  spidev0.1: ${GREEN}DISPONIBLE${NC}"
    else
        echo -e "  spidev0.1: ${YELLOW}no disponible${NC} (OK si no se usa)"
    fi

    echo ""
    if command -v raspi-gpio &> /dev/null; then
        echo "  Configuracion SPI actual:"
        for pin_info in "10:MOSI" "11:SCLK" "9:MISO"; do
            pin="${pin_info%%:*}"
            label="${pin_info##*:}"
            alt=$(raspi-gpio get $pin 2>/dev/null | grep -o "ALT[0-9]" | head -1)
            if [ -n "$alt" ]; then
                echo -e "    GPIO$pin ($label): ${GREEN}$alt${NC}"
            else
                echo -e "    GPIO$pin ($label): ${YELLOW}GPIO${NC} (no en modo SPI)"
            fi
        done
        echo -e "  ${YELLOW}Nota:${NC} Los pines SPI se configuran al ejecutar la app (bcm2835_spi_begin)"
    else
        echo -e "  ${YELLOW}raspi-gpio no disponible${NC} (sudo apt-get install raspi-gpio)"
    fi
}

# --- Permisos ---
check_permissions() {
    echo ""
    echo -e "${CYAN}=== PERMISOS ===${NC}"
    echo ""

    if [ -e /dev/mem ]; then
        if [ -r /dev/mem ] && [ -w /dev/mem ]; then
            echo -e "  /dev/mem:       ${GREEN}ACCESIBLE${NC}"
        else
            echo -e "  /dev/mem:       ${RED}SIN PERMISOS${NC} (ejecutar con sudo)"
        fi
    fi

    if [ -e /dev/gpiomem ]; then
        if [ -r /dev/gpiomem ] && [ -w /dev/gpiomem ]; then
            echo -e "  /dev/gpiomem:   ${GREEN}ACCESIBLE${NC}"
        else
            echo -e "  /dev/gpiomem:   ${RED}SIN PERMISOS${NC}"
        fi
    fi

    echo ""
    echo "  bcm2835 usa /dev/mem (requiere sudo)"
}

# --- Info sistema ---
check_system() {
    echo ""
    echo -e "${CYAN}=== SISTEMA ===${NC}"
    echo ""

    ARCH=$(uname -m)
    if [[ $ARCH == "armv7l" || $ARCH == "arm" ]]; then
        echo -e "  Arquitectura:   ${GREEN}32-bit${NC} ($ARCH)"
    else
        echo -e "  Arquitectura:   ${GREEN}64-bit${NC} ($ARCH)"
    fi

    echo "  Fecha:          $(date)"
    echo "  Kernel:         $(uname -r)"
    echo "  Hostname:       $(hostname)"

    echo ""
    echo "  Librerias:"
    if ldconfig -p 2>/dev/null | grep -q bcm2835; then
        echo -e "    bcm2835:      ${GREEN}INSTALADA${NC}"
    else
        echo -e "    bcm2835:      ${RED}NO INSTALADA${NC}"
    fi

    if dpkg -l 2>/dev/null | grep -q libqrencode-dev; then
        echo -e "    qrencode:     ${GREEN}INSTALADA${NC}"
    else
        echo -e "    qrencode:     ${YELLOW}no instalada${NC}"
    fi
}

# --- Main ---
while true; do
    show_menu
    read -r option
    case $option in
        1) check_pins ;;
        2) check_spi ;;
        3) check_permissions ;;
        4) check_pins; check_spi; check_permissions ;;
        5) check_system ;;
        0) echo ""; exit 0 ;;
        *) echo -e "  ${RED}Opcion invalida${NC}" ;;
    esac
done
