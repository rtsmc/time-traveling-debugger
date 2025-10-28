# Step Back & Loop Breakpoints - New Features Guide

## 🎯 What's New

### Feature 1: Step Back at Breakpoints ⏪
When paused at a breakpoint, you can now **step backward** through execution history!

### Feature 2: Loop Breakpoints Persist 🔁
Breakpoints in loops now **keep breaking on every iteration** (not just once)!

---

## 🚀 Step Back Feature

### How It Works

When you hit a breakpoint, the debugger maintains a trace history in memory. You can step backward through this history to see what happened before.

### Commands

| Command | Description |
|---------|-------------|
| `b` | Step **back** to previous execution |
| `n` | Step **next** (forward) |
| `h` | Show trace **history** |
| `c` | Continue execution |
| `q` | Quit |

### Example Session

```bash
$ python3 idebug.py test_loop_breakpoints.py

(debugger) > break test_loop_breakpoints.py 24
(debugger) > run

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_loop_breakpoints.py Line: 24
Hit count: 3
Code:         total += i

Commands:
  c - Continue to next breakpoint
  n - Step to next line
  b - Step back to previous line
  h - Show trace history
  q - Quit execution

> b    # Step back!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⟲ TRACE HISTORY [45/46]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Execution: #44
File: test_loop_breakpoints.py Line: 23
Code:     for i in range(5):
Variables: i=2; total=3; simple_loop=<function>
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> b    # Step back again!

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⟲ TRACE HISTORY [44/46]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Execution: #43
File: test_loop_breakpoints.py Line: 25
Code:         print(f"  i={i}, total={total}")
Variables: i=1; total=1; simple_loop=<function>
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

> h    # Show history

Trace History (46 entries):
  [37] #36 test_loop_breakpoints.py:24
  [38] #37 test_loop_breakpoints.py:25
  [39] #38 test_loop_breakpoints.py:23
  [40] #39 test_loop_breakpoints.py:24
  [41] #40 test_loop_breakpoints.py:25
  [42] #41 test_loop_breakpoints.py:23
  [43] #42 test_loop_breakpoints.py:24
> [44] #43 test_loop_breakpoints.py:25    ← You are here
  [45] #44 test_loop_breakpoints.py:23
  [46] #45 test_loop_breakpoints.py:24

> n    # Step forward (resumes execution)
```

---

## 🔁 Loop Breakpoints Persist

### How It Works

Previously, a breakpoint in a loop would only hit once, then you'd have to press 'c' to continue and it wouldn't break again.

**Now:** Breakpoints persist! Every iteration through the loop hits the breakpoint.

### Example: Loop with 5 Iterations

```python
for i in range(5):
    total += i    # Breakpoint on this line
    print(total)
```

**Old behavior:**
```
⚫ BREAKPOINT HIT (i=0)
> c    # Continue
[Loop completes without breaking again]
```

**New behavior:**
```
⚫ BREAKPOINT HIT (i=0)
Hit count: 1
> c

⚫ BREAKPOINT HIT (i=1)
Hit count: 2
> c

⚫ BREAKPOINT HIT (i=2)
Hit count: 3
> n    # Can step or continue

⚫ BREAKPOINT HIT (i=3)
Hit count: 4
> c

⚫ BREAKPOINT HIT (i=4)
Hit count: 5
> c

[Loop completes after all iterations]
```

### Benefits

1. **Inspect each iteration** - See how variables change
2. **Selective stepping** - Continue through some iterations, step through others
3. **Find iteration-specific bugs** - See exactly which iteration fails
4. **Hit count tracking** - Know which iteration you're on

---

## 🎮 Complete Workflow Examples

### Example 1: Debug Loop with Step Back

```bash
$ python3 idebug.py myloop.py

(debugger) > break myloop.py 15
(debugger) > run

# Loop iteration 1
⚫ BREAKPOINT HIT
Hit count: 1
> n    # Step forward

# Loop iteration 2
⚫ BREAKPOINT HIT
Hit count: 2
> n

# Loop iteration 3
⚫ BREAKPOINT HIT
Hit count: 3
> b    # Wait, step back to see iteration 2 again

⟲ TRACE HISTORY [23/25]
# Shows iteration 2 state

> b    # Step back further to iteration 1

⟲ TRACE HISTORY [22/25]
# Shows iteration 1 state

> n    # Resume forward execution
```

### Example 2: Find Which Iteration Has Bug

```bash
$ python3 idebug.py buggy_loop.py

(debugger) > break buggy_loop.py 42
(debugger) > run

⚫ BREAKPOINT HIT - Hit count: 1
> c    # Iteration 1 looks fine

⚫ BREAKPOINT HIT - Hit count: 2
> c    # Iteration 2 looks fine

⚫ BREAKPOINT HIT - Hit count: 3
> n    # Wait, this one looks suspicious
> n
> n    # Step through to confirm

⚫ BREAKPOINT HIT - Hit count: 4
> b    # Go back to see what happened
> b
> h    # Show history to trace the bug
```

### Example 3: Nested Loops

```python
for i in range(3):
    for j in range(3):
        result = i * j    # Breakpoint here
        print(result)
```

