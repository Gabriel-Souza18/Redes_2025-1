CC = gcc
CFLAGS = -Wall -Wextra -Isrc/utils
LDFLAGS = -lpthread
SRC_DIR = src
UTILS_DIR = $(SRC_DIR)/utils
BUILD_DIR = build

# Alvos principais
all: concorrente iterativo thread thread_e_fila

concorrente: $(SRC_DIR)/concorrente.c $(UTILS_DIR)/httpHandler.o $(UTILS_DIR)/serverUtils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

iterativo: $(SRC_DIR)/iterativo.c $(UTILS_DIR)/httpHandler.o $(UTILS_DIR)/serverUtils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

thread: $(SRC_DIR)/thread.c $(UTILS_DIR)/httpHandler.o $(UTILS_DIR)/serverUtils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

thread_e_fila: $(SRC_DIR)/thread_e_fila.c $(UTILS_DIR)/httpHandler.o $(UTILS_DIR)/serverUtils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regras para objetos
$(UTILS_DIR)/httpHandler.o: $(UTILS_DIR)/httpHandler.c $(UTILS_DIR)/httpHandler.h
	$(CC) $(CFLAGS) -c $< -o $@

$(UTILS_DIR)/serverUtils.o: $(UTILS_DIR)/serverUtils.c $(UTILS_DIR)/serverUtils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f concorrente iterativo thread thread_e_fila
	rm -f $(UTILS_DIR)/*.o
	rm -f log.txt