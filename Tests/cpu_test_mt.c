#include <stdio.h>
#include <pthread.h>

void* consumir_cpu(void* arg) {
    double x = 0;
    while (1) {
        x += 1.23456789;
    }
    return NULL;
}

int main() {
    int num_hilos = 50;  // ajusta según cuántos núcleos quieras ocupar
    pthread_t hilos[num_hilos];

    for (int i = 0; i < num_hilos; i++) {
        pthread_create(&hilos[i], NULL, consumir_cpu, NULL);
    }

    for (int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    return 0;
}
