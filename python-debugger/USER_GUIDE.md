# Time-Traveling Python Debugger - User Guide

Complete guide for the integrated C-based Python debugger with interactive trace viewer.

## Quick Start

```bash
# 1. Build everything
make all

# 2. Run a Python file with tracing
python3 runner.py test.py

# 3. Test crash handling (auto-launches CLI)
make test-crash

# 4. Manually view a trace file
./traceviewer trace.log
```

## Features

✅ **Real-time tracing** - Captures every line execution  
✅ **Variable tracking** - Records all variable states  
✅ **Crash recovery** - Auto-launches debugger on exceptions  
✅ **Time travel** - Navigate execution history forward/backward  
✅ **Fast search** - Find variable usage instantly  
✅ **Color-coded UI** - Easy-to-read terminal interface  
✅ **Zero dependencies** - Pure C and Python stdlib  

## Installation

### Prerequisites
- Python 3.7+ with development headers
- GCC compiler
- Make (optional)

### Build

```bash
# Build everything at once
make all

# Or build components separately
python3 setup.py build_ext --inplace  # Python extension
gcc -o traceviewer traceviewer.c -Wall  # Trace viewer CLI

# Verify
python3 -c 'import cdebugger; print("✓ Extension loaded")'
./traceviewer --help 2>/dev/null || echo "✓ CLI compiled"
```

## Usage

### Running Python Files with Tracing

```bash
# Basic usage
python3 runner.py your_script.py

# Specify custom trace file
python3 runner.py your_script.py my_trace.log

# Using Make
make test         # Run test.py
make test-crash   # Run crash test (auto-launches CLI)
```

### Interactive Trace Viewer

The trace viewer automatically launches when your Python program crashes. You can also launch it manually:

```bash
./traceviewer trace.log
```

### CLI Commands

```
Commands:
  n              - Next execution step (step forward in time)
  back           - Previous execution step (step backward in time)
  :<number>      - Jump to execution #N (e.g., :100)
  summary        - Show trace statistics
  find <var>     - Search for variable usage (e.g., find x)
  jump <line>    - Jump to source line (e.g., jump 42)
  help           - Show help
  quit or q      - Exit
```

### Example Session

```
$ python3 runner.py test_crash.py

[... program runs and crashes ...]

Launching debugger CLI...

╔════════════════════════════════════════════════════════╗
║         Python Time-Traveling Debugger v1.0          ║
╚════════════════════════════════════════════════════════╝

✓ Loaded 25 execution steps

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #0]
File: test_crash.py Line: 5
Code: def dangerous_function(x, y):
Variables: (none)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[1/25] > n

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #1]
File: test_crash.py Line: 10
Code:     print("Starting crash test...")
Variables:
  • a=10
  • b=5
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[2/25] > find c

Searching for variable 'c'...

[15] test_crash.py:18
  → c=0

Found 1 occurrence(s).

[2/25] > :15

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #15]
File: test_crash.py Line: 18
Code:     c = 0
Variables:
  • a=10
  • b=5
  • result1=2.0
  • c=0
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[16/25] > n

[... execution continues to crash point ...]

[16/25] > quit

Goodbye! Happy debugging! 🐛
```

## Trace File Format

```
EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
0|||test.py|||5|||x = 5|||x=5
1|||test.py|||6|||y = 10|||x=5;y=10
2|||test.py|||7|||z = x + y|||x=5;y=10;z=15
```

**Fields:**
1. **Execution Order** - Sequential counter (0, 1, 2, ...)
2. **Filename** - Full path to source file
3. **Line Number** - Source line number
4. **Code** - Actual source code
5. **Variables** - All local variables as `name=value;name=value`

## File Structure

```
debugger/
├── debugger.c           # C extension (trace generator)
├── traceviewer.c        # C CLI (trace navigator)
├── setup.py             # Build config for extension
├── runner.py            # Python execution wrapper
├── parse_trace.py       # Python trace parser (for reference)
├── Makefile             # Build automation
├── build_and_test.sh    # Automated setup script
├── test.py              # Example working script
├── test_crash.py        # Example crash script
└── README.md            # This file
```

## Advanced Usage

### Searching for Variables

```bash
[1/25] > find x

Searching for variable 'x'...

[5] test.py:10
  → x=5

[6] test.py:11
  → x=5

[10] test.py:15
  → x=10

Found 3 occurrence(s).
```

### Jumping to Specific Points

```bash
# Jump to execution #100
[1/500] > :100

# Jump to source line 42
[1/500] > jump 42

# Go back 10 steps
[100/500] > back
[99/500] > back
[98/500] > back
```

