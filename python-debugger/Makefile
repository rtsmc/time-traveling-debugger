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
	@echo -e "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo -e "$(CYAN)║         Building Time-Traveling Debugger               ║$(RESET)"
	@echo -e "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo -e ""
	@$(MAKE) -s traceviewer
	@$(MAKE) -s cdebugger
	@echo -e ""
	@$(MAKE) -s verify
	@echo -e ""
	@echo -e "$(GREEN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo -e "$(GREEN)║              Build Complete! 🎉                        ║$(RESET)"
	@echo -e "$(GREEN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo -e ""
	@echo -e "$(CYAN)To run the debugger:$(RESET)"
	@echo -e "  $(PYTHON) idebug.py test_import_main.py"
	@echo -e ""

# Check dependencies before building
check-deps:
	@echo -e "$(CYAN)Checking dependencies...$(RESET)"
	@command -v $(CC) >/dev/null 2>&1 || { echo -e "$(RED)✗ gcc not found. Please install gcc.$(RESET)"; exit 1; }
	@echo -e "$(GREEN)✓$(RESET) gcc found"
	@command -v $(PYTHON) >/dev/null 2>&1 || { echo -e "$(RED)✗ python3 not found. Please install python3.$(RESET)"; exit 1; }
	@echo -e "$(GREEN)✓$(RESET) python3 found"
ifneq ($(PYTHON_CHECK),yes)
	@echo -e "$(RED)✗ Python development headers not found$(RESET)"
	@echo -e "$(YELLOW)Install python3-dev:$(RESET)"
	@echo -e "  Fedora/RHEL:  sudo dnf install python3-devel"
	@echo -e "  Debian/Ubuntu: sudo apt-get install python3-dev"
	@echo -e "  Arch:          sudo pacman -S python"
	@exit 1
else
	@echo -e "$(GREEN)✓$(RESET) python3-dev found"
endif
ifneq ($(READLINE_CHECK),yes)
	@echo -e "$(RED)✗ readline not found (required)$(RESET)"
	@echo -e "$(YELLOW)Install readline development library:$(RESET)"
	@echo -e "  Fedora/RHEL:  sudo dnf install readline-devel"
	@echo -e "  Debian/Ubuntu: sudo apt-get install libreadline-dev"
	@echo -e "  Arch:          sudo pacman -S readline"
	@echo -e "  macOS:         brew install readline"
	@exit 1
else
	@echo -e "$(GREEN)✓$(RESET) readline found"
endif
	@echo -e ""

# Build traceviewer
traceviewer: traceviewer.c
	@echo -e "$(CYAN)Building traceviewer (with readline support)...$(RESET)"
	@mkdir -p build
	$(CC) -o build/traceviewer traceviewer.c $(CFLAGS) $(READLINE_FLAGS)
	@chmod +x build/traceviewer 2>/dev/null || true
	@if [ -x build/traceviewer ]; then \
		echo -e "$(GREEN)✓ traceviewer built successfully$(RESET)"; \
	else \
		echo -e "$(RED)✗ Failed to build traceviewer$(RESET)"; \
		exit 1; \
	fi

PYTHON_INCLUDES    := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_path('include'))")
PYTHON_EXT_SUFFIX  := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))")
CDEBUGGER_SO       = cdebugger$(PYTHON_EXT_SUFFIX)

cdebugger: debugger.c
	@echo -e "$(CYAN)Building cdebugger extension...$(RESET)"
	$(CC) -shared -fPIC -O2 \
		-I$(PYTHON_INCLUDES) \
		-o $(CDEBUGGER_SO) \
		debugger.c
	@if [ -f $(CDEBUGGER_SO) ]; then \
		echo -e "$(GREEN)✓ cdebugger built: $(CDEBUGGER_SO)$(RESET)"; \
	else \
		echo -e "$(RED)✗ Failed to build cdebugger$(RESET)"; \
		exit 1; \
	fi

# Verify build
verify:
	@echo -e "$(CYAN)Verifying build...$(RESET)"
	@ERRORS=0; \
	if [ ! -x build/traceviewer ]; then \
		echo -e "$(RED)✗ traceviewer not found or not executable$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	else \
		echo -e "$(GREEN)✓$(RESET) traceviewer is ready"; \
	fi; \
	if ! ls cdebugger*.so >/dev/null 2>&1; then \
		echo -e "$(RED)✗ cdebugger extension not found$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	else \
		echo -e "$(GREEN)✓$(RESET) cdebugger extension is ready"; \
	fi; \
	if $(PYTHON) -c "import cdebugger" 2>/dev/null; then \
		echo -e "$(GREEN)✓$(RESET) cdebugger module imports correctly"; \
	else \
		echo -e "$(RED)✗ cdebugger module cannot be imported$(RESET)"; \
		ERRORS=$$((ERRORS + 1)); \
	fi; \
	if [ $$ERRORS -gt 0 ]; then \
		echo -e ""; \
		echo -e "$(RED)Build verification failed with $$ERRORS error(s)$(RESET)"; \
		exit 1; \
	fi

