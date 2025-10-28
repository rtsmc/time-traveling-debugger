# Interactive Debugger (idebug.py) - Complete Guide

## 🎯 What's Different?

### OLD Way (Modify Code)
```python
# Had to modify your code:
import cdebugger
cdebugger.set_breakpoint("test.py", 20)
cdebugger.start_trace("trace.log")
# ... your code ...
cdebugger.stop_trace()
```

### NEW Way (No Code Modification!)
```bash
# Just run with idebug:
python3 idebug.py test.py

# CLI opens BEFORE execution
# Set breakpoints interactively
# No code changes needed!
```

## 🚀 Quick Start

```bash
# 1. Run any Python file with idebug
python3 idebug.py myprogram.py

# 2. CLI opens - set breakpoints
(debugger) > break myprogram.py 25
(debugger) > break myprogram.py 50
(debugger) > list
(debugger) > run

# 3. Program runs with breakpoints
# 4. Post-execution CLI launches automatically
```

## 📋 Complete Workflow Example

```bash
$ python3 idebug.py test_loop_breakpoints.py

╔════════════════════════════════════════════════════════╗
║      Python Interactive Debugger - Setup Mode        ║
╚════════════════════════════════════════════════════════╝

Set breakpoints and configure debugging before execution.

Commands:
  break <file> <line>  - Set a breakpoint
  list                 - List all breakpoints
  clear [num]          - Clear breakpoint(s)
  show <file>          - Show file with line numbers
  run                  - Start execution
  help                 - Show this help
  quit                 - Exit without running

Example:
  > break test.py 25
  > break test.py 50
  > list
  > run

(debugger) > show test_loop_breakpoints.py

============================================================
File: test_loop_breakpoints.py
============================================================
     1 | """
     2 | Breakpoint Test with Loops
     3 | Tests breakpoint behavior in various loop scenarios
     4 | """
     5 | 
     6 | import cdebugger
...
    22 |     for i in range(5):
    23 |         # BREAKPOINT at line 24 - will hit 5 times!
    24 |         total += i
    25 |         print(f"  i={i}, total={total}")
...
============================================================
Total lines: 88

(debugger) > break test_loop_breakpoints.py 24

✓ Breakpoint set at test_loop_breakpoints.py:24

(debugger) > break test_loop_breakpoints.py 32

✓ Breakpoint set at test_loop_breakpoints.py:32

(debugger) > list

Breakpoints:
──────────────────────────────────────────────────────────
  1. test_loop_breakpoints.py:24
  2. test_loop_breakpoints.py:32
──────────────────────────────────────────────────────────
Total: 2 breakpoint(s)

(debugger) > run

============================================================
Starting execution...
============================================================
Breakpoints: 2
  • test_loop_breakpoints.py:24
  • test_loop_breakpoints.py:32
============================================================

✓ Breakpoint set at test_loop_breakpoints.py:24
✓ Breakpoint set at test_loop_breakpoints.py:32

Starting trace to: trace.log

[Program runs with breakpoints...]

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_loop_breakpoints.py Line: 24
Hit count: 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Commands:
  c - Continue to next breakpoint
  n - Step to next line
  q - Quit execution

> n
[... continues ...]

[After execution, trace viewer CLI launches]
```

## 🎮 Commands Reference

### Pre-Execution Commands (Setup CLI)

| Command | Short | Description | Example |
|---------|-------|-------------|---------|
| `break <file> <line>` | `b` | Set a breakpoint | `break test.py 25` |
| `list` | `l` | List all breakpoints | `list` |
| `clear [num]` | - | Clear breakpoint(s) | `clear 1` or `clear all` |
| `show <file>` | - | Show file with line numbers | `show test.py` |
| `run` | `r` | Start execution | `run` |
| `help` | - | Show help | `help` |
| `quit` | `q` | Exit without running | `quit` |

### During Execution (At Breakpoint)

| Command | Description |
|---------|-------------|
| `n` | Step to next line |
| `c` | Continue to next breakpoint |
| `q` | Quit execution |

