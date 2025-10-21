#!/bin/bash
# Build and Test Script for Python Debugger

set -e  # Exit on error

echo "========================================"
echo "Python Debugger - Build & Test Script"
echo "========================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}→ $1${NC}"
}

# Check Python version
print_info "Checking Python version..."
PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
print_success "Python version: $PYTHON_VERSION"

# Check for Python dev headers
print_info "Checking for Python development headers..."
if python3 -c "import sysconfig" 2>/dev/null; then
    print_success "Python development headers found"
else
    print_error "Python development headers not found"
    echo ""
    echo "Please install Python development headers:"
    echo "  Ubuntu/Debian: sudo apt-get install python3-dev"
    echo "  macOS: xcode-select --install"
    echo "  Fedora: sudo dnf install python3-devel"
    exit 1
fi

# Clean previous builds
print_info "Cleaning previous builds..."
rm -rf build/
rm -f cdebugger*.so cdebugger*.pyd
rm -f trace.log
print_success "Cleaned"

# Build the extension
print_info "Building C extension..."
python3 setup.py build_ext --inplace
if [ $? -eq 0 ]; then
    print_success "Build successful"
else
    print_error "Build failed"
    exit 1
fi

# Check if .so or .pyd file was created
if [ -f cdebugger.so ] || [ -f cdebugger.pyd ]; then
    print_success "Extension file created"
else
    print_error "Extension file not found"
    exit 1
fi

# Test import
print_info "Testing module import..."
python3 -c "import cdebugger" 2>/dev/null
if [ $? -eq 0 ]; then
    print_success "Module imports successfully"
else
    print_error "Module import failed"
    exit 1
fi

# Run test.py
echo ""
echo "========================================"
echo "Running test.py"
echo "========================================"
python3 runner.py test.py
if [ $? -eq 0 ]; then
    print_success "test.py completed"
else
    print_error "test.py failed"
fi

# Check trace file
echo ""
if [ -f trace.log ]; then
    print_success "Trace file created: trace.log"
    LINES=$(wc -l < trace.log)
    print_info "Trace file contains $LINES lines"
    
    echo ""
    echo "First 5 entries from trace.log:"
    echo "----------------------------------------"
    head -6 trace.log
    echo "----------------------------------------"
else
    print_error "Trace file not found"
fi

# Run crash test
echo ""
echo "========================================"
echo "Running test_crash.py (should crash)"
echo "========================================"
python3 runner.py test_crash.py 2>&1 | head -20
if [ -f trace.log ]; then
    print_success "Crash test trace file created"
fi

# Parse trace file
echo ""
echo "========================================"
echo "Parsing trace file"
echo "========================================"
python3 parse_trace.py trace.log

# Final summary
echo ""
echo "========================================"
echo "Installation Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo "  1. Examine trace.log to understand the format"
echo "  2. Study parse_trace.py to see how to read traces"
echo "  3. Create your CLI using TraceParser as a base"
echo "  4. Integrate your CLI into runner.py"
echo ""
echo "Usage:"
echo "  python3 runner.py <your_script.py> [trace_file]"
echo ""
echo "Documentation:"
echo "  - README.md for full documentation"
echo "  - QUICKSTART.md for quick reference"
echo "  - PROJECT_STRUCTURE.md for file organization"
echo ""
print_success "All tests passed! You're ready to go!"
