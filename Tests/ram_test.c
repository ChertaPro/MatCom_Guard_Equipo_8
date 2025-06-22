#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    const long MB = 1024 * 1024;
    long cantidad = 4096;  // 4096 MB = 4 GB

    printf("Asignando %ld MB de RAM...\n", cantidad);

    char *mem = malloc(cantidad * MB);
    if (mem == NULL) {
        perror("malloc");
        return 1;
    }

    for (long i = 0; i < cantidad * MB; i++) {
        mem[i] = i % 256;
    }

    printf("Memoria asignada, presione Ctrl+C para finalizar.\n");

    while (1) {
        sleep(1);
    }

    free(mem);
    return 0;
}
