#ifndef CONFIG_H
#define CONFIG_H

extern double UMBRAL_CPU;
extern double UMBRAL_RAM;
extern int TIEMPO_UMBRAL;
extern long TOTAL_RAM_KB;
extern int MODO_SERVICIO;

void leerConfiguracion();
void leerMemTotal();
void leerWhitelist();
int estaEnWhitelist(const char *nombre);

#endif
