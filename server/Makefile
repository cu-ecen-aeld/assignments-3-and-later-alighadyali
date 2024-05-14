# Makefile

CC=$(CROSS_COMPILE)gcc
# Compiler flags
CFLAGS = -Wall -Wextra -I.

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

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
