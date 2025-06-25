#include "gui.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "list.h"

MsgList Alert_Devices, Alert_Process, Alert_Ports;

void do_append_info(char *message)
{
    append_info(message);
    while (gtk_events_pending())
        gtk_main_iteration();
}

void do_append_alert(char *message, int id) // id 0:devices 1:process 2:ports
{
    if (id == 0)
        msglist_add(&Alert_Devices, message);
    else if (id == 1)
        msglist_add(&Alert_Process, message);
    else
        msglist_add(&Alert_Ports, message);
    append_alert(message);
    while (gtk_events_pending())
        gtk_main_iteration();
}

void general_append_info(char *dev, char *proc, char *port)
{
    char *buffer = malloc(strlen(dev) + strlen(proc) + strlen(port));
    strcpy(buffer, dev);
    strcat(buffer, proc);
    strcat(buffer, port);
    clear_info();
    do_append_info(buffer);
}

void init_list()
{
    msglist_init(&Alert_Devices);
    msglist_init(&Alert_Process);
    msglist_init(&Alert_Ports);
}

void free_list()
{
    msglist_free(&Alert_Devices);
    msglist_free(&Alert_Process);
    msglist_free(&Alert_Ports);
}

int main(int argc, char *argv[])
{
    init_list();
    gui_init(&argc, &argv);

    // code

    gui_run();
    free_list();

    return 0;
}