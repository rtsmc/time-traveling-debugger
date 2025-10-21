# Python Debugger with Trace File Generation

A C-based Python debugger that generates detailed trace files of program execution for post-mortem debugging and analysis.

## Overview

This debugger runs your Python scripts normally (so you can interact with them) while silently recording every line executed in the background. When your program crashes or hits a breakpoint, you have a complete trace file showing:

- **Execution order** - A counter showing the sequence of execution
- **File location** - Which file and line number was executed
- **Source code** - The actual code on that line
- **Variable state** - All local variables and their values at that moment

## Architecture

The system consists of four main components:

### 1. `cdebugger` (C Extension Module)
The core tracing engine written in C that hooks into Python's internal trace system for maximum performance.

**Key Functions:**
- `start_trace(filename)` - Begins tracing and opens the trace file
- `stop_trace()` - Stops tracing and closes the trace file
- `set_breakpoint(filename, line)` - Sets a breakpoint (placeholder for future CLI integration)
- `get_trace_filename()` - Returns the current trace file path

**How it Works:**
1. Registers a callback function with Python's `PyEval_SetTrace()`
2. Python calls this callback before executing each line
3. The callback extracts execution context (file, line, variables)
4. Writes formatted trace data to the file
5. Returns control to Python to continue normal execution

### 2. `runner.py` (Execution Wrapper)
A Python script that runs your target Python file with tracing enabled.

**Responsibilities:**
- Initializes the C debugger module
- Starts trace file generation
- Executes your Python script
- Catches exceptions and stops tracing
- **Automatically launches the trace viewer CLI on crash**

### 3. `traceviewer` (Interactive CLI Debugger)
A C-based interactive command-line interface for navigating trace files.

**Features:**
- Navigate execution history (forward/backward)
- Jump to specific execution points
- Search for variable usage
- View execution summary
- Color-coded display
- Fast navigation through large trace files

### 4. Trace File (Output)
A pipe-delimited (|||) text file containing the execution history.

**Format:**
```
EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
0|||test.py|||1|||x = 5|||x=5
1|||test.py|||2|||y = 10|||x=5;y=10
2|||test.py|||3|||z = x + y|||x=5;y=10;z=15
```

## Installation

### Prerequisites
- Python 3.7+ with development headers
- GCC or Clang compiler
- Make (optional)

### Build Steps

```bash
# Clone or download the project
cd debugger

# Option 1: Using Make
make build

# Option 2: Using setup.py directly
python setup.py build_ext --inplace

# Verify installation
python -c "import cdebugger; print('Success!')"
```

This creates `cdebugger.so` (Linux/Mac) or `cdebugger.pyd` (Windows) in your directory.

## Usage

### Basic Usage

```bash
python runner.py <your_script.py> [trace_file.log]
```

**Examples:**
```bash
# Run test.py and generate trace.log
python runner.py test.py trace.log

# Or use default trace file name
python runner.py test.py

# Using Make
make test
make test-crash
```

### What Happens

1. **Normal Execution**: Your script runs normally - you can provide input, see output, etc.
2. **Background Tracing**: Every line execution is logged to the trace file
3. **On Crash**: The exception is displayed and trace file location is shown
4. **CLI Launch**: (Future) Your CLI automatically opens with the trace file loaded

### Example Test Script

The included `test.py` demonstrates various operations:
```python
def calculate_sum(a, b):
    result = a + b
    return result

x = 5
y = 10
z = calculate_sum(x, y)
print(f"Result: {z}")
```

Run it:
```bash
python runner.py test.py
# or
make test
```

To test crash handling:
```bash
python runner.py test_crash.py
# or
make test-crash
```

Output:
```
Starting test program...
============================================================
EXCEPTION OCCURRED:
============================================================
Traceback (most recent call last):
  File "test_crash.py", line 11, in <module>
    bad = 1 / 0
ZeroDivisionError: division by zero
============================================================

Trace file saved to: trace.log
Entering debug CLI...
```

## Trace File Format

Each line in the trace file represents one executed line of Python code:

```
EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
```

**Fields:**
1. **EXECUTION_ORDER**: Sequential counter (0, 1, 2, ...) showing execution order
2. **FILENAME**: Full path to the Python file
3. **LINE_NUMBER**: Line number in the source file
4. **CODE**: The actual source code on that line (trimmed)
5. **VARIABLES**: Semicolon-separated list of `name=value` pairs

**Variable Format:**
```
var1=value1;var2=value2;var3=value3
```

