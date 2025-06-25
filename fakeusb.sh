#!/bin/bash

# Archivo de imagen de la memoria simulada
IMG_NAME="fakeusb.img"
MOUNT_POINT="/mnt/fakeusb"

# Tamaño de la memoria en MB
SIZE_MB=500

# Función para crear la imagen y formatearla
crear_imagen() {
    echo "📦 Creando imagen de $SIZE_MB MB..."
    dd if=/dev/zero of="$IMG_NAME" bs=1M count=$SIZE_MB status=none
    mkfs.vfat "$IMG_NAME"
    echo "✅ Imagen $IMG_NAME creada y formateada como vfat."
}

# Función para montar la imagen
montar() {
    if [ ! -f "$IMG_NAME" ]; then
        echo "❌ No se encontró $IMG_NAME. Creando..."
        crear_imagen
    fi

    # Verificar si ya está montada
    if mount | grep -q "$MOUNT_POINT"; then
        echo "⚠️  Ya hay una imagen montada en $MOUNT_POINT."
        mount | grep "$MOUNT_POINT"
        exit 1
    fi

    sudo mkdir -p "$MOUNT_POINT"

    # Asignar loop device libre
    LOOPDEV=$(sudo losetup -f --show "$IMG_NAME")

    sudo mount -t vfat "$LOOPDEV" "$MOUNT_POINT"
    echo "✅ Imagen montada en $MOUNT_POINT con $LOOPDEV"
    mount | grep "$MOUNT_POINT"
}

# Función para desmontar la imagen
desmontar() {
    if mount | grep -q "$MOUNT_POINT"; then
        sudo umount "$MOUNT_POINT"
        echo "✅ Imagen desmontada de $MOUNT_POINT."

        # Liberar loop device asociado
        LOOPDEV=$(sudo losetup -j "$IMG_NAME" | cut -d':' -f1)
        if [ -n "$LOOPDEV" ]; then
            sudo losetup -d "$LOOPDEV"
            echo "✅ Loop device $LOOPDEV liberado."
        fi
    else
        echo "⚠️  No hay nada montado en $MOUNT_POINT."
    fi
}

# Función para eliminar la imagen
eliminar() {
    desmontar
    rm -f "$IMG_NAME"
    echo "🗑️ Imagen eliminada."
}

# Menú de opciones
case "$1" in
    crear)
        crear_imagen
        ;;
    montar)
        montar
        ;;
    desmontar)
        desmontar
        ;;
    eliminar)
        eliminar
        ;;
    *)
        echo "Uso: $0 {crear|montar|desmontar|eliminar}"
        ;;
esac
