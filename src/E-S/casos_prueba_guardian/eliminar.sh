#!/bin/bash
if [ ! -f ~/prueba_guardian/nuevo_archivo.txt ]; then
    echo "No existe el archivo a eliminar. Ejecuta crear.sh primero."
    exit 1
fi

rm ~/prueba_guardian/nuevo_archivo.txt
echo "[✓] Archivo eliminado: nuevo_archivo.txt"
