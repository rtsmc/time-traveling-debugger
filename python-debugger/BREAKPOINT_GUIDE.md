# Breakpoint System - Complete Guide

## 🎯 Overview

The debugger now supports **real-time breakpoints**! You can:
- Set breakpoints before running
- Program pauses when it hits a breakpoint
- Step forward line-by-line
- Continue to next breakpoint
- Trace populates as you step

## 🚀 Quick Start

### Using the New Breakpoint-Enabled Debugger

```bash
# 1. Replace debugger.c with the new breakpoint version
cp debugger_with_breakpoints.c debugger.c

# 2. Rebuild
rm -rf build/ cdebugger*.so
python3 setup.py build_ext --inplace

# 3. Rebuild CLI with new commands
gcc -o traceviewer traceviewer.c -Wall -O2

# 4. Run with breakpoints
python3 test_with_breakpoints.py
```

## 📝 How It Works

### Setting Breakpoints

```python
import cdebugger

# Set breakpoints BEFORE starting trace
cdebugger.set_breakpoint("myfile.py", 20)  # Stop at line 20
cdebugger.set_breakpoint("myfile.py", 50)  # Stop at line 50

# Start tracing
cdebugger.start_trace("trace.log")

# Your code runs here...
# When it hits line 20, execution pauses

cdebugger.stop_trace()
```

### When Breakpoint Hits

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_with_breakpoints.py Line: 20
Hit count: 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Commands:
  c - Continue to next breakpoint
  n - Step to next line
  q - Quit execution

> 
```

### Interactive Commands

When paused at a breakpoint:

- **`c`** - Continue execution until next breakpoint
- **`n`** - Step to next line (single step)
- **`q`** - Quit execution immediately

### Stepping Through Code

```
> n

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
➜ STEP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_with_breakpoints.py Line: 21
Code:     y = 20
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> n

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
➜ STEP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_with_breakpoints.py Line: 22
Code:     z = 30
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> c

[continues to next breakpoint...]
```

## 📊 Complete Workflow

### 1. Write Your Script

```python
# myprogram.py
import cdebugger

# Set breakpoints
cdebugger.set_breakpoint("myprogram.py", 15)
cdebugger.set_breakpoint("myprogram.py", 30)

# Start tracing
cdebugger.start_trace("trace.log")

def process_data(data):
    result = []
    for item in data:        # LINE 15 - Breakpoint here
        processed = item * 2
        result.append(processed)
    return result

def main():
    data = [1, 2, 3, 4, 5]
    print("Processing...")
    
    results = process_data(data)
    
    total = sum(results)     # LINE 30 - Breakpoint here
    print(f"Total: {total}")

if __name__ == "__main__":
    main()
    cdebugger.stop_trace()
```

### 2. Run It

```bash
python3 myprogram.py
```

### 3. Interact at Breakpoints

```
✓ Breakpoint set at myprogram.py:15
✓ Breakpoint set at myprogram.py:30
Starting trace to: trace.log
Processing...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: myprogram.py Line: 15
Hit count: 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Commands:
  c - Continue to next breakpoint
  n - Step to next line
  q - Quit execution

> n    # Step to see what happens

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
➜ STEP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: myprogram.py Line: 16
Code:         processed = item * 2
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> c    # Continue to next breakpoint

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: myprogram.py Line: 30
Hit count: 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> c    # Continue to end

Total: 30

Execution completed successfully.
Trace file saved to: trace.log
Launching debugger CLI...
```

### 4. Review in CLI

```bash
# CLI launches automatically
./traceviewer trace.log
```

The trace now contains:
- All steps up to first breakpoint
- All steps you manually stepped through
- All steps between breakpoints
- All steps after last breakpoint

## 🎮 CLI Commands (Updated)

In the trace viewer:

```
n              - Next execution step (in trace)
back           - Previous execution step (in trace)
:<number>      - Jump to execution #N
view           - View full source file
summary        - Show trace summary
find <var>     - Search for variable
break <line>   - Jump to line in trace (renamed from 'jump')
help           - Show help
quit           - Exit
```

Note: `jump` is now `break` to match breakpoint terminology

## 🔧 API Reference

### Python API

```python
import cdebugger

# Set breakpoint
cdebugger.set_breakpoint(filename, line_number)
# Returns: True if successful

# Clear all breakpoints
cdebugger.clear_breakpoints()

# Start tracing (same as before)
cdebugger.start_trace(filename)

# Stop tracing (same as before)
cdebugger.stop_trace()

