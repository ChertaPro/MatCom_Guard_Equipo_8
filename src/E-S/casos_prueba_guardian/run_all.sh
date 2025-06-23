#!/bin/bash
echo "=============================="
echo "🛡️  Iniciando pruebas del Guardián"
echo "=============================="
sleep 1

echo -e "\n▶️ Caso 1: Crear archivo"
./crear.sh
sleep 2

echo -e "\n▶️ Caso 2: Modificar archivo"
./modificar.sh
sleep 2

echo -e "\n▶️ Caso 3: Eliminar archivo"
./eliminar.sh
sleep 2

echo -e "\n▶️ Caso 4: Crear subcarpeta con archivo"
./crear_subcarpeta.sh
sleep 2

echo -e "\n▶️ Caso 5: Simulación en unidad USB"
./usb_simulado.sh
sleep 2

echo -e "\n✅ Todas las pruebas han sido ejecutadas."
echo -e "📂 Revisa la interfaz gráfica o el archivo ~/.fileguardian.log"
echo -e "🧼 Puedes limpiar con: rm -rf ~/prueba_guardian"

echo "=============================="