# Clean build artifacts
clean:
	@echo -e "$(CYAN)Cleaning build files...$(RESET)"
	@rm -rf build/
	@rm -f cdebugger*.so
	@echo -e "$(GREEN)✓ Clean complete$(RESET)"

# Build everything from scratch
rebuild: clean all

# Run tests
test: all
	@echo -e ""
	@echo -e "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo -e "$(CYAN)║                  Running Tests                         ║$(RESET)"
	@echo -e "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo -e ""
	@echo -e "Test 1: Module import test..."
	@if $(PYTHON) -c "import cdebugger; print('  $(GREEN)✓$(RESET) cdebugger imports correctly')" 2>/dev/null; then :; else \
		echo -e "  $(RED)✗$(RESET) cdebugger import failed"; \
	fi
	@echo -e ""
	@echo -e "Test 2: Running debugger..."
	@if echo -e "r" | $(PYTHON) idebug.py test_clean_simple.py >/dev/null 2>&1; then \
		echo -e "  $(GREEN)✓$(RESET) Debugger runs successfully"; \
	else \
		echo -e "  $(RED)✗$(RESET) Debugger failed"; \
	fi
	@echo -e ""
	@if [ -f trace.log ]; then \
		echo -e "Test 3: Testing show command..."; \
		if echo -e -e "show\nq" | ./traceviewer trace.log 2>&1 | grep -q "File:"; then \
			echo -e "  $(GREEN)✓$(RESET) Show command works"; \
		else \
			echo -e "  $(RED)✗$(RESET) Show command failed"; \
		fi; \
		echo -e ""; \
		echo -e "$(GREEN)All tests completed!$(RESET)"; \
	else \
		echo -e "$(YELLOW)⚠ trace.log not found, skipping trace viewer test$(RESET)"; \
	fi

# Install to system
install: all
	@echo -e "$(CYAN)Installing to /usr/local/bin...$(RESET)"
	@sudo cp traceviewer /usr/local/bin/ || { echo -e "$(RED)✗ Installation failed$(RESET)"; exit 1; }
	@sudo chmod +x /usr/local/bin/traceviewer
	@echo -e "$(GREEN)✓ Installed traceviewer to /usr/local/bin/$(RESET)"
	@echo -e ""
	@echo -e "$(YELLOW)Note:$(RESET) idebug.py must be run from the project directory"

# Show help
help:
	@echo -e "$(CYAN)╔════════════════════════════════════════════════════════╗$(RESET)"
	@echo -e "$(CYAN)║   Python Time-Traveling Debugger - Build System        ║$(RESET)"
	@echo -e "$(CYAN)╚════════════════════════════════════════════════════════╝$(RESET)"
	@echo -e ""
	@echo -e "$(GREEN)Build Targets:$(RESET)"
	@echo -e "  make              - Build everything (recommended)"
	@echo -e "  make all          - Same as 'make'"
	@echo -e "  make traceviewer  - Build only traceviewer"
	@echo -e "  make rebuild      - Clean and rebuild everything"
	@echo -e ""
	@echo -e "$(GREEN)Utility Targets:$(RESET)"
	@echo -e "  make clean        - Remove all build files"
	@echo -e "  make test         - Build and run tests"
	@echo -e "  make verify       - Verify build components"
	@echo -e "  make check-deps   - Check build dependencies"
	@echo -e "  make install      - Install traceviewer to /usr/local/bin"
	@echo -e "  make help         - Show this help message"
	@echo -e ""
	@echo -e "$(GREEN)Requirements:$(RESET)"
	@echo -e "  • gcc              (C compiler)"
	@echo -e "  • python3          (Python interpreter)"
	@echo -e "  • python3-dev      (Python development headers)"
	@echo -e "  • readline-devel   (REQUIRED for tab completion)"
	@echo -e ""
	@echo -e "$(GREEN)Install Dependencies:$(RESET)"
	@echo -e "  $(CYAN)Fedora/RHEL:$(RESET)"
	@echo -e "    sudo dnf install gcc python3-devel readline-devel"
	@echo -e ""
	@echo -e "  $(CYAN)Debian/Ubuntu:$(RESET)"
	@echo -e "    sudo apt-get install gcc python3-dev libreadline-dev"
	@echo -e ""
	@echo -e "  $(CYAN)Arch Linux:$(RESET)"
	@echo -e "    sudo pacman -S gcc python readline"
	@echo -e ""
	@echo -e "$(GREEN)Quick Start:$(RESET)"
	@echo -e "  make              # Build everything"
	@echo -e "  make test         # Run tests"
	@echo -e "  $(PYTHON) idebug.py test_import_main.py  # Run debugger"
	@echo -e ""
