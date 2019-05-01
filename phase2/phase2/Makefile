SOURCE = *
TARGET = scc

CC = g++
CFLAGS = -g -Wall

all: $(TARGET)

$(TARGET): $(SOURCE).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE).cpp

clean:
	$(RM) -r $(TARGET) $(TARGET).dSYM/ .DS_Store
