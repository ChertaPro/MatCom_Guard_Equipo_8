// gui.h
#ifndef MAIN_H
#define MAIN_H

#include "list.h"

extern MsgList Alert_Devices, Alert_Process, Alert_Ports, Info_Devices, Info_Process, Info_Ports;

void do_append_alert(char *message, int id);
void do_append_info(char *message);

#endif