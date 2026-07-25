#!/bin/bash
# git_menu.sh - Menu organizado para gestion del proyecto e-Paper RPi
#
# CONFIGURACION DE USUARIO  (ajusta segun tu Raspberry Pi)
# ─────────────────────────────────────────────────────────────
RPI_USER="pi"                  # usuario SSH en la Raspberry
RPI_HOST="raspi.local"         # hostname/IP de la Raspberry
RPI_PROJECT="src/epaper_rpi"   # ruta del proyecto en la Raspberry
LOCAL_REPO="."                 # carpeta local del repositorio
REMOTE_BRANCH="origin/main"    # rama remota para sync
# ─────────────────────────────────────────────────────────────
# (no pongas contrasenas aca; ssh-keygen + ssh-copy-id es mas seguro)

# ============================================================
# COLORES
# ============================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# ============================================================
# FUNCIONES DEL REPOSITORIO
# ============================================================

# --- Tags (funcionalidad original) ---
tag_last() {
    echo ""
    echo -e "${CYAN}=== ULTIMO TAG ===${NC}"
    git for-each-ref --sort=-creatordate \
        --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short)' \
        refs/tags | head -1
}

tag_list() {
    echo ""
    echo -e "${CYAN}=== TAGS (orden inverso) ===${NC}"
    git for-each-ref --sort=-creatordate \
        --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short)' \
        refs/tags
}

tag_author() {
    echo ""
    echo -e "${CYAN}=== TAGS POR AUTOR ===${NC}"
    git for-each-ref --sort=creatordate \
        --format '%(creatordate:format:%Y-%m-%d %H:%M) %(refname:short) - %(taggername)' \
        refs/tags
}

# --- Estado local ---
repo_status() {
    echo ""
    echo -e "${CYAN}=== ESTADO ===${NC}"
    git status
    echo ""
    echo -e "${CYAN}=== ULTIMOS COMMITS ===${NC}"
    git log --oneline -10
}

# --- Push al remoto ---
repo_push() {
    echo ""
    local current_branch
    current_branch=$(git rev-parse --abbrev-ref HEAD)
    echo -e "${YELLOW}Branch actual:${NC} $current_branch"
    echo -e "${YELLOW}Push a:${NC} origin/$current_branch"
    echo -n "  Confirmar push? (s/n): "
    read -r confirm
    if [ "$confirm" = "s" ] || [ "$confirm" = "S" ]; then
        git push origin "$current_branch"
        echo -e "${GREEN}Push completado${NC}"
    else
        echo "  Cancelado"
    fi
}

# --- Pull desde remoto ---
repo_pull() {
    echo ""
    local current_branch
    current_branch=$(git rev-parse --abbrev-ref HEAD)
    echo -e "${YELLOW}Haciendo pull de:${NC} origin/$current_branch"
    git pull origin "$current_branch"
    echo -e "${GREEN}Pull completado${NC}"
}

# ============================================================
# FUNCIONES REMOTAS (Raspberry Pi)
# ============================================================

# --- Sincronizar archivos a la RPi (rsync) ---
rpi_sync() {
    local dest="$RPI_USER@$RPI_HOST:$RPI_PROJECT/"
    echo ""
    echo -e "${YELLOW}Origen:${NC}  $LOCAL_REPO"
    echo -e "${YELLOW}Destino:${NC} $dest"
    echo -n "  Confirmar rsync? (s/n): "
    read -r confirm
    if [ "$confirm" != "s" ] && [ "$confirm" != "S" ]; then
        echo "  Cancelado"
        return
    fi

    rsync -avz --delete \
        --exclude='.git/' \
        --exclude='obj/' \
        --exclude='bin/' \
        --exclude='*.o' \
        --exclude='*.d' \
        "$LOCAL_REPO"/ "$dest"
    echo -e "${GREEN}Sincronizacion completada${NC}"
}

# --- Compilar en la RPi ---
rpi_build() {
    local project="$RPI_PROJECT"
    local folder_var="${1:-}"  # nombre de la subcarpeta (ej: RASPI_EPD_SRC_Dino_Game)

    if [ -n "$folder_var" ]; then
        project="$project/$folder_var"
    fi

    echo ""
    echo -e "${YELLOW}Compilando en${NC} $RPI_USER@$RPI_HOST:$project"
    ssh -o StrictHostKeyChecking=no "$RPI_USER@$RPI_HOST" \
        "cd $project && make clean && make"

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Compilacion exitosa${NC}"
    else
        echo -e "${RED}Error de compilacion${NC}"
    fi
}

# --- Ejecutar en la RPi (requiere sudo) ---
rpi_run() {
    local project="$RPI_PROJECT"
    local folder_var="${1:-}"
    local binary="${2:-bin/main}"

    if [ -n "$folder_var" ]; then
        project="$project/$folder_var"
    fi

    echo ""
    echo -e "${YELLOW}Ejecutando${NC} $RPI_USER@$RPI_HOST:$project/$binary"
    ssh -o StrictHostKeyChecking=no "$RPI_USER@$RPI_HOST" \
        "cd $project && sudo ./$binary"
}

