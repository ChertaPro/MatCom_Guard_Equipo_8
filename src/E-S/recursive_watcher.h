#ifndef RECURSIVE_WATCHER_H
#define RECURSIVE_WATCHER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <libnotify/notify.h>

void start_recursive_monitor();

#endif