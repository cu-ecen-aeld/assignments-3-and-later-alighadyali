# Makefile

CC ?= $(CROSS_COMPILE)gcc
# Compiler flags
CFLAGS ?= -Wall -Wextra -I.

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
LDFLAGS ?= -lpthread

TARGET ?= aesdsocket

all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phony target to clean up
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
