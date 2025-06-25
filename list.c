#include "list.h"

/* Inicializa una lista vacía */
void msglist_init(MsgList *list)
{
    list->data = NULL;
    list->length = 0;
}

/* Añade txt al final de la lista */
void msglist_add(MsgList *list, const char *txt)
{
    size_t add_len = strlen(txt);
    size_t new_len = list->length + add_len + 1; // +1 para '\0'
    char *tmp = realloc(list->data, new_len);
    if (!tmp)
    {
        fprintf(stderr, "Error de memoria en msglist_add\n");
        return;
    }
    list->data = tmp;
    memcpy(list->data + list->length, txt, add_len + 1);
    list->length += add_len;
}

/* Limpia el contenido de la lista (la vuelve vacía) */
void msglist_clear(MsgList *list)
{
    free(list->data);
    list->data = NULL;
    list->length = 0;
}

/* Libera la lista completamente */
void msglist_free(MsgList *list)
{
    free(list->data);
    list->data = NULL;
    list->length = 0;
}

/* Imprime el contenido de la lista (útil para debugging) */
void msglist_print(const MsgList *list)
{
    printf("» %s\n", list->data ? list->data : "(vacío)");
}