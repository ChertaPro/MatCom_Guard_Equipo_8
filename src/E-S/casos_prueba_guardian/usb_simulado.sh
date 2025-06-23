#!/bin/bash
MOUNT="/media/$(whoami)/usb_prueba"
if [ ! -d "$MOUNT" ]; then
    echo "No se encontró el punto de montaje $MOUNT"
    echo "Conecta una USB y accede a ella para montar automáticamente."
    exit 1
fi

mkdir -p "$MOUNT/test"
touch "$MOUNT/test/usb_archivo.txt"
echo "Contenido desde USB" >> "$MOUNT/test/usb_archivo.txt"
rm "$MOUNT/test/usb_archivo.txt"
echo "[✓] Prueba en USB simulada completada"
