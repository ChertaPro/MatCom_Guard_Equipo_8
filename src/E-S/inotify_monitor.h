#ifndef INOTIFY_MONITOR_H
#define INOTIFY_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <libnotify/notify.h>

#define MONITOR_DIR "/media"
#define MAX_DEVICES 64
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

extern char mounted_devices[MAX_DEVICES][256];
extern int device_count;
extern pthread_mutex_t device_mutex;

void start_inotify_monitor();

#endif