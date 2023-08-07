# Makefile for compiling main.c and function.c
# and generating executable file "test"

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Source files
SRC = main.c function.c

# Object files
OBJ = $(SRC:.c=.o)

# Target
TARGET = test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
