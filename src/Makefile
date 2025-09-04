CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -pthread -Iinclude

BIN_DIR := ../bin
OBJ_DIR := .objs

COMMON_OBJS := $(OBJ_DIR)/workload.o $(OBJ_DIR)/timing.o $(OBJ_DIR)/utils.o

APPS := $(BIN_DIR)/linkedlist_serial \
        $(BIN_DIR)/linkedlist_mutex \
        $(BIN_DIR)/linkedlist_rwlock

all: $(BIN_DIR) $(OBJ_DIR) $(APPS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/workload.o: workload/workload.c include/workload.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/timing.o: timing/timing.c include/timing.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/utils.o: utils.c include/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/serial_linked_list.o: serial_linked_list.c include/timing.h include/utils.h include/workload.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/mutex_linked_list.o: mutex_linked_list.c include/timing.h include/utils.h include/workload.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/rwlock_linked_list.o: rwlock_linked_list.c include/timing.h include/utils.h include/workload.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/linkedlist_serial: $(OBJ_DIR)/serial_linked_list.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/linkedlist_mutex: $(OBJ_DIR)/mutex_linked_list.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/linkedlist_rwlock: $(OBJ_DIR)/rwlock_linked_list.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
