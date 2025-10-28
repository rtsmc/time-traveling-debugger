# 🎯 Loop Breakpoint Tests - Quick Reference

## ✅ What's Included

### Test Files

1. **test_loop_breakpoints.py** - Basic loop scenarios
   - Simple for loop (5 iterations)
   - Nested loops (3x3)
   - While loop
   - Loop with break

2. **test_advanced_loops.py** - Advanced scenarios
   - Countdown loops
   - Continue statements (skips evens)
   - For-else constructs
   - List comprehensions
   - Enumerate/zip loops
   - Infinite loop with break
   - Nested break/continue

### Documentation

- **LOOP_TESTING_GUIDE.md** - Complete testing guide
- **BREAKPOINT_GUIDE.md** - Full breakpoint system docs

## 🚀 Run Tests

```bash
# Basic test (recommended start here)
python3 test_loop_breakpoints.py

# Advanced test
python3 test_advanced_loops.py
```

## 🎮 What to Try

### Try #1: See Each Loop Iteration
```
1. Run: python3 test_loop_breakpoints.py
2. At breakpoint line 24, press 'n'
3. Keep pressing 'n' to see i=0, i=1, i=2, i=3, i=4
4. Watch variables change each time
```

### Try #2: Skip Through Loop
```
1. Run: python3 test_loop_breakpoints.py
2. At breakpoint line 24, press 'c'
3. Loop completes automatically
4. Stops at next breakpoint
```

### Try #3: See Nested Loops
```
1. Run: python3 test_loop_breakpoints.py
2. At breakpoint line 32, press 'n'
3. Step through inner loop
4. Hit breakpoint again for next outer iteration
```

### Try #4: Continue Statement Behavior
```
1. Run: python3 test_advanced_loops.py
2. At breakpoint line 40, press 'c'
3. Notice it only hits on ODD numbers
4. Skips even numbers due to continue
```

## 📊 Expected Breakpoint Hits

### test_loop_breakpoints.py
```
Line 22: 1 hit   (before loop)
Line 24: 5 hits  (inside loop - each iteration)
Line 32: 3 hits  (outer loop)
Line 42: 1 hit   (after loops)
```

### test_advanced_loops.py
```
Line 25: 1 hit   (while loop entry)
Line 27: 5 hits  (while loop body)
Line 40: 5 hits  (only odd numbers 1,3,5,7,9)
Line 55: 1 hit   (after list comprehension)
```

## 🎯 Key Learning Points

1. **Breakpoints in loops hit multiple times**
   - Hit count shows iteration number
   
2. **Step vs Continue**
   - 'n' = step one iteration
   - 'c' = complete loop, move to next breakpoint

3. **Continue statements affect hits**
   - Breakpoint skipped when continue executes
   - Fewer hits than loop iterations

4. **Nested loops multiply hits**
   - Inner breakpoint hits = outer × inner iterations
   - Example: 3x3 nested = 9 hits on inner breakpoint

5. **Trace grows as you step**
   - Each 'n' adds one line to trace
   - Each 'c' adds all remaining lines to next breakpoint

## 🐛 Common Test Scenarios

### Scenario: Find Which Iteration Has Bug
```bash
# Set breakpoint in loop
# Step through each iteration with 'n'
# Check variables to find wrong value
# Note iteration number from hit count
```

### Scenario: Skip to Problem Iteration
```bash
# If bug is in iteration 3
# Press 'c' to continue
# Will hit breakpoint 3 times
# On 3rd hit, start stepping with 'n'
```

### Scenario: Debug Nested Loop
```bash
# Set breakpoint in outer loop
# Press 'n' to see outer iterations
# Set breakpoint in inner loop (next run)
# Press 'n' to see all inner iterations
```

## ✨ Quick Commands Reference

**During Execution (at breakpoint):**
- `n` - Step to next line
- `c` - Continue to next breakpoint  
- `q` - Quit immediately

**In CLI (after execution):**
- `n` - Next step in trace
- `back` - Previous step
- `find i` - Search for variable i
- `break 24` - Jump to line 24
- `view` - See full file
- `summary` - Show stats

## 📥 Download

**Full Package:** [python-debugger.zip (60 KB)](computer:///mnt/user-data/outputs/python-debugger.zip)

Includes:
- ✅ Breakpoint-enabled debugger
- ✅ CLI that always launches
- ✅ 5 test files (including 2 loop tests)
- ✅ Complete documentation

## 🎉 Start Testing!

```bash
# 1. Setup
cp debugger_with_breakpoints.c debugger.c
rm -rf build/ cdebugger*.so
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2

# 2. Run basic loop test
python3 test_loop_breakpoints.py

# 3. When breakpoint hits:
#    - Press 'n' to step
#    - Press 'c' to continue
#    - Press 'q' to quit

# 4. After execution:
#    - CLI launches automatically
#    - Navigate the trace
#    - View variables at each step
```

**Happy debugging loops! 🔁🐛**
