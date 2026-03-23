CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -I.

SRCS = main.c alarm_sigaction.c stateMachine.c tx_datalink.c
OBJS = $(SRCS:.c=.o)
TARGET = ex

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	sudo ./$(TARGET) /dev/ttyUSB0

.PHONY: all clean run