### Post-Execution (Trace Viewer)

| Command | Description |
|---------|-------------|
| `n` | Next step in trace |
| `back` | Previous step |
| `view` | View full source |
| `find <var>` | Search for variable |
| `break <line>` | Jump to line |
| `summary` | Show stats |
| `quit` | Exit |

## 📝 Detailed Command Examples

### Setting Breakpoints

```bash
# Basic breakpoint
(debugger) > break myfile.py 25
✓ Breakpoint set at myfile.py:25

# Multiple breakpoints
(debugger) > break myfile.py 10
(debugger) > break myfile.py 20
(debugger) > break myfile.py 30

# Breakpoints in different files
(debugger) > break main.py 15
(debugger) > break utils.py 42
(debugger) > break helpers.py 100
```

### Viewing Files

```bash
# Show target file
(debugger) > show test.py

# Show specific file
(debugger) > show utils.py

# Lines with breakpoints marked with ⚫
  ⚫  25 |     result = calculate(x, y)
```

### Managing Breakpoints

```bash
# List all breakpoints
(debugger) > list

Breakpoints:
──────────────────────────────────────────────────────────
  1. test.py:10
  2. test.py:20
  3. test.py:30
  4. utils.py:42
──────────────────────────────────────────────────────────
Total: 4 breakpoint(s)

# Clear specific breakpoint
(debugger) > clear 2
✓ Cleared breakpoint at test.py:20

# Clear all breakpoints
(debugger) > clear all
✓ All breakpoints cleared
```

### Running

```bash
# Start execution with breakpoints
(debugger) > run

# Or use shorthand
(debugger) > r
```

## 🎯 Complete Usage Examples

### Example 1: Debug Existing Code (No Modifications)

```bash
# You have: calculator.py
# You want to: debug it without changing code

$ python3 idebug.py calculator.py

(debugger) > show calculator.py
# [See the file with line numbers]

(debugger) > break calculator.py 15
(debugger) > break calculator.py 25
(debugger) > list
(debugger) > run

# Program runs with breakpoints!
```

### Example 2: Debug Loop Without Code Changes

```bash
$ python3 idebug.py myprogram.py

(debugger) > show myprogram.py
# Find the loop you want to debug

(debugger) > break myprogram.py 42  # Inside loop
(debugger) > run

# At breakpoint in loop:
> n    # Step through first iteration
> n    # Step through second iteration
> c    # Continue rest of loop
```

### Example 3: Multiple Files

```bash
$ python3 idebug.py main.py

(debugger) > break main.py 20
(debugger) > break utils.py 50
(debugger) > break helpers.py 100
(debugger) > list
(debugger) > run

# Breakpoints work across all files!
```

### Example 4: Quick Debug Session

```bash
$ python3 idebug.py app.py

(debugger) > break app.py 100
(debugger) > run

# At breakpoint:
> n    # Step a few times
> n
> n
> c    # Continue

[Trace viewer opens]
> find x           # Search for variable
> view             # See full file
> quit
```

## 🆚 Comparison: Old vs New

### Debugging test_loop_breakpoints.py

**OLD WAY:**
```python
# Had to modify test_loop_breakpoints.py:
import cdebugger
cdebugger.set_breakpoint("test_loop_breakpoints.py", 24)
cdebugger.start_trace("trace.log")
# ... rest of code ...
cdebugger.stop_trace()
```

**NEW WAY:**
```bash
# No code modification needed!
python3 idebug.py test_loop_breakpoints.py

(debugger) > break test_loop_breakpoints.py 24
(debugger) > run
```

## 💡 Pro Tips

### Tip 1: Preview File Before Setting Breakpoints

```bash
(debugger) > show myfile.py
# [Read the file, find interesting lines]

(debugger) > break myfile.py 42
(debugger) > run
```

### Tip 2: Set Multiple Breakpoints at Once

```bash
(debugger) > break app.py 10
(debugger) > break app.py 20
(debugger) > break app.py 30
(debugger) > break app.py 40
(debugger) > list      # Verify
(debugger) > run
```

