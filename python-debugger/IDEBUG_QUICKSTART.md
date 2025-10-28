# 🎯 Interactive Debugger - Quick Start

## ✨ NEW: Debug Without Modifying Code!

### The Problem (Before)
```python
# Had to modify your code:
import cdebugger
cdebugger.set_breakpoint("test.py", 20)  # ← Add this
cdebugger.start_trace("trace.log")       # ← Add this
# ... your code ...
cdebugger.stop_trace()                   # ← Add this
```

### The Solution (Now)
```bash
# Just run with idebug - NO code changes!
python3 idebug.py test.py
```

## 🚀 Usage

```bash
# 1. Launch interactive debugger
python3 idebug.py myprogram.py

# 2. Set breakpoints (CLI opens first)
(debugger) > break myprogram.py 25
(debugger) > break myprogram.py 50
(debugger) > run

# 3. Program runs with breakpoints
# 4. Trace viewer opens automatically
```

## 🎮 Simple Example

```bash
$ python3 idebug.py test_loop_breakpoints.py

╔════════════════════════════════════════════════════════╗
║      Python Interactive Debugger - Setup Mode        ║
╚════════════════════════════════════════════════════════╝

(debugger) > show test_loop_breakpoints.py
# [Shows file with line numbers]

(debugger) > break test_loop_breakpoints.py 24
✓ Breakpoint set at test_loop_breakpoints.py:24

(debugger) > list
Breakpoints:
  1. test_loop_breakpoints.py:24

(debugger) > run

[Program executes, hits breakpoint]

⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24

> n    # Step
> c    # Continue

[Trace viewer launches]
```

## 📋 Commands

### Setup CLI (Before Execution)
```
show <file>         - View file with line numbers
break <file> <line> - Set breakpoint
list                - List breakpoints
clear [num]         - Clear breakpoint(s)
run                 - Start execution
quit                - Exit without running
```

### At Breakpoint (During Execution)
```
n - Step to next line
c - Continue to next breakpoint
q - Quit
```

### Trace Viewer (After Execution)
```
n          - Next step
back       - Previous step
view       - View full file
find <var> - Search variable
quit       - Exit
```

## 🆚 Comparison

| Feature | Old Way | New Way (idebug.py) |
|---------|---------|---------------------|
| Code modification | Required | **Not needed!** |
| Setup | Manual in code | **Interactive CLI** |
| Breakpoints | Hardcoded | **Dynamic** |
| Third-party code | Can't debug | **Can debug!** |
| Production code | Unsafe | **Safe** |

## ✅ Benefits

1. **No Code Changes** - Debug any file as-is
2. **Interactive Setup** - See file, set breakpoints visually
3. **Safe** - Never modify original code
4. **Flexible** - Add/remove breakpoints anytime
5. **Works Everywhere** - Third-party code, production, etc.

## 🎯 Use Cases

### Use Case 1: Debug Third-Party Code
```bash
# Can't modify library code? No problem!
python3 idebug.py /path/to/library/module.py
(debugger) > break module.py 100
(debugger) > run
```

### Use Case 2: Production Debugging
```bash
# Copy production code (don't modify original)
cp /production/app.py ./debug_app.py

# Debug the copy
python3 idebug.py debug_app.py
(debugger) > break debug_app.py 50
(debugger) > run

# Fix bug, apply to production
# No debugger code in production!
```

### Use Case 3: Quick Investigation
```bash
# Just got a Python file, want to understand it
python3 idebug.py mystery_script.py
(debugger) > show mystery_script.py  # Read it
(debugger) > break mystery_script.py 1  # Stop at start
(debugger) > run
> n  # Step through line by line
```

## 📥 Download

**[python-debugger.zip (68 KB)](computer:///mnt/user-data/outputs/python-debugger.zip)**

Includes:
- ✅ idebug.py (interactive debugger)
- ✅ runner.py (simple runner)
- ✅ Complete documentation
- ✅ Test files

## 🔧 Setup

```bash
# One-time setup
cp debugger_with_breakpoints.c debugger.c
rm -rf build/ cdebugger*.so
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2
```

## 🎉 Try It Now!

```bash
# Test with provided examples
python3 idebug.py test_loop_breakpoints.py
python3 idebug.py test_variables.py
python3 idebug.py test.py

# Use with your own code
python3 idebug.py myapp.py
```

## 📚 Documentation

- **INTERACTIVE_DEBUGGER_GUIDE.md** - Complete guide
- **BREAKPOINT_GUIDE.md** - Breakpoint system details
- **LOOP_TESTING_GUIDE.md** - Loop debugging

## 💡 Quick Tips

```bash
# View file first
(debugger) > show myfile.py

# Set multiple breakpoints
(debugger) > break myfile.py 10
(debugger) > break myfile.py 20
(debugger) > break myfile.py 30

# Verify before running
(debugger) > list

# Run!
(debugger) > run

# Step through at breakpoint
> n
> n
> c
```

## 🚀 Summary

**Old way:**
- Modify code
- Add debugger imports
- Hardcode breakpoints
- Can't debug third-party code

**New way (idebug.py):**
- **NO code modification**
- **Interactive CLI**
- **Dynamic breakpoints**
- **Works with ANY Python file**

**Command:**
```bash
python3 idebug.py <any_python_file>
```

That's it! 🎯