**Example Entry:**
```
42|||/home/user/test.py|||7|||z = calculate(x, y)|||x=5;y=10;calculate=<function calculate at 0x7f8b>
```

This tells you:
- This was the 43rd line executed (0-indexed)
- In file `/home/user/test.py`
- On line 7 of that file
- The code was `z = calculate(x, y)`
- At that moment, `x` was 5, `y` was 10, and `calculate` was a function

## Code Deep Dive

### How Tracing Works (debugger.c)

```c
// 1. Python calls this before executing each line
static int trace_callback(PyObject *obj, PyFrameObject *frame, 
                          int what, PyObject *arg)
{
    // 2. Extract execution context
    const char *filename = PyUnicode_AsUTF8(frame->f_code->co_filename);
    int lineno = PyFrame_GetLineNumber(frame);
    
    // 3. Read the source code line from disk
    char *source_line = get_source_line(filename, lineno);
    
    // 4. Get all local variables in current scope
    PyObject *locals = PyFrame_GetLocals(frame);
    
    // 5. Write to trace file
    fprintf(trace_file, "%ld|||%s|||%d|||%s",
            execution_counter++, filename, lineno, source_line);
    write_variables(trace_file, locals);
    fprintf(trace_file, "\n");
    
    // 6. Return to Python to continue execution
    return 0;
}
```

### Variable Extraction

```c
static void write_variables(FILE *fp, PyObject *locals)
{
    // Iterate over all variables in the local scope
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(locals, &pos, &key, &value)) {
        // Get variable name
        const char *var_name = PyUnicode_AsUTF8(key);
        
        // Get string representation of value
        PyObject *repr = PyObject_Repr(value);
        const char *var_value = PyUnicode_AsUTF8(repr);
        
        // Write: name=value
        fprintf(fp, "%s=%s", var_name, var_value);
    }
}
```

### Source Line Reading

```c
static char* get_source_line(const char *filename, int lineno)
{
    FILE *fp = fopen(filename, "r");
    
    // Read line by line until we reach lineno
    char *line = NULL;
    int current_line = 0;
    
    while (getline(&line, &len, fp) != -1) {
        current_line++;
        if (current_line == lineno) {
            return line;  // Found it!
        }
    }
    
    return strdup("<unavailable>");
}
```

## Performance Considerations

### What's Fast
- ✅ C implementation means minimal Python overhead
- ✅ Only traces LINE events (not calls/returns)
- ✅ Direct file I/O without Python marshalling
- ✅ Skips standard library and site-packages

### What's Slow
- ⚠️ Reading source lines from disk on every execution
- ⚠️ Converting all variables to strings
- ⚠️ Writing to disk on every line (no buffering)

### Current Filtering
The tracer automatically skips:
- Standard library files (site-packages)
- Debugger internals (cdebugger)

## Integration with Your CLI

In `runner.py`, replace this placeholder:

```python
# Launch your CLI here
# Example:
# from your_cli import DebugCLI
# cli = DebugCLI(trace_file)
# cli.run()
```

With your actual CLI launch code:

```python
from your_cli import DebugCLI

def run_with_trace(python_file, trace_file="trace.log"):
    # ... existing code ...
    
    except Exception as e:
        cdebugger.stop_trace()
        traceback.print_exc()
        
        # Launch your CLI with the trace file
        cli = DebugCLI(trace_file)
        cli.run()  # Enter interactive mode
```

## Makefile Commands

```bash
make build        # Build the C extension
make clean        # Remove build artifacts and trace files
make test         # Build and run test.py
make test-crash   # Build and run test_crash.py
make install      # Build and verify installation
make help         # Show help message
```

## Future Enhancements

### 1. Breakpoint Support
Add breakpoint tracking and pause execution:
```c
static Breakpoint breakpoints[MAX_BREAKPOINTS];
static int num_breakpoints = 0;

static int check_breakpoint(PyFrameObject *frame) {
    // Check if current line matches any breakpoint
    // If yes, pause and launch CLI
}
```

### 2. Binary Trace Format
For better performance with large traces:
```c
// Instead of text, write binary structs
typedef struct {
    uint64_t exec_order;
    uint32_t lineno;
    uint16_t filename_id;
    // ...
} TraceEntry;

fwrite(&entry, sizeof(TraceEntry), 1, trace_file);
```

