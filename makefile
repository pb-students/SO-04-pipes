CC = gcc
CFLAGS = -g -Wall

TARGET = pipes

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET) *.o