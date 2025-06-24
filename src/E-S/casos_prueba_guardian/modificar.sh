#!/bin/bash
if [ ! -f ~/prueba_guardian/nuevo_archivo.txt ]; then
    echo "No existe el archivo a modificar. Ejecuta crear.sh primero."
    exit 1
fi

echo "Texto de modificación" >> ~/prueba_guardian/nuevo_archivo.txt
echo "[✓] Archivo modificado: nuevo_archivo.txt"