### 3. Buffered Writing
Reduce I/O overhead:
```c
#define BUFFER_SIZE 8192
static char trace_buffer[BUFFER_SIZE];
static int buffer_pos = 0;

// Write to buffer, flush when full
```

### 4. Call Stack Tracking
Show function call hierarchy:
```c
static int call_depth = 0;

if (what == PyTrace_CALL) call_depth++;
if (what == PyTrace_RETURN) call_depth--;

fprintf(trace_file, "%ld|||%d|||...", exec_counter, call_depth);
```

### 5. Selective Variable Logging
Only log variables that changed:
```c
static PyObject *last_locals = NULL;

// Compare current locals with last_locals
// Only write variables that changed
```

## Troubleshooting

### Build Errors

**Error: Python.h not found**
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev

# macOS
xcode-select --install

# Fedora
sudo dnf install python3-devel
```

**Error: frameobject.h not found (Python 3.11+)**

For Python 3.11+, you may need to update the includes:
```c
// In debugger.c, replace:
#include <frameobject.h>

// With:
#include <internal/pycore_frame.h>
```

Or use the compatibility layer by adding:
```c
#if PY_VERSION_HEX >= 0x030b0000
#include <internal/pycore_frame.h>
#endif
```

### Runtime Issues

**Module import fails**
```bash
# Verify the .so/.pyd file exists
ls cdebugger.*

# Check Python can find it
python -c "import sys; print(sys.path)"

# Try importing
python -c "import cdebugger"
```

**Trace file is empty**

Check that:
- You have write permissions in the directory
- The file isn't open in another program
- `start_trace()` is called before execution

**Variables show as `<e>`**

This happens when:
- Variable has no string representation
- Object's `__repr__` raises an exception
- Complex objects (file handles, sockets)

## File Structure

```
debugger/
├── debugger.c          # C extension source
├── setup.py            # Build configuration
├── runner.py           # Python execution wrapper
├── Makefile            # Build automation
├── README.md           # This file
├── test.py             # Example test script
├── test_crash.py       # Example crash test
└── trace.log           # Generated trace file (after running)
```

## API Reference

### C Extension (cdebugger)

#### `start_trace(filename: str) -> None`
Begins tracing execution to the specified file.

**Parameters:**
- `filename`: Path to the trace file to create

**Raises:**
- `RuntimeError`: If tracing is already active
- `IOError`: If file cannot be opened

**Example:**
```python
import cdebugger
cdebugger.start_trace("my_trace.log")
```

#### `stop_trace() -> None`
Stops tracing and closes the trace file.

**Example:**
```python
cdebugger.stop_trace()
```

#### `set_breakpoint(filename: str, lineno: int) -> None`
Sets a breakpoint (currently a placeholder).

**Parameters:**
- `filename`: File path
- `lineno`: Line number

#### `get_trace_filename() -> str | None`
Returns the current trace file path or None.

## Example Trace File

Here's what a real trace file looks like:

```
EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
0|||/home/user/test.py|||6|||def calculate_sum(a, b):|||
1|||/home/user/test.py|||15|||def main():|||
2|||/home/user/test.py|||16|||    print("Starting test program...")|||
3|||/home/user/test.py|||19|||    x = 5|||x=5
4|||/home/user/test.py|||20|||    y = 10|||x=5;y=10
5|||/home/user/test.py|||22|||    sum_result = calculate_sum(x, y)|||x=5;y=10
6|||/home/user/test.py|||7|||    result = a + b|||a=5;b=10
7|||/home/user/test.py|||8|||    return result|||a=5;b=10;result=15
8|||/home/user/test.py|||23|||    print(f"Sum of {x} and {y} is: {sum_result}")|||x=5;y=10;sum_result=15
```

## Contributing

Contributions are welcome! Areas for improvement:
- Performance optimizations (buffering, binary format)
- Breakpoint functionality
- Call stack visualization
- Variable diff tracking
- Memory usage optimization

## License

[Your License Here]

## Questions & Support

Common questions:
- **The trace file is huge!** - Consider implementing filtering or compression
- **Variables with circular references?** - The code handles this with error markers
- **How do I integrate my CLI?** - Edit `runner.py` and add your CLI launch code
- **Can I trace only specific files?** - Yes, modify the filter in `trace_callback()`

---

**Next Steps:**
1. Build and test the tracer: `make build && make test`
2. Examine the generated trace.log file
3. Create your CLI to read and navigate trace files
4. Integrate CLI launch into runner.py
5. Add breakpoint support
6. Optimize for your specific use case
