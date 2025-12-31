CC = gcc
CFLAGS = -Wall -O2 -std=gnu11
TARGET = wrldtree
SRC = wrldtree.c

# Detect Windows
ifeq ($(OS),Windows_NT)
    TARGET = wrldtree.exe
    RM = del /Q
else
    RM = rm -f
endif

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "✓ Build complete: $(TARGET)"

clean:
	$(RM) $(TARGET) tree_temp.txt 2>/dev/null || true
	@echo "✓ Cleaned"

install: $(TARGET)
ifeq ($(OS),Windows_NT)
	@echo "→ On Windows, add directory to PATH or copy to a directory in PATH"
else
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "✓ Installed to /usr/local/bin/"
endif

uninstall:
ifeq ($(OS),Windows_NT)
	@echo "→ Remove wrldtree.exe from your PATH"
else
	rm -f /usr/local/bin/$(TARGET)
	@echo "✓ Uninstalled"
endif

test: $(TARGET)
	@echo "→ Running tests..."
	@./$(TARGET) --help > /dev/null && echo "✓ Help works"
	@./$(TARGET) --print > /dev/null && echo "✓ Print works"
	@./$(TARGET) --depth 1 --print > /dev/null && echo "✓ Depth limiting works"
	@echo "✓ All tests passed"