# Get trace filename (same as before)
filename = cdebugger.get_trace_filename()
```

### Example: Multiple Breakpoints

```python
import cdebugger

# Set multiple breakpoints
cdebugger.set_breakpoint("app.py", 10)
cdebugger.set_breakpoint("app.py", 20)
cdebugger.set_breakpoint("app.py", 30)
cdebugger.set_breakpoint("utils.py", 15)  # Different file

cdebugger.start_trace("trace.log")

# Your code runs, pausing at each breakpoint

cdebugger.stop_trace()
```

### Example: Conditional Debugging

```python
import cdebugger

def debug_mode(enable=True):
    if enable:
        cdebugger.set_breakpoint("app.py", 50)
        cdebugger.start_trace("debug_trace.log")

debug_mode(True)  # Enable debugging

# Your app code...

cdebugger.stop_trace()
```

## 📈 Trace Population Strategy

The trace file is populated dynamically:

1. **Before first breakpoint**: All lines traced automatically
2. **At breakpoint**: Execution pauses, trace includes everything up to this point
3. **Stepping (n)**: Each step adds one line to trace
4. **Continuing (c)**: All lines traced until next breakpoint
5. **After last breakpoint**: All remaining lines traced

This means your trace file grows as you interact with the debugger!

## 🎯 Use Cases

### 1. Debug Specific Function

```python
cdebugger.set_breakpoint("app.py", 100)  # Function entry
cdebugger.set_breakpoint("app.py", 150)  # Function exit

# Step through function line by line
```

### 2. Inspect Loop Iteration

```python
cdebugger.set_breakpoint("app.py", 50)  # Inside loop

# Hit 'c' to see each iteration
# Hit 'n' to step through one iteration
```

### 3. Debug Conditionals

```python
cdebugger.set_breakpoint("app.py", 75)  # Before if statement

# Step through to see which branch executes
```

### 4. Find Bug Location

```python
# Set breakpoints at suspicious locations
cdebugger.set_breakpoint("app.py", 100)
cdebugger.set_breakpoint("app.py", 200)
cdebugger.set_breakpoint("app.py", 300)

# Continue through until you find the problem
```

## 🔄 Comparison: Old vs New

### OLD System (trace-only)
```
1. Run program
2. Program executes completely
3. View trace after completion
4. Can only go backward/forward in recorded trace
```

### NEW System (breakpoints)
```
1. Set breakpoints
2. Run program
3. Pauses at breakpoints
4. Step forward (populates trace as you go)
5. Continue to next breakpoint
6. View complete trace after
7. Can see exactly what happened when
```

## ⚡ Performance Notes

- Breakpoints have minimal overhead when not hit
- Stepping is interactive (real-time)
- Trace file grows as you step
- No limit on number of breakpoints

## 🐛 Troubleshooting

### Breakpoints Not Hit

Check:
1. Filename must match exactly (use absolute path if needed)
2. Line number must be executable code (not comments/blank)
3. Breakpoint must be set BEFORE `start_trace()`

### Program Doesn't Pause

Make sure:
1. You're using the NEW debugger (`debugger_with_breakpoints.c`)
2. Rebuilt the extension: `python3 setup.py build_ext --inplace`
3. Breakpoint is on actual code line

### Can't Step Forward

- Make sure you typed 'n' not 'N'
- Check terminal supports interactive input
- Try 'c' to continue instead

## 📚 Complete Example

```python
#!/usr/bin/env python3
"""
Complete example showing breakpoint debugging
"""
import cdebugger

# Configure breakpoints
cdebugger.set_breakpoint(__file__, 25)
cdebugger.set_breakpoint(__file__, 35)

# Start tracing
cdebugger.start_trace("complete_example.log")

def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)

def main():
    print("Fibonacci Calculator")
    
    # Breakpoint 1 will hit here (line 25)
    for i in range(5):
        result = fibonacci(i)
        print(f"fib({i}) = {result}")
    
    print("Calculation complete")
    
    # Breakpoint 2 will hit here (line 35)
    total = sum(fibonacci(i) for i in range(5))
    print(f"Sum: {total}")

if __name__ == "__main__":
    main()
    cdebugger.stop_trace()
    
    print("\n✓ Trace complete! View with: ./traceviewer complete_example.log")
```

## 🎉 Summary

**You now have:**
- ✅ Real-time breakpoints
- ✅ Interactive stepping
- ✅ Dynamic trace population
- ✅ Full control over execution
- ✅ Complete trace for post-analysis

**The debugger is now a full-featured development tool!** 🚀
