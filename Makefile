PREFIX ?= /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
APPLICATIONSDIR = $(DATADIR)/applications
ICONDIR = $(DATADIR)/pixmaps

CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -O2 -Iinclude
LIBS = `pkg-config --libs gtk+-3.0` -lm

# Source files
SRC_DIR = src
SOURCES = $(SRC_DIR)/main_gui.c $(SRC_DIR)/gui.c $(SRC_DIR)/sensors.c $(SRC_DIR)/logger.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = system-analyser

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.o

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -D -m 644 system-analyser.desktop $(DESTDIR)$(APPLICATIONSDIR)/system-analyser.desktop
	install -D -m 644 assets/logo_single_no_bg.png $(DESTDIR)$(ICONDIR)/system-analyser.png

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(APPLICATIONSDIR)/system-analyser.desktop
	rm -f $(DESTDIR)$(ICONDIR)/system-analyser.png
