CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lreadline
TARGET = wumbo-shell
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): wumbo-shell.c
	$(CC) $(CFLAGS) -o $(TARGET) wumbo-shell.c $(LDFLAGS)

install: $(TARGET)
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean