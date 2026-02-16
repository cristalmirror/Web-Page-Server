# Variables para facilitar cambios futuros
CC := gcc
LDLIBS := -lssl -lcrypto -lpthread
TARGET := servidor_ssl
SRCS := main.c download_operations.c

# El primer objetivo es el que se ejecuta por defecto [1, 2]
all: $(TARGET)

# Regla para construir el ejecutable
# $@ se refiere al nombre del objetivo (servidor_ssl) [2, 3]
# $^ se refiere a todas las dependencias (main.c download_operations.c) [3]
$(TARGET): $(SRCS)
	$(CC) $^ $(LDLIBS) -o $@

# Objetivo para limpiar los archivos generados [4, 5]
.PHONY: all clean
clean:
	rm -f $(TARGET)