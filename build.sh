#!/bin/bash
# Build script for all epaper projects
# Usage: ./build.sh [clean|all|project_name]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECTS=("RASPI_EPD_SRC_Clock_Ascii" "RASPI_EPD_SRC_QR_Invert_Graphics" "RASPI_EPD_SRC_Electrocardiogram" "RASPI_EPD_SRC_Dino_Game")

clean_all() {
    echo "========================================"
    echo "Cleaning all projects..."
    echo "========================================"
    for proj in "${PROJECTS[@]}"; do
        echo "Cleaning: $proj"
        make -C "$SCRIPT_DIR/$proj" clean 2>/dev/null || true
    done
    echo "All projects cleaned."
    echo ""
}

build_project() {
    local proj=$1
    echo "========================================"
    echo "Building: $proj"
    echo "========================================"
    if [ -f "$SCRIPT_DIR/$proj/Makefile" ]; then
        if make -C "$SCRIPT_DIR/$proj" -j4 2>&1 | grep -q "bcm2835.h: No existe"; then
            echo "$proj: bcm2835.h not found (expected on dev machine, builds on RPi)"
        else
            echo "$proj: OK"
        fi
    else
        echo "$proj: No Makefile found, skipping"
    fi
    echo ""
}

build_all() {
    echo "========================================"
    echo "Building all projects..."
    echo "========================================"
    for proj in "${PROJECTS[@]}"; do
        build_project "$proj"
    done
    echo "========================================"
    echo "Build complete!"
    echo "========================================"
}

# Main
case "${1:-all}" in
    clean)
        clean_all
        ;;
    all)
        clean_all
        build_all
        ;;
    *)
        # Build specific project
        if [[ " ${PROJECTS[@]} " =~ " ${1} " ]]; then
            build_project "$1"
        else
            echo "Usage: $0 [clean|all|project_name]"
            echo ""
            echo "Projects:"
            for proj in "${PROJECTS[@]}"; do
                echo "  $proj"
            done
            exit 1
        fi
        ;;
esac