### Tip 3: Clear and Reset

```bash
(debugger) > list
# Oops, set wrong breakpoints

(debugger) > clear all
(debugger) > break app.py 50    # Set correct one
(debugger) > run
```

### Tip 4: Exit Without Running

```bash
(debugger) > break app.py 10
(debugger) > break app.py 20

# Changed your mind?
(debugger) > quit
Exiting without execution
```

### Tip 5: Use with Any Python File

```bash
# Works with ANY Python file
python3 idebug.py some_third_party_lib.py
python3 idebug.py downloaded_script.py
python3 idebug.py production_code.py

# No code modifications needed!
```

## 🎨 Advanced Workflows

### Workflow 1: Iterative Debugging

```bash
# First run - set one breakpoint
$ python3 idebug.py app.py
(debugger) > break app.py 50
(debugger) > run
[Find the bug is earlier]

# Second run - adjust breakpoints
$ python3 idebug.py app.py
(debugger) > break app.py 30
(debugger) > break app.py 40
(debugger) > run
[Find exact location]

# Third run - narrow down
$ python3 idebug.py app.py
(debugger) > break app.py 35
(debugger) > run
[Fix the bug!]
```

### Workflow 2: Multi-File Debugging

```bash
$ python3 idebug.py main.py

# Set breakpoints across project
(debugger) > break main.py 10
(debugger) > break database.py 50
(debugger) > break api.py 100
(debugger) > break utils.py 25

(debugger) > list
# See all breakpoints

(debugger) > run
# Trace execution through entire codebase
```

### Workflow 3: Production Code Debug

```bash
# Copy production code (don't modify!)
cp /production/app.py ./debug_app.py

# Debug the copy
$ python3 idebug.py debug_app.py
(debugger) > break debug_app.py 100
(debugger) > run

# Find bug, fix in original
# No debugger code left in production!
```

## 🔄 Complete Flow Diagram

```
┌─────────────────────────────────────┐
│  python3 idebug.py myfile.py        │
└──────────────┬──────────────────────┘
               │
               ↓
┌─────────────────────────────────────┐
│   Pre-Execution CLI Opens           │
│   - show file                       │
│   - set breakpoints                 │
│   - list breakpoints                │
│   - run when ready                  │
└──────────────┬──────────────────────┘
               │
               ↓ (run command)
┌─────────────────────────────────────┐
│   Program Executes                  │
│   - Hits breakpoints                │
│   - Interactive at each one         │
│   - n = step, c = continue          │
└──────────────┬──────────────────────┘
               │
               ↓ (completion)
┌─────────────────────────────────────┐
│   Post-Execution CLI Opens          │
│   - Navigate trace                  │
│   - Find variables                  │
│   - Analyze execution               │
└─────────────────────────────────────┘
```

## 🎉 Benefits

### ✅ No Code Modification
- Debug any Python file without changes
- Works with third-party code
- Safe for production code
- Clean separation of concerns

### ✅ Interactive Setup
- See file before setting breakpoints
- Add/remove breakpoints easily
- Preview all settings before running
- Quit without executing if needed

### ✅ Full Control
- Set breakpoints precisely
- Multiple files supported
- Clear and reset anytime
- Complete trace for analysis

## 📚 Quick Reference

```bash
# Launch
python3 idebug.py <file>

# Common workflow
(debugger) > show <file>      # See code
(debugger) > break <file> <line>  # Set breakpoint
(debugger) > list             # Verify
(debugger) > run              # Execute

# At breakpoint
> n    # Step
> c    # Continue
> q    # Quit

# After execution
> view          # See file
> find x        # Search
> quit          # Exit
```

## 🚀 Start Using It!

```bash
# Test it now with provided examples
python3 idebug.py test_loop_breakpoints.py
python3 idebug.py test_variables.py
python3 idebug.py test.py

# Use with your own code
python3 idebug.py myapp.py
```

**No code modifications needed! Set breakpoints interactively!** 🎯
