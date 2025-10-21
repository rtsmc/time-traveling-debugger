# Makefile for Python Debugger

# Python executable
PYTHON = python3

# C compiler
CC = gcc
CFLAGS = -Wall -O2

# Build everything
.PHONY: all
all: build viewer

# Build the Python C extension
.PHONY: build
build:
	$(PYTHON) setup.py build_ext --inplace

# Build the trace viewer CLI
.PHONY: viewer
viewer:
	$(CC) $(CFLAGS) -o traceviewer traceviewer.c

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf build/
	rm -f cdebugger*.so cdebugger*.pyd
	rm -f traceviewer
	rm -f trace.log
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete

# Test the debugger with test.py
.PHONY: test
test: all
	$(PYTHON) runner.py test.py

# Test the debugger with crash test
.PHONY: test-crash
test-crash: all
	$(PYTHON) runner.py test_crash.py

# Test the trace viewer directly
.PHONY: test-viewer
test-viewer: viewer
	@if [ -f trace.log ]; then \
		./traceviewer trace.log; \
	else \
		echo "No trace.log found. Run 'make test' first."; \
	fi

# Install (build and verify)
.PHONY: install
install: all
	@echo "Testing import..."
	@$(PYTHON) -c "import cdebugger; print('✓ cdebugger module loaded successfully')"
	@echo "Testing trace viewer..."
	@./traceviewer --help 2>/dev/null || echo "✓ traceviewer compiled successfully"
	@echo "Installation complete!"

# Help
.PHONY: help
help:
	@echo "Python Debugger - Makefile Commands"
	@echo "===================================="
	@echo ""
	@echo "  make all          - Build everything (Python extension + trace viewer)"
	@echo "  make build        - Build the Python C extension"
	@echo "  make viewer       - Build the trace viewer CLI"
	@echo "  make clean        - Remove build artifacts and trace files"
	@echo "  make test         - Build and run test.py"
	@echo "  make test-crash   - Build and run test_crash.py (launches CLI)"
	@echo "  make test-viewer  - Test the trace viewer with existing trace.log"
	@echo "  make install      - Build and verify installation"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Manual Usage:"
	@echo "  python runner.py <python_file> [trace_file]"
	@echo "  ./traceviewer <trace_file>"
	@echo ""
	@echo "Example:"
	@echo "  make test-crash   # Runs test, crashes, auto-launches CLI"
	@echo "  ./traceviewer trace.log  # Manually open trace file"

# Default target
.DEFAULT_GOAL := help
