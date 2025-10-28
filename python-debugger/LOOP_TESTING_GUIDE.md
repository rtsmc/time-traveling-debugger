# Loop Breakpoint Testing Guide

## 🎯 Test Files Included

### 1. test_loop_breakpoints.py
Basic loop tests covering common scenarios:
- Simple for loop
- Nested loops
- While loop
- Loop with break statement

### 2. test_advanced_loops.py
Advanced loop scenarios:
- Countdown loops
- Continue statements
- For-else constructs
- List comprehensions
- Enumerate and zip
- Infinite loops with break
- Nested break/continue

## 🚀 Quick Start

```bash
# 1. Setup (if not done already)
cp debugger_with_breakpoints.c debugger.c
rm -rf build/ cdebugger*.so
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2

# 2. Run basic loop test
python3 test_loop_breakpoints.py

# 3. Run advanced loop test
python3 test_advanced_loops.py
```

## 📋 What to Expect

### Test 1: Simple Loop (Line 24 Breakpoint)

```python
for i in range(5):
    # BREAKPOINT HERE - will hit 5 times!
    total += i
```

**Expected Behavior:**
```
⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 1

> n    # Step to see i=0

⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 2

> n    # Step to see i=1

⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 3

> c    # Continue (will hit 2 more times)

⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 4

⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 5

[Loop complete, continues to next breakpoint]
```

### Test 2: Nested Loops (Line 32 Breakpoint)

```python
for i in range(3):
    # BREAKPOINT HERE - will hit 3 times (outer loop)
    for j in range(3):
        product = i * j
```

**Expected Behavior:**
```
⚫ BREAKPOINT HIT
Hit count: 1    # i=0

> n    # Step through inner loop

⚫ BREAKPOINT HIT
Hit count: 2    # i=1

> n    # Step through again

⚫ BREAKPOINT HIT
Hit count: 3    # i=2

> c    # Continue
```

### Test 3: Continue Statement (Line 40 Breakpoint)

```python
for i in range(10):
    if i % 2 == 0:
        continue  # Skip evens
    # BREAKPOINT HERE - only hits on ODD numbers!
    print(f"Odd: {i}")
```

**Expected Behavior:**
```
⚫ BREAKPOINT HIT (i=1)
Hit count: 1

> c

⚫ BREAKPOINT HIT (i=3)
Hit count: 2

> c

⚫ BREAKPOINT HIT (i=5)
Hit count: 3

[Only hits 5 times total, not 10, because of continue]
```

## 🎮 Interactive Test Scenarios

### Scenario 1: Step Through Each Iteration

```
Goal: See every iteration of a loop

1. Run: python3 test_loop_breakpoints.py
2. At line 24 breakpoint, press 'n'
3. Keep pressing 'n' for each iteration
4. Watch variables change: i=0, i=1, i=2, etc.
```

### Scenario 2: Skip to Next Breakpoint

```
Goal: Let loop complete, move to next section

1. Run: python3 test_loop_breakpoints.py
2. At line 24 breakpoint, press 'c'
3. Loop completes automatically
4. Stops at next breakpoint (line 32)
```

### Scenario 3: Quit Mid-Loop

```
Goal: Exit before loop completes

1. Run: python3 test_loop_breakpoints.py
2. At line 24 breakpoint (iteration 1), press 'q'
3. Program exits
4. Partial trace saved
5. CLI launches, showing only executed lines
```

### Scenario 4: Nested Loop Navigation

```
Goal: Understand nested loop execution

1. Run: python3 test_loop_breakpoints.py
2. At line 32 (outer loop), press 'n'
3. Step through inner loop iterations
4. At line 32 again (next outer iteration), press 'n'
5. See pattern of execution
```

## 🔍 Debugging Tips

### Finding the Current Iteration

When breakpoint hits in a loop:
1. Look at the trace file
2. Check variable values
3. Note the hit count

Example:
```
⚫ BREAKPOINT HIT
File: test_loop_breakpoints.py Line: 24
Hit count: 3

# This is the 3rd iteration (i=2)
```

### Understanding Hit Counts

- **Hit count = 1**: First time breakpoint hit
- **Hit count > 1**: Repeated hit (loop iteration)
- **Increasing count**: Loop is progressing
- **No more hits**: Loop completed or skipped

### Common Patterns

**Simple Loop:**
```
Hit count: 1, 2, 3, 4, 5
[5 iterations = 5 hits]
```

