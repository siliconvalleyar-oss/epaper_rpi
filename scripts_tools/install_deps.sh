#!/bin/bash
# install_deps.sh - Instalacion de dependencias para e-Paper Display

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

show_menu() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  Instalacion de Dependencias${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo "  1) Verificar dependencias"
    echo "  2) Instalar bcm2835"
    echo "  3) Instalar libqrencode-dev"
    echo "  4) Instalar todo"
    echo ""
    echo "  0) Salir"
    echo ""
    echo -n "  Opcion: "
}

# --- Verificar ---
check_deps() {
    echo ""
    echo -e "${CYAN}=== ESTADO DE DEPENDENCIAS ===${NC}"
    echo ""

    # bcm2835
    if ldconfig -p 2>/dev/null | grep -q bcm2835; then
        echo -e "  bcm2835:        ${GREEN}INSTALADA${NC}"
    else
        echo -e "  bcm2835:        ${RED}FALTA${NC}"
    fi

    # qrencode
    if dpkg -l 2>/dev/null | grep -q libqrencode-dev; then
        echo -e "  libqrencode:    ${GREEN}INSTALADA${NC}"
    else
        echo -e "  libqrencode:    ${RED}FALTA${NC}"
    fi

    # libraspberrypi-dev
    if dpkg -l 2>/dev/null | grep -q libraspberrypi-dev; then
        echo -e "  libraspberrypi: ${GREEN}INSTALADA${NC}"
    else
        echo -e "  libraspberrypi: ${YELLOW}no instalada${NC} (opcional)"
    fi

    # Compilador
    if command -v g++ &> /dev/null; then
        echo -e "  g++:            ${GREEN}$(g++ --version | head -1)${NC}"
    else
        echo -e "  g++:            ${RED}FALTA${NC}"
    fi
}

# --- Instalar bcm2835 ---
install_bcm2835() {
    echo ""
    if ldconfig -p 2>/dev/null | grep -q bcm2835; then
        echo -e "  ${GREEN}bcm2835 ya esta instalada${NC}"
        return
    fi

    echo "  Instalando bcm2835..."
    cd /tmp
    wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz || {
        echo -e "  ${RED}Error al descargar bcm2835${NC}"
        return 1
    }
    tar xzf bcm2835-1.71.tar.gz
    cd bcm2835-1.71
    ./configure --quiet
    make --quiet
    sudo make install
    sudo ldconfig
    cd -
    echo -e "  ${GREEN}bcm2835 instalada correctamente${NC}"
}

# --- Instalar qrencode ---
install_qrencode() {
    echo ""
    if dpkg -l 2>/dev/null | grep -q libqrencode-dev; then
        echo -e "  ${GREEN}libqrencode-dev ya esta instalada${NC}"
        return
    fi

    echo "  Instalando libqrencode-dev..."
    sudo apt-get install -y libqrencode-dev
    echo -e "  ${GREEN}libqrencode-dev instalada correctamente${NC}"
}

# --- Instalar todo ---
install_all() {
    install_bcm2835
    install_qrencode
    echo ""
    echo -e "${CYAN}=== Dependencias instaladas ===${NC}"
}

# --- Main ---
while true; do
    show_menu
    read -r option
    case $option in
        1) check_deps ;;
        2) install_bcm2835 ;;
        3) install_qrencode ;;
        4) install_all ;;
        0) echo ""; exit 0 ;;
        *) echo -e "  ${RED}Opcion invalida${NC}" ;;
    esac
done
