CC = gcc
CFLAGS = -Wall -Wextra -Isrc/utils
LDFLAGS = -lpthread
SRC_DIR = src
UTILS_DIR = $(SRC_DIR)/utils
BUILD_DIR = build

# Alvos principais
all: concorrente

concorrente: $(SRC_DIR)/concorrente.c $(UTILS_DIR)/httpHandler.o $(UTILS_DIR)/serverUtils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regras para objetos
$(UTILS_DIR)/httpHandler.o: $(UTILS_DIR)/httpHandler.c $(UTILS_DIR)/httpHandler.h
	$(CC) $(CFLAGS) -c $< -o $@

$(UTILS_DIR)/serverUtils.o: $(UTILS_DIR)/serverUtils.c $(UTILS_DIR)/serverUtils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f concorrente
	rm -f $(UTILS_DIR)/*.o