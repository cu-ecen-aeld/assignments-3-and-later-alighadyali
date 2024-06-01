# Makefile

CC := $(CROSS_COMPILE)gcc
# Compiler flags
CFLAGS = -Wall -Wextra -I.

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
LFLAGS := -lpthread

TARGET = aesdsocket

all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Rule to compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phony target to clean up
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
