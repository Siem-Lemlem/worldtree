CC = gcc
CFLAGS = -Wall -O2 -std=gnu11
TARGET = wrldtree
SRC = wrldtree.c

ifeq ($(OS),Windows_NT)
    TARGET = wrldtree.exe
    RM = del /Q
else
    RM = rm -f
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "✓ Build complete: $(TARGET)"

install: $(TARGET)
ifeq ($(OS),Windows_NT)
	@echo "→ Manual Step: Copy $(TARGET) to a folder in your PATH (e.g., C:\bin)"
	@echo "→ AVOID using 'setx' to add paths!"
else
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "✓ Installed to /usr/local/bin/"
endif