**Loop with Continue:**
```
Hit count: 1, 2, 3
[Only hits when condition met]
```

**Nested Loop (outer breakpoint):**
```
Hit count: 1, 2, 3
[Outer loop iterations]
```

**Nested Loop (inner breakpoint):**
```
Hit count: 1, 2, 3, 4, 5, 6, 7, 8, 9
[3x3 = 9 hits total]
```

## 📊 Expected Results

### test_loop_breakpoints.py

| Breakpoint | Line | Expected Hits | Notes |
|------------|------|---------------|-------|
| BP1 | 22 | 1 | Before loop starts |
| BP2 | 24 | 5 | Inside loop (5 iterations) |
| BP3 | 32 | 3 | Outer loop (3 iterations) |
| BP4 | 42 | 1 | After loops |

### test_advanced_loops.py

| Breakpoint | Line | Expected Hits | Notes |
|------------|------|---------------|-------|
| BP1 | 25 | 1 | While loop entry |
| BP2 | 27 | 5 | While loop body (5 iterations) |
| BP3 | 40 | 5 | Only odd numbers (0-9, skips evens) |
| BP4 | 55 | 1 | After list comprehension |

## 🧪 Verification Steps

### Step 1: Count Breakpoint Hits

```bash
# Run test
python3 test_loop_breakpoints.py

# At each breakpoint, note the hit count
# Verify it matches expectations
```

### Step 2: Check Variable Values

```bash
# When stepping through loop
# Check variable values in trace:
./traceviewer trace_loops.log

# Search for loop variable
> find i

# See all values: i=0, i=1, i=2, i=3, i=4
```

### Step 3: Verify Trace Completeness

```bash
# After running with 'c' (continue)
./traceviewer trace_loops.log

# Check total execution count
> summary

# Should show all loop iterations
```

## 🎯 Test Cases to Try

### Case 1: Step Through Entire Loop
```
Command sequence: n, n, n, n, n
Result: See each iteration individually
```

### Case 2: Continue Through Loop
```
Command sequence: c
Result: All iterations execute, stops at next breakpoint
```

### Case 3: Mixed Mode
```
Command sequence: n, n, c
Result: Step 2 iterations, then continue rest
```

### Case 4: Early Exit
```
Command sequence: n, q
Result: Partial execution, early termination
```

## 🐛 Troubleshooting

### Breakpoint Not Hitting Expected Times

**Problem:** Breakpoint hits fewer times than expected

**Check:**
1. Is there a `continue` statement?
2. Is there a `break` statement?
3. Is there a conditional that skips the line?

**Example:**
```python
for i in range(10):
    if i > 5:
        break  # Loop exits early!
    # Breakpoint here only hits 6 times, not 10
```

### Breakpoint Hits Too Many Times

**Problem:** More hits than expected

**Check:**
1. Nested loops multiplying iterations
2. Function called multiple times
3. Recursive calls

**Example:**
```python
for i in range(3):
    for j in range(3):
        # Breakpoint here hits 9 times (3*3)
```

### Can't Step Forward

**Problem:** 'n' command doesn't work

**Solution:**
1. Make sure you're at a breakpoint
2. Type lowercase 'n', not 'N'
3. Check terminal input is working

### Variables Not Updating

**Problem:** Variable values seem stuck

**Solution:**
1. Check you're actually stepping
2. View trace to confirm progress
3. Search for variable: `find varname`

## 📚 Learning Exercises

### Exercise 1: Count Loop Iterations
Set breakpoint inside loop, step through, count hits manually

### Exercise 2: Find Bug in Loop
Set breakpoint, step through until wrong value appears

### Exercise 3: Nested Loop Understanding
Set breakpoints in both outer and inner loop, observe pattern

### Exercise 4: Skip vs Step
Use 'c' for some iterations, 'n' for others, compare results

## 🎉 Summary

**Test Files:**
- ✅ test_loop_breakpoints.py - Basic scenarios
- ✅ test_advanced_loops.py - Complex scenarios

**Key Concepts:**
- Breakpoints hit multiple times in loops
- Hit count shows iteration number
- Step ('n') vs Continue ('c') behavior
- Continue statement affects hit count
- Nested loops multiply hits

**Commands:**
- `n` - Step to next line (one iteration)
- `c` - Continue (complete loop)
- `q` - Quit (stop execution)

Now test the breakpoint system with loops! 🚀
