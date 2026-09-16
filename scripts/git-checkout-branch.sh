#!/usr/bin/env bash
set -euo pipefail

REMOTE="${1:-origin}"
BASE_BRANCH="${2:-main}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "ERROR: No se encontro un repositorio git en el directorio actual." >&2
  exit 1
fi

mapfile -t branches < <(git branch -r | sed "s|^\s*${REMOTE}/||" | sed "/^HEAD/d" | sort)

if [ "${#branches[@]}" -eq 0 ]; then
  echo "No se encontraron ramas remotas en '${REMOTE}'." >&2
  exit 1
fi

echo "Ramas disponibles en '${REMOTE}':"
echo ""
for i in "${!branches[@]}"; do
  printf "  [%d] %s\n" "$((i+1))" "${branches[$i]}"
done
echo ""

branch=""
while [ -z "$branch" ]; do
  read -rp "Elegi el numero de la rama (1-${#branches[@]}): " choice
  if [[ "$choice" =~ ^[0-9]+$ ]] && (( choice >= 1 && choice <= ${#branches[@]} )); then
    branch="${branches[$((choice-1))]}"
  else
    echo "Opcion invalida. Ingresa un numero entre 1 y ${#branches[@]}." >&2
  fi
done

echo ""
echo "Cambiando a rama '${branch}'..."
git checkout -B "$branch" "${REMOTE}/${branch}"
echo "Listo. Ahora trabajas en '${branch}'."
