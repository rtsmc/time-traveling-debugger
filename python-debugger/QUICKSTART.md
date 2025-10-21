# Quick Start Guide

## Installation (3 steps)

```bash
# 1. Build the extension
python setup.py build_ext --inplace
# or
make build

# 2. Verify it works
python -c "import cdebugger; print('✓ Success!')"

# 3. Run a test
python runner.py test.py
# or
make test
```

## Basic Usage

```bash
# Run any Python file with tracing
python runner.py your_script.py

# Specify custom trace file
python runner.py your_script.py my_trace.log

# Test crash handling
python runner.py test_crash.py
```

## Understanding Trace Files

Trace files use this format:
```
EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
```

Example:
```
0|||test.py|||5|||x = 5|||x=5
1|||test.py|||6|||y = 10|||x=5;y=10
2|||test.py|||7|||z = x + y|||x=5;y=10;z=15
```

## Parsing Trace Files

Use the included parser:
```bash
python parse_trace.py trace.log
```

Or in your code:
```python
from parse_trace import TraceParser

parser = TraceParser("trace.log")
entry = parser.get_entry(0)  # Get first entry
print(entry.code)             # Print the code
print(entry.variables)        # Print variables
```

## File Overview

- **debugger.c** - C extension (the core tracer)
- **setup.py** - Build configuration
- **runner.py** - Runs Python files with tracing
- **parse_trace.py** - Example trace file parser
- **test.py** - Example test script
- **test_crash.py** - Example crash test
- **Makefile** - Build automation

## Common Commands

```bash
make build       # Build the extension
make clean       # Clean build artifacts
make test        # Run test.py
make test-crash  # Run test_crash.py
make help        # Show all commands
```

## Integrating Your CLI

Edit `runner.py` around line 40:

```python
# Replace this:
# TODO: Launch your CLI here

# With this:
from your_cli import DebugCLI
cli = DebugCLI(trace_file)
cli.run()
```

## Next Steps

1. Build and test the basic functionality
2. Examine the generated trace.log file
3. Use parse_trace.py to understand the format
4. Create your CLI using TraceParser as a starting point
5. Integrate your CLI into runner.py
6. Add features like breakpoints, step-through, etc.

## Troubleshooting

**Can't import cdebugger**
- Run `python setup.py build_ext --inplace`
- Check that cdebugger.so (or .pyd) exists

**Python.h not found**
- Install Python dev headers:
  - Ubuntu: `sudo apt-get install python3-dev`
  - macOS: `xcode-select --install`
  - Fedora: `sudo dnf install python3-devel`

**Trace file is empty**
- Check write permissions
- Make sure the Python script actually executes
- Try running test.py first

## Getting Help

Read the full README.md for detailed documentation.
