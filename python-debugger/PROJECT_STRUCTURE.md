# Python Debugger - Project Structure

## Directory Layout

```
python-debugger/
│
├── debugger.c              # Core C extension - implements tracing
├── setup.py                # Build configuration for C extension
├── runner.py               # Main entry point - runs Python files with tracing
├── parse_trace.py          # Example trace file parser (use for CLI)
│
├── test.py                 # Example test script (successful execution)
├── test_crash.py           # Example test script (crashes)
│
├── Makefile                # Build automation (optional but convenient)
├── .gitignore              # Git ignore patterns
│
├── README.md               # Full documentation
└── QUICKSTART.md           # Quick start guide
```

## Generated Files (after building)

```
python-debugger/
│
├── build/                  # Build artifacts (can be deleted)
│   └── temp.*/
│
├── cdebugger.so            # Compiled C extension (Linux/Mac)
├── cdebugger.pyd           # Compiled C extension (Windows)
│
└── trace.log               # Generated trace file (after running)
```

## File Purposes

### Core Files (DO NOT MODIFY unless you know what you're doing)

- **debugger.c** (1,550 lines)
  - Implements the Python C API hooks
  - Captures execution flow and variable state
  - Writes trace file in real-time
  
- **setup.py** (100 lines)
  - setuptools configuration
  - Compiles C extension

### User-Facing Files (MODIFY these for your needs)

- **runner.py** (120 lines)
  - Wrapper script to run Python files
  - Handles exception catching
  - **Integration point for your CLI** (line ~40)

- **parse_trace.py** (180 lines)
  - Example trace file parser
  - **Use this as a template for your CLI**
  - Shows how to read and navigate trace files

### Testing Files

- **test.py** (70 lines)
  - Sample Python script
  - Tests various Python features
  - Use to verify debugger works

- **test_crash.py** (30 lines)
  - Script that crashes intentionally
  - Tests exception handling

### Documentation

- **README.md** (500+ lines)
  - Complete documentation
  - Architecture explanation
  - API reference
  - Troubleshooting guide

- **QUICKSTART.md** (100 lines)
  - Quick installation guide
  - Basic usage examples
  - Common commands

### Build Files

- **Makefile** (60 lines)
  - Convenient build commands
  - Not required (can use setup.py directly)

- **.gitignore**
  - Excludes build artifacts
  - Excludes trace files from git

## Workflow

### 1. Initial Setup
```bash
# Build the extension
make build
# or
python setup.py build_ext --inplace
```

### 2. Test It Works
```bash
# Run test
make test
# or
python runner.py test.py

# Check trace.log was created
cat trace.log
```

### 3. Develop Your CLI
```bash
# Study the trace format
python parse_trace.py trace.log

# Use TraceParser class in your CLI
# See parse_trace.py for examples
```

### 4. Integrate Your CLI
```python
# Edit runner.py around line 40
from your_cli import DebugCLI
cli = DebugCLI(trace_file)
cli.run()
```

## Key Integration Points

### Where to Add Your CLI

**File: runner.py, Line ~40**
```python
except Exception as e:
    cdebugger.stop_trace()
    traceback.print_exc()
    
    # ADD YOUR CLI HERE:
    from your_cli import DebugCLI
    cli = DebugCLI(trace_file)
    cli.run()
```

### How to Read Trace Files

**Use the TraceParser class from parse_trace.py:**
```python
from parse_trace import TraceParser, TraceEntry

parser = TraceParser("trace.log")

# Get specific entry
entry = parser.get_entry(42)
print(f"Line {entry.lineno}: {entry.code}")

# Get variable history
history = parser.get_variable_history("x")

# Find all executions of a line
entries = parser.find_by_line("test.py", 10)
```

## Customization Points

### 1. Change Trace Format (debugger.c)

Modify the `fprintf` call in `trace_callback`:
```c
// Current format:
fprintf(trace_file, "%ld|||%s|||%d|||%s",
        execution_counter++, filename, lineno, source_line);

// Add custom fields:
fprintf(trace_file, "%ld|||%s|||%d|||%d|||%s",
        execution_counter++, filename, lineno, call_depth, source_line);
```

### 2. Add Filtering (debugger.c)

Modify the filter in `trace_callback`:
```c
// Skip standard library (current)
if (strstr(filename, "site-packages") != NULL) {
    return 0;
}

// Add custom filters:
if (strstr(filename, "/usr/") != NULL) {
    return 0;  // Skip system files
}
```

### 3. Add Breakpoints (debugger.c)

Add global breakpoint storage:
```c
typedef struct {
    char filename[256];
    int lineno;
} Breakpoint;

static Breakpoint breakpoints[100];
static int num_breakpoints = 0;

// Check in trace_callback:
if (check_breakpoint(filename, lineno)) {
    // Pause execution
    // Launch CLI
}
```

### 4. Performance Optimization (debugger.c)

Add buffering:
```c
#define BUFFER_SIZE 8192
static char buffer[BUFFER_SIZE];
static int buf_pos = 0;

// Instead of fprintf, write to buffer
// Flush when full
```

## Development Workflow

### Modify and Test
```bash
# 1. Edit source files
vim debugger.c

# 2. Rebuild
make clean && make build

# 3. Test
make test

# 4. Check output
cat trace.log
```

### Debug Build Issues
```bash
# Verbose build
python setup.py build_ext --inplace -v

# Check for extension
ls cdebugger.*

# Test import
python -c "import cdebugger"
```

## Dependencies

### Build Dependencies
- Python 3.7+ with headers
- GCC or Clang
- Make (optional)

### Runtime Dependencies
- None! Pure Python stdlib

### Install Build Deps
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev build-essential

# macOS
xcode-select --install

# Fedora/RHEL
sudo dnf install python3-devel gcc make
```

## File Sizes (Approximate)

```
debugger.c          ~45 KB (1,550 lines)
setup.py            ~300 bytes
runner.py           ~3.5 KB
parse_trace.py      ~6 KB
test.py             ~2 KB
test_crash.py       ~700 bytes
Makefile            ~1.5 KB
README.md           ~30 KB
QUICKSTART.md       ~3 KB

cdebugger.so        ~50-100 KB (after build)
trace.log           Varies (grows with execution)
```

## Best Practices

1. **Always rebuild after editing C code**
   ```bash
   make clean && make build
   ```

2. **Test with simple scripts first**
   ```bash
   make test
   ```

3. **Check trace.log after each run**
   ```bash
   tail -20 trace.log
   ```

4. **Use parse_trace.py to verify format**
   ```bash
   python parse_trace.py trace.log
   ```

5. **Keep backups before major changes**
   ```bash
   cp debugger.c debugger.c.backup
   ```

## Next Steps

1. ✅ Build the project
2. ✅ Run test.py and examine trace.log
3. ✅ Study parse_trace.py
4. 🔲 Design your CLI interface
5. 🔲 Implement CLI using TraceParser
6. 🔲 Integrate CLI into runner.py
7. 🔲 Add breakpoint support
8. 🔲 Add step-through functionality
9. 🔲 Optimize performance

## Questions?

- Trace file too large? → Add filtering
- Performance slow? → Add buffering
- Need breakpoints? → Implement check_breakpoint()
- Want better format? → Modify fprintf calls
- CLI not launching? → Check runner.py integration

Refer to README.md for detailed documentation!
