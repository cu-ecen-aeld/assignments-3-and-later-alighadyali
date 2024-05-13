# Makefile

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -I.

# Source files
SRCS := $(wildcard *.c)
# Object files
OBJS := $(SRCS:.c=.o)

# Main target
TARGET = aesdsocket

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phony target to clean up
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
