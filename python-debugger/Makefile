# Makefile for Python Time-Traveling Debugger
# Handles automatic detection and building of all components

CC = gcc
CFLAGS = -Wall -O2
PYTHON ?= python

# Colors for output
RED = \033[1;31m
GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
RESET = \033[0m

# Check if readline is available by checking for the header and library
READLINE_CHECK := $(shell pkg-config --exists readline 2>/dev/null && echo yes || (echo '\#include <readline/readline.h>\nint main(){return 0;}' | $(CC) -x c - -lreadline 2>/dev/null && echo yes))

# Check if Python development headers are available
PYTHON_CHECK := $(shell $(PYTHON) -c "import sysconfig" 2>/dev/null && echo yes)

# Readline is mandatory
READLINE_FLAGS = -lreadline

.PHONY: all clean test help install check-deps verify rebuild

# Default target - build everything
all: check-deps
	@echo "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(CYAN)║         Building Time-Traveling Debugger              ║$(RESET)"
	@echo "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo ""
	@$(MAKE) -s traceviewer
	@echo ""
	@$(MAKE) -s verify
	@echo ""
	@echo "$(GREEN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(GREEN)║              Build Complete! 🎉                       ║$(RESET)"
	@echo "$(GREEN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo ""
	@echo "$(CYAN)To run the debugger:$(RESET)"
	@echo "  $(PYTHON) idebug.py test_import_main.py"
	@echo ""

# Check dependencies before building
check-deps:
	@echo "$(CYAN)Checking dependencies...$(RESET)"
	@command -v $(CC) >/dev/null 2>&1 || { echo "$(RED)✗ gcc not found. Please install gcc.$(RESET)"; exit 1; }
	@echo "$(GREEN)✓$(RESET) gcc found"
	@command -v $(PYTHON) >/dev/null 2>&1 || { echo "$(RED)✗ python3 not found. Please install python3.$(RESET)"; exit 1; }
	@echo "$(GREEN)✓$(RESET) python3 found"
ifneq ($(PYTHON_CHECK),yes)
	@echo "$(RED)✗ Python development headers not found$(RESET)"
	@echo "$(YELLOW)Install python3-dev:$(RESET)"
	@echo "  Fedora/RHEL:  sudo dnf install python3-devel"
	@echo "  Debian/Ubuntu: sudo apt-get install python3-dev"
	@echo "  Arch:          sudo pacman -S python"
	@exit 1
else
	@echo "$(GREEN)✓$(RESET) python3-dev found"
endif
ifneq ($(READLINE_CHECK),yes)
	@echo "$(RED)✗ readline not found (required)$(RESET)"
	@echo "$(YELLOW)Install readline development library:$(RESET)"
	@echo "  Fedora/RHEL:  sudo dnf install readline-devel"
	@echo "  Debian/Ubuntu: sudo apt-get install libreadline-dev"
	@echo "  Arch:          sudo pacman -S readline"
	@echo "  macOS:         brew install readline"
	@exit 1
else
	@echo "$(GREEN)✓$(RESET) readline found"
endif
	@echo ""

# Build traceviewer
traceviewer: traceviewer.c
	@echo "$(CYAN)Building traceviewer (with readline support)...$(RESET)"
	@$(CC) -o traceviewer traceviewer.c $(CFLAGS) $(READLINE_FLAGS) 2>&1 | \
		grep -v "warning:" | grep -v "note:" || true
	@chmod +x traceviewer 2>/dev/null || true
	@if [ -x traceviewer ]; then \
		echo "$(GREEN)✓ traceviewer built successfully$(RESET)"; \
	else \
		echo "$(RED)✗ Failed to build traceviewer$(RESET)"; \
		exit 1; \
	fi

# Verify build
verify:
	@echo "$(CYAN)Verifying build...$(RESET)"
	@ERRORS=0; \
	if [ ! -x traceviewer ]; then \
		echo "$(RED)✗ traceviewer not found or not executable$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	else \
		echo "$(GREEN)✓$(RESET) traceviewer is ready"; \
	fi; \
	if ! ls cdebugger*.so >/dev/null 2>&1; then \
		echo "$(RED)✗ cdebugger extension not found$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	else \
		echo "$(GREEN)✓$(RESET) cdebugger extension is ready"; \
	fi; \
	if $(PYTHON) -c "import cdebugger" 2>/dev/null; then \
		echo "$(GREEN)✓$(RESET) cdebugger module imports correctly"; \
	else \
		echo "$(RED)✗ cdebugger module cannot be imported$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	fi; \
	if [ $$ERRORS -gt 0 ]; then \
		echo ""; \
		echo "$(RED)Build verification failed with $$ERRORS error(s)$(RESET)"; \
		exit 1; \
	fi

# Clean build artifacts
clean:
	@echo "$(CYAN)Cleaning build files...$(RESET)"
	@rm -f traceviewer
	@rm -f cdebugger*.so
	@rm -f *.o
	@rm -rf build/
	@rm -rf __pycache__/
	@echo "$(GREEN)✓ Clean complete$(RESET)"

# Build everything from scratch
rebuild: clean all

# Run tests
test: all
	@echo ""
	@echo "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(CYAN)║                  Running Tests                         ║$(RESET)"
	@echo "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo ""
	@echo "Test 1: Module import test..."
	@if $(PYTHON) -c "import cdebugger; print('  $(GREEN)✓$(RESET) cdebugger imports correctly')" 2>/dev/null; then :; else \
		echo "  $(RED)✗$(RESET) cdebugger import failed"; \
	fi
	@echo ""
	@echo "Test 2: Running debugger..."
	@if echo "r" | $(PYTHON) idebug.py test_clean_simple.py >/dev/null 2>&1; then \
		echo "  $(GREEN)✓$(RESET) Debugger runs successfully"; \
	else \
		echo "  $(RED)✗$(RESET) Debugger failed"; \
	fi
	@echo ""
	@if [ -f trace.log ]; then \
		echo "Test 3: Testing show command..."; \
		if echo -e "show\nq" | ./traceviewer trace.log 2>&1 | grep -q "File:"; then \
			echo "  $(GREEN)✓$(RESET) Show command works"; \
		else \
			echo "  $(RED)✗$(RESET) Show command failed"; \
		fi; \
		echo ""; \
		echo "$(GREEN)All tests completed!$(RESET)"; \
	else \
		echo "$(YELLOW)⚠ trace.log not found, skipping trace viewer test$(RESET)"; \
	fi

# Install to system
install: all
	@echo "$(CYAN)Installing to /usr/local/bin...$(RESET)"
	@sudo cp traceviewer /usr/local/bin/ || { echo "$(RED)✗ Installation failed$(RESET)"; exit 1; }
	@sudo chmod +x /usr/local/bin/traceviewer
	@echo "$(GREEN)✓ Installed traceviewer to /usr/local/bin/$(RESET)"
	@echo ""
	@echo "$(YELLOW)Note:$(RESET) idebug.py must be run from the project directory"

# Show help
help:
	@echo "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo "$(CYAN)║   Python Time-Traveling Debugger - Build System       ║$(RESET)"
	@echo "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo ""
	@echo "$(GREEN)Build Targets:$(RESET)"
	@echo "  make              - Build everything (recommended)"
	@echo "  make all          - Same as 'make'"
	@echo "  make traceviewer  - Build only traceviewer"
	@echo "  make rebuild      - Clean and rebuild everything"
	@echo ""
	@echo "$(GREEN)Utility Targets:$(RESET)"
	@echo "  make clean        - Remove all build files"
	@echo "  make test         - Build and run tests"
	@echo "  make verify       - Verify build components"
	@echo "  make check-deps   - Check build dependencies"
	@echo "  make install      - Install traceviewer to /usr/local/bin"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "$(GREEN)Requirements:$(RESET)"
	@echo "  • gcc              (C compiler)"
	@echo "  • python3          (Python interpreter)"
	@echo "  • python3-dev      (Python development headers)"
	@echo "  • readline-devel   (REQUIRED for tab completion)"
	@echo ""
	@echo "$(GREEN)Install Dependencies:$(RESET)"
	@echo "  $(CYAN)Fedora/RHEL:$(RESET)"
	@echo "    sudo dnf install gcc python3-devel readline-devel"
	@echo ""
	@echo "  $(CYAN)Debian/Ubuntu:$(RESET)"
	@echo "    sudo apt-get install gcc python3-dev libreadline-dev"
	@echo ""
	@echo "  $(CYAN)Arch Linux:$(RESET)"
	@echo "    sudo pacman -S gcc python readline"
	@echo ""
	@echo "$(GREEN)Quick Start:$(RESET)"
	@echo "  make              # Build everything"
	@echo "  make test         # Run tests"
	@echo "  $(PYTHON) idebug.py test_import_main.py  # Run debugger"
	@echo ""
