# Makefile for Python Debugger

# Python executable
PYTHON = python3

# Build the C extension
.PHONY: build
build:
	$(PYTHON) setup.py build_ext --inplace

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf build/
	rm -f cdebugger*.so cdebugger*.pyd
	rm -f trace.log
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete

# Test the debugger with test.py
.PHONY: test
test: build
	$(PYTHON) runner.py test.py

# Test the debugger with crash test
.PHONY: test-crash
test-crash: build
	$(PYTHON) runner.py test_crash.py

# Install (build and verify)
.PHONY: install
install: build
	@echo "Testing import..."
	@$(PYTHON) -c "import cdebugger; print('✓ cdebugger module loaded successfully')"
	@echo "Installation complete!"

# Help
.PHONY: help
help:
	@echo "Python Debugger - Makefile Commands"
	@echo "===================================="
	@echo ""
	@echo "  make build        - Build the C extension"
	@echo "  make clean        - Remove build artifacts and trace files"
	@echo "  make test         - Build and run test.py"
	@echo "  make test-crash   - Build and run test_crash.py"
	@echo "  make install      - Build and verify installation"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Manual Usage:"
	@echo "  python runner.py <python_file> [trace_file]"
	@echo ""
	@echo "Example:"
	@echo "  python runner.py test.py my_trace.log"

# Default target
.DEFAULT_GOAL := help
