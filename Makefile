CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./src -D_GNU_SOURCE
LDFLAGS = -lncurses -lm
DEBUG_FLAGS = -g -DDEBUG

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
THEME_DIR = themes
CONFIG_DIR = config

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET = $(BIN_DIR)/pappier

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

install: $(TARGET)
	mkdir -p /usr/local/bin
	cp $(TARGET) /usr/local/bin/pappier
	mkdir -p /etc/pappier
	cp $(CONFIG_DIR)/pappier.conf /etc/pappier/
	mkdir -p /usr/share/pappier/themes
	cp $(THEME_DIR)/*.theme /usr/share/pappier/themes/

uninstall:
	rm -f /usr/local/bin/pappier
	rm -rf /etc/pappier
	rm -rf /usr/share/pappier

dist: clean
	mkdir -p dist
	tar -czf dist/pappier-$(shell date +%Y%m%d).tar.gz \
		--exclude=dist \
		--exclude=obj \
		--exclude=bin \
		.

.PHONY: all clean install uninstall dist debug