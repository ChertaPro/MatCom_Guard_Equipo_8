#ifndef MESSAGE_H
#define MESSAGE_H

char** mostrar_alertas_message();
char** mostrar_puertos_message();
void add_message(const char* msg, int is_alert);
char** get_messages(int* count, int get_alerts);

#endif