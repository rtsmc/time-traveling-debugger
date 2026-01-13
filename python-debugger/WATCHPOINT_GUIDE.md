# Watchpoint Feature Guide

## Overview

Watchpoints allow you to pause execution whenever a variable is accessed (read or written). This is incredibly useful for tracking down bugs related to unexpected variable changes or understanding data flow through your program.

## Watchpoint Types

### 1. Normal Watchpoint (`w <variable>`)
Triggers on **both** reads and writes to the variable.

```
> w counter
✓ Watchpoint set on 'counter' (type: read/write)
```

### 2. Read Watchpoint (`rw <variable>`)
Triggers only when the variable is **read** (used in expressions, print statements, etc.).

```
> rw result
✓ Watchpoint set on 'result' (type: read)
```

### 3. Write Watchpoint (`ww <variable>`)
Triggers only when the variable is **written** (assigned a new value).

```
> ww x
✓ Watchpoint set on 'x' (type: write)
```

## Commands

### Setting Watchpoints

```
w <variable>    - Set normal watchpoint (read/write)
rw <variable>   - Set read watchpoint
ww <variable>   - Set write watchpoint
```

**Examples:**
```
> w counter         # Track both reads and writes to 'counter'
> rw result         # Track only reads of 'result'
> ww x              # Track only writes to 'x'
```

### Managing Watchpoints

```
listw           - List all watchpoints
clearw          - Clear all watchpoints
clearw <num>    - Clear specific watchpoint by number
```

**Examples:**
```
> listw
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Watchpoints:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  1. counter (read/write)
  2. result (read)
  3. x (write)
Total: 3 watchpoint(s)

> clearw 2       # Clear the 'result' watchpoint
> clearw         # Clear all watchpoints
```

### Navigating with Watchpoints

Once watchpoints are set, use the continue commands to navigate:

```
c    - Continue to next breakpoint or watchpoint
rc   - Reverse continue to previous breakpoint or watchpoint
```

## How Watchpoints Work

### Read Detection
A variable is considered "read" when:
- It appears in the code being executed
- It exists in the current variable state
- It's used in an expression, function call, or print statement

**Example:**
```python
x = 10      # No read (only write)
y = x + 5   # READ of x (used in expression)
print(x)    # READ of x (used in print)
```

### Write Detection
A variable is considered "written" when:
- It's assigned a new value
- Its value changes from the previous execution step
- It's created (didn't exist before)

**Example:**
```python
x = 10      # WRITE (new variable)
x = 20      # WRITE (value changed)
x = 20      # No write (value unchanged)
```

## Example Session

### Step 1: Run the debugger
```bash
$ python3 idebug.py test_watchpoint.py
> run
```

### Step 2: Open the trace viewer
After execution completes, the trace viewer opens automatically:
```
Loading trace file: trace.log
✓ Loaded 47 execution steps
```

### Step 3: Set watchpoints
```
[1/47] > w counter
✓ Watchpoint set on 'counter' (type: read/write)

[1/47] > ww result
✓ Watchpoint set on 'result' (type: write)

[1/47] > listw
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Watchpoints:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  1. counter (read/write)
  2. result (write)
Total: 2 watchpoint(s)
```

### Step 4: Continue to watchpoints
```
[1/47] > c

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
👁 WATCHPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Variable 'result' was write
[8/47] test_watchpoint.py:7
    result = x + y
Variables: x=10; y=20; result=30

[8/47] > c

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
👁 WATCHPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Variable 'counter' was write
[12/47] test_watchpoint.py:12
    counter = 0
Variables: counter=0
```

### Step 5: Reverse continue
```
[12/47] > rc

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⟲ WATCHPOINT HIT (REVERSE)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Variable 'result' was write
[8/47] test_watchpoint.py:7
    result = x + y
```

## Integration with Breakpoints

Watchpoints work seamlessly with breakpoints:
- Both watchpoints and breakpoints are checked during `c` and `rc` commands
- Watchpoints are checked first (higher priority)
- You can have both breakpoints and watchpoints active simultaneously

**Example:**
```
[1/47] > b test_watchpoint.py 15    # Set breakpoint
[1/47] > w counter                   # Set watchpoint
[1/47] > c                           # Continue to first hit (breakpoint OR watchpoint)
```

## Use Cases

### 1. Tracking Variable Changes
Find when and where a variable's value changes:
```
> ww user_id
> c   # Jump to next write
```

### 2. Finding Variable Usage
See everywhere a variable is used:
```
> rw config
> c   # Jump to next read
```

### 3. Debugging Loops
Track loop counters and accumulator variables:
```
> w i
> w total
> c   # See each iteration where these change
```

### 4. Understanding Data Flow
Follow how data moves through your program:
```
> w input_data
> c   # See reads and writes
```

## Tips and Tricks

1. **Start Specific**: Use write watchpoints (`ww`) when you only care about assignments
2. **Combine with Breakpoints**: Set breakpoints at key locations, watchpoints on key variables
3. **Use listw Often**: Keep track of active watchpoints
4. **Clear When Done**: Use `clearw` to remove watchpoints you no longer need
5. **Watch Simple Variables**: Watchpoints work best on simple variables (not complex objects)

## Limitations

- Maximum 50 watchpoints per session
- Watchpoints match variable names exactly (case-sensitive)
- Works on local and global variables visible in the trace
- Complex object attributes may not trigger as expected

## Quick Reference

```
Command          | Description
-----------------|---------------------------------
w <var>          | Normal watchpoint (read/write)
rw <var>         | Read watchpoint
ww <var>         | Write watchpoint
listw            | List all watchpoints
clearw           | Clear all watchpoints
clearw <num>     | Clear specific watchpoint
c                | Continue to next breakpoint/watchpoint
rc               | Reverse to previous breakpoint/watchpoint
```

## Example Workflow

```bash
# 1. Run your program
python3 idebug.py my_script.py
> run

# 2. In trace viewer, set watchpoints
> w counter
> ww result
> rw config

# 3. Navigate using continue
> c     # Jump to first watchpoint/breakpoint
> c     # Next
> rc    # Go back

# 4. Analyze what happened
> show          # View full code
> summary       # See trace statistics

# 5. Clean up
> clearw        # Remove watchpoints if testing something else
```

Happy debugging! 🐛✨
