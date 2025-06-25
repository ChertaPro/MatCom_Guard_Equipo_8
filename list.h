#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *data;    // Puntero al buffer dinámico
    size_t length; // Longitud actual (sin contar '\0')
} MsgList;

void msglist_init(MsgList *list);
void msglist_add(MsgList *list, const char *txt);
void msglist_clear(MsgList *list);
void msglist_free(MsgList *list);
void msglist_print(const MsgList *list);

#endif