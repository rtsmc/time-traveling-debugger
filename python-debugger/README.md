# Python Time-Traveling Debugger

A powerful interactive debugger with breakpoints, step back, and time-travel capabilities. **No code modification required!**

## 🚀 Quick Start

### 1. Build (One-Time Setup)

```bash
# Build the C extension and trace viewer
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2
```

### 2. Debug Any Python File

```bash
python3 idebug.py your_script.py
```

### 3. Set Breakpoints & Run

```
> break your_script.py 25
> break your_script.py 50
> list
> run
```

That's it! The debugger will stop at your breakpoints and let you step forward/backward through execution.

## ✨ Key Features

- 🎯 **Interactive Breakpoints** - Set them before execution, no code changes needed
- ⏪ **Step Backwards** - Travel back in time through execution
- 🔁 **Loop Persistence** - Breakpoints hit on every loop iteration
- 📊 **Execution History** - View up to 1000 previous steps
- 🔍 **Post-Execution Analysis** - Explore complete trace after program finishes

## 📋 Commands

### Pre-Execution (Setup CLI)
```
break <file> <line>  - Set a breakpoint (short: b)
list                 - List all breakpoints (short: l)
clear [num]          - Clear breakpoint(s)
show <file>          - Show file with line numbers
run                  - Start execution (short: r)
help                 - Show help
quit                 - Exit (short: q)
```

### During Execution (At Breakpoint)
```
c - Continue to next breakpoint
n - Step forward (next line)
b - Step back (previous line)
h - Show trace history
q - Quit execution
```

### Post-Execution (Trace Viewer)
```
n             - Next step in trace
back          - Previous step
:<number>     - Jump to execution number
view          - View full source file
summary       - Show trace statistics
find <var>    - Search for variable
break <line>  - Jump to source line
help          - Show help
quit          - Exit (short: q)
```

## 💡 Example Session

```bash
$ python3 idebug.py test_clean_loops.py

╔════════════════════════════════════════════════════════╗
║         Python Time-Traveling Debugger v1.0          ║
╚════════════════════════════════════════════════════════╝

> break test_clean_loops.py 11
✓ Breakpoint set at test_clean_loops.py:11

> run
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_clean_loops.py Line: 11
Hit count: 1
Variables: i=0; total=0

> c    # Continue to next iteration

⚫ BREAKPOINT HIT (Iteration 2)
Hit count: 2
Variables: i=1; total=0

> n    # Step forward

➜ STEP
Line: 12
Variables: i=1; total=1

> b    # Step back!

⟲ TRACE HISTORY
Variables: i=1; total=0
```

## 📁 Files

### Core
- **idebug.py** - Interactive debugger (main tool)
- **debugger.c** - C extension for tracing and breakpoints
- **traceviewer.c** - Post-execution trace viewer
- **setup.py** - Build configuration

### Example Tests
- **test_clean_simple.py** - Simple function test
- **test_clean_loops.py** - Loop tests (for, while, nested)
- **test.py** - Basic test
- **test_crash.py** - Exception handling test
- **test_variables.py** - Variable tracking test

## 🎯 Use Cases

### Debug Without Modifying Code
```bash
# No need to add import statements or breakpoint() calls!
python3 idebug.py production_script.py
> break production_script.py 100
> run
```

### Find Loop Bugs
```bash
python3 idebug.py myloop.py
> break myloop.py 42
> run

# At breakpoint
> c    # Skip to next iteration
> c    # Skip to next iteration
> c    # Found the problem iteration!
> b    # Step back to see what happened
> h    # View execution history
```

### Debug Third-Party Libraries
```bash
# Debug any Python file, even from libraries
python3 idebug.py /path/to/library/module.py
> break module.py 50
> run
```

## 🔧 Requirements

- **Python 3.9 - 3.12+** (fully compatible)
- GCC (for building C extensions)
- Linux/macOS (Windows users: use WSL)

### Installation of Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install python3-dev gcc
```

**macOS:**
```bash
xcode-select --install
```

## 🎉 What Makes This Different?

Unlike traditional debuggers:
- ✅ **No code modification** - Works on any Python file as-is
- ✅ **Time travel** - Step backwards through execution
- ✅ **Loop awareness** - Breakpoints persist across iterations
- ✅ **Interactive setup** - Set breakpoints before execution
- ✅ **Post-mortem** - Explore execution after it completes

## 🐛 Troubleshooting

**"No module named cdebugger"**
```bash
python3 setup.py build_ext --inplace
```

**"gcc: command not found"**
```bash
# Ubuntu/Debian
sudo apt-get install gcc

# macOS
xcode-select --install
```

**"Python.h: No such file"**
```bash
# Ubuntu/Debian
sudo apt-get install python3-dev
```

## 📝 License

Open source - feel free to use and modify!

## 🚀 Get Started Now

```bash
# 1. Build (one time)
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2

# 2. Debug any Python file
python3 idebug.py your_script.py

# 3. Set breakpoints and run!
> break your_script.py 25
> run
```

**Happy debugging! 🎯**