# --- Detener proceso en la RPi ---
rpi_stop() {
    local process="${1:-ecg_demo}"

    echo ""
    echo -e "${YELLOW}Deteniendo proceso '$process' en${NC} $RPI_HOST..."
    ssh -o StrictHostKeyChecking=no "$RPI_USER@$RPI_HOST" \
        "pkill -f '$process' 2>/dev/null; echo '  Proceso terminado'"
}

# --- Actualizar, compilar y ejecutar en un solo paso ---
rpi_deploy() {
    local folder_var="$1"
    local binary="$2"
    rpi_sync
    rpi_build "$folder_var"
    echo ""
    echo -e "${CYAN}¿Ejecutar ahora?${NC}"
    echo -n "  (s/n): "
    read -r confirm
    if [ "$confirm" = "s" ] || [ "$confirm" = "S" ]; then
        rpi_run "$folder_var" "$binary"
    fi
}

# ============================================================
# MENU PRINCIPAL
# ============================================================
show_menu() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  GIT MENU - E-Paper RPi${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
    echo "  Config: $RPI_USER@$RPI_HOST:$RPI_PROJECT"
    echo ""
    echo "--- REPOSITORIO LOCAL ---"
    echo "  1)  Ultimo tag"
    echo "  2)  Lista de tags"
    echo "  3)  Tags por autor"
    echo "  4)  Status + ultimos commits"
    echo "  5)  Push a origin"
    echo "  6)  Pull desde origin"
    echo ""
    echo "--- RPi REMOTO ---"
    echo "  7)  Sincronizar archivos (rsync)"
    echo "  8)  Compilar proyecto en RPi"
    echo "  9)  Ejecutar binario en RPi"
    echo "  10) Detener proceso en RPi"
    echo "  11) Deploy completo (sync + build + run)"
    echo ""
    echo "--- UTILIDADES ---"
    echo "  c)  Cambiar configuracion RPi"
    echo "  i)  Informacion del script"
    echo ""
    echo "  0)  Salir"
    echo ""
    echo -n "  Opcion: "
}

# ============================================================
# CONFIGURACION
# ============================================================
edit_config() {
    echo ""
    echo -e "${CYAN}=== CONFIGURACION RPi ===${NC}"
    echo ""
    echo "  Deja en blanco para mantener el valor actual"
    echo ""
    echo -n "  Usuario SSH    [$RPI_USER]: "
    read -r input
    [ -n "$input" ] && RPI_USER="$input"

    echo -n "  Hostname/IP    [$RPI_HOST]: "
    read -r input
    [ -n "$input" ] && RPI_HOST="$input"

    echo -n  "Ruta proyecto   [$RPI_PROJECT]: "
    read -r input
    [ -n "$input" ] && RPI_PROJECT="$input"

    echo ""
    echo -e "${GREEN}Configuracion actualizada${NC}"
    echo "  $RPI_USER@$RPI_HOST:~/$RPI_PROJECT"
}

# ============================================================
# INFO
# ============================================================
show_info() {
    echo ""
    echo -e "${CYAN}=== INFORMACION ===${NC}"
    echo ""
    echo "  git origin:"
    git remote -v 2>/dev/null | head -2
    echo ""
    echo "  Folder actual: $(pwd)"
    echo "  Proyectos disponibles:"
    ls -d RASPI_EPD_SRC_* 2>/dev/null || echo "    (ninguno)"
    ls -d epaper_success_v1.0.* 2>/dev/null || echo "    (solo RASPI_EPD_SRC_*)"
    echo ""
    echo -e "${YELLOW}Recomendacion:${NC}"
    echo "  Usa ssh-keygen + ssh-copy-id para acceso sin contrasena:"
    echo "    ssh-keygen -t ed25519"
    echo "    ssh-copy-id $RPI_USER@$RPI_HOST"
}

# ============================================================
# PROMPT PARA COMANDO RAPIDO
# ============================================================
ask_folder() {
    echo ""
    echo "  Proyectos disponibles:"
    local i=1
    for d in RASPI_EPD_SRC_*; do
        if [ -d "$d" ]; then
            echo "    $i) $d"
            eval "folder_$i='$d'"
            i=$((i+1))
        fi
    done
    echo -n "  Selecciona numero (o Enter para raiz): "
    read -r sel
    if [ -n "$sel" ] && [ "$sel" -gt 0 ] 2>/dev/null; then
        eval "echo \${folder_$sel}"
    fi
}

# ============================================================
# MAIN LOOP
# ============================================================
while true; do
    show_menu
    read -r option
    case $option in
        1) tag_last ;;
        2) tag_list ;;
        3) tag_author ;;
        4) repo_status ;;
        5) repo_push ;;
        6) repo_pull ;;
        7) rpi_sync ;;
        8) rpi_build "$(ask_folder)" ;;
        9) rpi_run "$(ask_folder)" "$(echo -n '  Binary [bin/main]: '; read b; echo ${b:-bin/main})" ;;
        10) rpi_stop "$(echo -n '  Process name [ecg_demo]: '; read p; echo ${p:-ecg_demo})" ;;
        11) echo -n "  Folder (Enter para raiz): "; read f; echo -n "  Binary [bin/main]: "; read b; rpi_deploy "$f" "${b:-bin/main}" ;;
        c|C) edit_config ;;
        i|I) show_info ;;
        0) echo ""; exit 0 ;;
        *) echo -e "  ${RED}Opcion invalida${NC}" ;;
    esac
done
