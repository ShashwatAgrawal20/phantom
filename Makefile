CC = clang
CFLAGS = -Wall -Wextra
TARGET = phantom
SOURCE = phantom.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./phantom ./input/README.md titties.html

.PHONY: all clean
clean:
	rm -f $(TARGET) titties.html
