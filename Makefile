CC = gcc
CFLAGS = -Wall -g

monitorOpenPorts: monitorOpenPorts.c
	$(CC) $(CFLAGS) -o monitorOpenPorts monitorOpenPorts.c
