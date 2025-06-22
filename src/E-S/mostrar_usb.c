#include <stdio.h>
#include <libudev.h>

int main()
{
    struct udev *udev = udev_new(); // Crea el contexto de udev
    if (!udev)
    {
        printf("No se pudo crear el contexto de udev.\n");
        return;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);                // Crea un enumerate para listar dispositivos
    udev_enumerate_add_match_subsystem(enumerate, "usb");                       // Filtra los dispositivos usb
    udev_enumerate_scan_devices(enumerate);                                     // Escanea los dispositivos
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate); // Obtiene la lista de dispositivos encontrados

    // struct udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(devices, devices) // Recorre la lista de dispositivos
    {
        const char *path = udev_list_entry_get_name(devices); // Obtiene la ruta del dispositivo
        dev = udev_device_new_from_syspath(udev, path);       // Obtiene el dispositivo udev desde esa ruta

        prinf("Dispositivo USB: %s\n", udev_device_get_devnode(dev)); // Mostrar información relevante

        udev_device_unref(dev); // Liberar el dispositivo actual
    }

    // Liberar objetos enumerate y contexto udev
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return;
}