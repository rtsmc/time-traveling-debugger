# Makefile for Python Time-Traveling Debugger

CC = gcc
CFLAGS = -Wall -O2
LIBS = -lreadline

# Check if readline is available
READLINE_CHECK := $(shell echo '\#include <readline/readline.h>' | $(CC) -E - > /dev/null 2>&1 && echo yes)

ifeq ($(READLINE_CHECK),yes)
    # Readline is available
    TARGET_FLAGS = $(CFLAGS) $(LIBS)
else
    # Readline not available, use basic version
    TARGET_FLAGS = $(CFLAGS) -DNO_READLINE
    $(warning WARNING: readline library not found. Building without CLI enhancements.)
    $(warning Install readline: sudo dnf install readline-devel  OR  sudo apt-get install libreadline-dev)
endif

.PHONY: all clean test help install

all: traceviewer cdebugger

traceviewer: traceviewer.c
	@echo "Building traceviewer..."
	$(CC) -o traceviewer traceviewer.c $(TARGET_FLAGS)
	@chmod +x traceviewer
	@echo "✓ traceviewer built successfully"

cdebugger: debugger.c setup.py
	@echo "Building Python C extension..."
	python3 setup.py build_ext --inplace
	@echo "✓ cdebugger extension built successfully"

# Force build without readline
basic: traceviewer.c
	@echo "Building traceviewer (basic version, no readline)..."
	$(CC) -o traceviewer traceviewer.c $(CFLAGS) -DNO_READLINE
	@chmod +x traceviewer
	@echo "✓ Basic traceviewer built successfully"

# Build everything fresh
rebuild: clean all

clean:
	@echo "Cleaning build files..."
	rm -f traceviewer
	rm -f cdebugger.*.so
	rm -f *.o
	rm -rf build/
	@echo "✓ Clean complete"

test: all
	@echo "Running test..."
	python3 idebug.py test_clean_simple.py < /dev/null || true
	@echo "✓ Test complete (check trace.log)"

install: all
	@echo "Installing to /usr/local/bin..."
	@sudo cp traceviewer /usr/local/bin/
	@sudo chmod +x /usr/local/bin/traceviewer
	@echo "✓ Installed traceviewer to /usr/local/bin/"
	@echo "Note: idebug.py should be run from this directory"

help:
	@echo "Python Time-Traveling Debugger - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build everything (auto-detects readline)"
	@echo "  make traceviewer  - Build only traceviewer"
	@echo "  make cdebugger    - Build only Python C extension"
	@echo "  make basic        - Build without readline (if having issues)"
	@echo "  make rebuild      - Clean and rebuild everything"
	@echo "  make clean        - Remove all build files"
	@echo "  make test         - Build and run quick test"
	@echo "  make install      - Install traceviewer to /usr/local/bin"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - gcc"
	@echo "  - python3"
	@echo "  - python3-dev"
	@echo "  - readline-devel (optional, for CLI enhancements)"
	@echo ""
	@echo "Install readline:"
	@echo "  Fedora/RHEL:  sudo dnf install readline-devel"
	@echo "  Debian/Ubuntu: sudo apt-get install libreadline-dev"
	@echo "  Arch:          sudo pacman -S readline"
