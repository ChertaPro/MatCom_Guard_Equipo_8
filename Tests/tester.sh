#!/bin/bash

# Compilar los programas de prueba
echo "📦 Compilando cpu_test_mt.c y ram_test.c..."
gcc cpu_test_mt.c -o cpu_test_mt -lpthread
gcc ram_test.c -o ram_test

# Verificar que se compilaron bien
if [[ ! -f cpu_test_mt || ! -f ram_test ]]; then
    echo "❌ Error al compilar los testers."
    exit 1
fi

# Lanzar ambos en segundo plano
echo "🚀 Ejecutando cpu_test_mt y ram_test..."
./cpu_test_mt &
CPU_PID=$!

./ram_test &
RAM_PID=$!

# Mostrar los PID
echo "✅ cpu_test_mt PID: $CPU_PID"
echo "✅ ram_test     PID: $RAM_PID"

echo ""
echo "✔️  Testers ejecutándose. Puedes monitorear desde MatComGuard."
echo "Para detenerlos manualmente:"
echo "  kill $CPU_PID $RAM_PID"
