CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -pthread -Iinclude

BIN_DIR := ../bin
SRC_DIR := .
APP_DIR := apps
OBJ_DIR := .objs

LIB_OBJS := $(OBJ_DIR)/linkedlist.o $(OBJ_DIR)/workload.o $(OBJ_DIR)/timing.o

APPS := $(BIN_DIR)/linkedlist_serial \
        $(BIN_DIR)/linkedlist_mutex \
        $(BIN_DIR)/linkedlist_rwlock

all: $(BIN_DIR) $(OBJ_DIR) $(APPS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/linkedlist.o: linkedlist/linkedlist.c include/linkedlist.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/workload.o: workload/workload.c include/workload.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/timing.o: timing/timing.c include/timing.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/linkedlist_serial: $(LIB_OBJS) $(APP_DIR)/linkedlist_serial.c
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/linkedlist_mutex: $(LIB_OBJS) $(APP_DIR)/linkedlist_mutex.c
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/linkedlist_rwlock: $(LIB_OBJS) $(APP_DIR)/linkedlist_rwlock.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
