# linuxburn Makefile

CC = gcc
CFLAGS = -g -W -Wall -Wno-unused `pkg-config --cflags gtk+-2.0`

V = @

all: linuxburn

linuxburn.o: linuxburn.c
	@echo + cc linuxburn.c
	$(V)$(CC) $(CFLAGS) -c linuxburn.c

linuxburn_gui.o: linuxburn_gui.c
	@echo + cc linuxburn_gui.c
	$(V)$(CC) $(CFLAGS) -c linuxburn_gui.c

sts_serial.o: sts_serial.c
	@echo + cc sts_serial.c
	$(V)$(CC) $(CFLAGS) -c sts_serial.c

linuxburn: linuxburn.o sts_serial.o linuxburn_gui.o
	@echo + link linuxburn
	$(V)$(CC) $(CFLAGS) -lpopt `pkg-config --libs gtk+-2.0` -o ../bin/$@ linuxburn.o sts_serial.o linuxburn_gui.o

clean:
	@echo + clean
	$(V)rm -rf *.o ../bin/linuxburn