### Viewing Summary

```bash
[1/25] > summary

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Trace Summary
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Total Executions: 25
First Entry: [0] test_crash.py:5
Last Entry:  [24] test_crash.py:19
Current Position: [0] (Entry 1 of 25)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Makefile Commands

```bash
make all          # Build everything
make build        # Build Python extension only
make viewer       # Build trace viewer only
make clean        # Remove all build artifacts
make test         # Run test.py
make test-crash   # Run crash test (launches CLI)
make test-viewer  # Test viewer with existing trace
make install      # Build and verify everything
make help         # Show all commands
```

## Troubleshooting

### Python Extension Issues

**Error: `PyFrame_GetLocals` not found**

This is a Python 3.9 compatibility issue. Edit `debugger.c` line 124:

```c
// Change from:
PyObject *locals = PyFrame_GetLocals(frame);

// To:
PyObject *locals = frame->f_locals;
if (locals == NULL) {
    locals = PyDict_New();
} else {
    Py_INCREF(locals);
}
```

Then rebuild:
```bash
make clean && make build
```

**Error: `Python.h` not found**

Install Python development headers:
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev

# Fedora/RHEL
sudo dnf install python3-devel

# macOS
xcode-select --install
```

### Trace Viewer Issues

**Trace viewer doesn't launch automatically**

Compile it manually:
```bash
gcc -o traceviewer traceviewer.c -Wall
```

**Can't see colors in terminal**

Your terminal may not support ANSI colors. The viewer will still work, just without colors.

**Trace file is empty**

Make sure:
1. `cdebugger` extension is properly loaded
2. You have write permissions
3. The Python script actually executes

## Performance Tips

### Large Trace Files

For programs with many execution steps:

1. **Filter during tracing** - Modify `debugger.c` to skip certain files
2. **Use binary format** - Implement binary trace format for speed
3. **Add buffering** - Reduce I/O overhead in `debugger.c`

### Memory Usage

The trace viewer loads the entire trace into memory. For very large traces (>100k lines):

1. Consider implementing streaming/pagination
2. Split trace files by module/function
3. Use compression (gzip trace files)

## Customization

### Changing Trace Format

Edit `debugger.c` around line 127:

```c
// Current format
fprintf(trace_file, "%ld|||%s|||%d|||%s",
        execution_counter++, filename, lineno, source_line);

// Add custom fields (e.g., timestamp, function name)
fprintf(trace_file, "%ld|||%s|||%d|||%s|||%s",
        execution_counter++, filename, lineno, 
        function_name, source_line);
```

Update `traceviewer.c` parsing logic accordingly.

### Adding New Commands

Edit `traceviewer.c` in the main command loop (around line 300):

```c
// Add new command
else if (strcmp(cmd, "mycommand") == 0) {
    // Your command logic here
}
```

### Filtering Traced Files

Edit `debugger.c` in `trace_callback()`:

```c
// Skip specific files
if (strstr(filename, "skip_this") != NULL) {
    return 0;  // Don't trace this file
}
```

## API Reference

### Python API (cdebugger)

```python
import cdebugger

# Start tracing to file
cdebugger.start_trace("trace.log")

# Your code here...

# Stop tracing
cdebugger.stop_trace()

# Get current trace filename
filename = cdebugger.get_trace_filename()

# Set breakpoint (placeholder)
cdebugger.set_breakpoint("test.py", 42)
```

### Command Line

```bash
# Run with tracing
python3 runner.py script.py [trace_file]

# View trace
./traceviewer trace.log
```

## Examples

### Example 1: Debug a Crash

```bash
# Run program (it crashes)
python3 runner.py buggy_script.py

# CLI auto-launches, navigate to crash point
> :500          # Jump near end
> back          # Step back to see what happened
> find error    # Search for error variable
> summary       # See execution stats
```

### Example 2: Analyze Variable Changes

```bash
./traceviewer trace.log

> find counter
# See all points where 'counter' appears

> :10
# Jump to first occurrence

> n
> n
> n
# Step through to see how counter changes
```

### Example 3: Find Performance Bottleneck

```bash
./traceviewer trace.log

> summary
# Check total execution count

> jump 100
# Jump to suspected slow function

> find loop_var
# See how many times loop executed
```

## Contributing

Areas for enhancement:
- Breakpoint functionality
- Call stack visualization  
- Performance profiling
- GUI interface
- Remote debugging

## License

[Add your license]

## Support

For issues or questions:
1. Check this README
2. Review the code comments
3. Examine example files

---

**Happy debugging! 🐛 Navigate through time and squash those bugs!**
