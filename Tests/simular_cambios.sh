#!/bin/bash

MOUNT_POINT="/mnt/fakeusb"

echo "🔧 Simulando cambios maliciosos en $MOUNT_POINT..."

# 1️⃣ Crecimiento inusual de tamaño
echo "Creando archivo grande..."
sudo dd if=/dev/zero of="$MOUNT_POINT/inusual_grande.doc" bs=1M count=500 status=none
sudo ls -lh "$MOUNT_POINT/inusual_grande.doc"

# 2️⃣ Replicación de archivos (copias aleatorias)
echo "Creando copias con nombres aleatorios..."
sudo cp "$MOUNT_POINT/inusual_grande.doc" "$MOUNT_POINT/copia_a.doc"
sudo cp "$MOUNT_POINT/inusual_grande.doc" "$MOUNT_POINT/copia_b.doc"

# 3️⃣ Crear archivo txt, cambiar a exe y permisos a 777
echo "Creando archivo normal y cambiando extensión y permisos..."
sudo bash -c 'echo "Contenido de prueba normal" > /mnt/fakeusb/archivo_normal.txt'
sudo mv "$MOUNT_POINT/archivo_normal.txt" "$MOUNT_POINT/archivo_normal.exe"
sudo chmod 777 "$MOUNT_POINT/archivo_normal.exe"

# 4️⃣ Modificar timestamps y ownership
echo "Modificando timestamps y ownership..."
sudo touch -m -t 202001010101.01 "$MOUNT_POINT/archivo_normal.exe"
sudo chown nobody:nogroup "$MOUNT_POINT/archivo_normal.exe"

# 5️⃣ Crear otros archivos nuevos como simulación
echo "Creando archivos nuevos adicionales..."
sudo bash -c 'echo "Otro contenido" > /mnt/fakeusb/nuevo_archivo1.txt'
sudo bash -c 'echo "Archivo sospechoso" > /mnt/fakeusb/informe_007.txt'

echo "✅ Simulación de cambios completada."