```bash
$ python3 idebug.py nested.py

(debugger) > break nested.py 10
(debugger) > run

# Hits 9 times total (3×3)

⚫ BREAKPOINT HIT (i=0, j=0)
Hit count: 1
> c

⚫ BREAKPOINT HIT (i=0, j=1)
Hit count: 2
> c

⚫ BREAKPOINT HIT (i=0, j=2)
Hit count: 3
> c

⚫ BREAKPOINT HIT (i=1, j=0)
Hit count: 4
> b    # Step back to previous iteration
> n    # Step forward to current

⚫ BREAKPOINT HIT (i=1, j=1)
Hit count: 5
> n

# ... continues for all 9 iterations
```

---

## 📊 Use Cases

### Use Case 1: "What Led to This?"

```bash
# You hit a breakpoint at a bad state
⚫ BREAKPOINT HIT
Variables: x=0; y=42; result=None

# Step back to see what happened before
> b
> b
> b
# Now you see where x became 0
```

### Use Case 2: "Watch Variable Change Through Loop"

```bash
# Set breakpoint in loop
> c    # i=0, total=0
> c    # i=1, total=1
> c    # i=2, total=3
> c    # i=3, total=6
> b    # Go back to see total=3
> b    # Go back to see total=1
> n    # Move forward through history
```

### Use Case 3: "Skip Most Iterations, Debug One"

```bash
# Loop with 100 iterations
> c    # Skip first 50
> c    # Skip iteration 51
> c    # Skip iteration 52
# ... 
# At iteration 75 (the suspicious one)
> n    # Step through this one carefully
> n
> b    # Oh wait, go back
> n    # Step forward again
> c    # Continue through rest
```

---

## 🎯 Command Reference

### At Breakpoint

| Command | Description | Example Use |
|---------|-------------|-------------|
| `c` | **Continue** - Run to next breakpoint | Skip iterations |
| `n` | **Next** - Step forward one line | Careful inspection |
| `b` | **Back** - Step backward in history | Review what happened |
| `h` | **History** - Show trace history list | See recent execution |
| `q` | **Quit** - Exit immediately | Stop debugging |

### History Navigation

```bash
# Current position in trace
> h

Trace History (50 entries):
  [45] #44 file.py:10
  [46] #45 file.py:11
  [47] #46 file.py:12
> [48] #47 file.py:13    ← You are here
  [49] #48 file.py:14
  [50] #49 file.py:15

# Step back
> b
# Now at [47]

# Step back again
> b
# Now at [46]

# Step forward (resumes execution, not just history)
> n
```

---

## 💡 Pro Tips

### Tip 1: Use History to Understand Flow

```bash
# When you hit a breakpoint
> h    # See recent execution

# Shows you:
# - What lines executed
# - What order they executed in
# - Which iteration you're in
```

### Tip 2: Step Back to Review Variables

```bash
# At breakpoint with unexpected value
Variables: result=42

> b    # Step back
Variables: result=21

> b    # Step back more
Variables: result=7

# Aha! Found where it changed!
```

### Tip 3: Combine Continue and Step Back

```bash
# Fast-forward to problem area
> c
> c
> c

# Found the issue
> b    # Review what led to it
> b
> b
```

### Tip 4: History Shows Execution Order

```bash
> h

  [40] #39 file.py:10    ← Function entry
  [41] #40 file.py:15    ← Inside function
  [42] #41 file.py:16    ← Still inside
  [43] #42 file.py:20    ← Function return
  [44] #43 file.py:30    ← Back to caller

# See the call stack through history!
```

---

## 🔄 Comparison: Before vs After

### Before (Old Behavior)

```
# Breakpoint in loop
⚫ BREAKPOINT HIT (i=0)
> c
[Loop completes, no more breaks]

# No step back
> b
[Command doesn't exist]
```

### After (New Behavior)

```
# Breakpoint in loop - hits every iteration!
⚫ BREAKPOINT HIT (i=0) - Hit count: 1
> c

⚫ BREAKPOINT HIT (i=1) - Hit count: 2
> c

⚫ BREAKPOINT HIT (i=2) - Hit count: 3
> b    # Step back!

⟲ TRACE HISTORY [23/25]
# Shows previous state

> n    # Continue forward
```

---

## 📝 Technical Details

### Trace History

- Stores up to 1000 recent execution steps
- Includes:
  - Filename and line number
  - Source code
  - Variable values
  - Execution number
- Memory-efficient (only recent history)

### Loop Persistence

- Breakpoints are checked on EVERY execution
- Hit count increments each time
- No limit on iterations
- Works with:
  - for loops
  - while loops
  - Nested loops
  - Recursive functions

---

## 🚀 Try It Now!

```bash
# 1. Rebuild with new features
cp debugger_with_breakpoints.c debugger.c
rm -rf build/ cdebugger*.so
python3 setup.py build_ext --inplace

# 2. Test loop breakpoints
python3 idebug.py test_loop_breakpoints.py
(debugger) > break test_loop_breakpoints.py 24
(debugger) > run

# When breakpoint hits:
> c    # Continue - hits again!
> c    # Hits again!
> n    # Step forward
> b    # Step back!
> h    # Show history
```

---

## 🎉 Summary

### ✅ Step Back Feature
- Press `b` to step backward
- View previous execution states
- See what led to current state
- Navigate trace history

### ✅ Loop Breakpoints
- Break on EVERY iteration
- Track iteration count
- Selective stepping
- Complete loop inspection

**Commands:**
```
c - Continue (hits breakpoint again in loop)
n - Step forward
b - Step back
h - Show history
q - Quit
```

**These features make loop debugging much more powerful! 🔁⏪**
