SRC := writer.c
TARGET = writer
OBJS := $(SRC:.c=.o)

# prepend cross compiler if environment var is set
CC = $(CROSS_COMPILE)gcc
# $(info $(CC))

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.map