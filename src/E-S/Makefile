CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0 --static` -ludev -lnotify -static -lpthread

# Archivos fuente y objeto
OBJS = main_Lumi.o device_viewer.o inotify_monitor.o recursive_watcher.o log_viewer.o
EXEC = MatComGuard

# Headers
HEADERS = device_viewer.h inotify_monitor.h recursive_watcher.h log_viewer.h

# Regla principal
all: $(EXEC)

# Enlazado final
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) $(LIBS)

# Reglas de compilación para cada objeto
main_Lumi.o: main_Lumi.c $(HEADERS)
	$(CC) $(CFLAGS) -c main_Lumi.c

device_viewer.o: device_viewer.c device_viewer.h
	$(CC) $(CFLAGS) -c device_viewer.c

inotify_monitor.o: inotify_monitor.c inotify_monitor.h
	$(CC) $(CFLAGS) -c inotify_monitor.c

recursive_watcher.o: recursive_watcher.c recursive_watcher.h
	$(CC) $(CFLAGS) -c recursive_watcher.c

log_viewer.o: log_viewer.c log_viewer.h
	$(CC) $(CFLAGS) -c log_viewer.c

# Limpieza
clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean