# Trace Filtering Guide

## What Gets Traced vs What Gets Skipped

### ✅ TRACED (Your Code)
- Your Python scripts (test.py, my_script.py, etc.)
- Any .py files in your current working directory
- Files you explicitly run with runner.py

### ❌ SKIPPED (System/Library Code)
- Python standard library (`/usr/lib/python3.x/`)
- Site packages (`site-packages/`)
- Frozen modules (`<frozen importlib>`)
- Built-in modules (`<built-in>`)
- Debugger internals (`cdebugger`, `runner.py`)
- System paths (`/usr/lib`, `/usr/local/lib`)

## Why Filter?

Without filtering, your trace would include:
- 1000+ lines from `import sys`
- 500+ lines from `import os`  
- Hundreds of lines from every library import
- Internal Python interpreter operations

**With filtering:** Only YOUR code appears in the trace!

## Example: Before vs After

### Before Filtering (BAD)
```
0|||/usr/lib/python3.9/os.py|||35|||import sys|||
1|||/usr/lib/python3.9/os.py|||36|||import errno|||
2|||/usr/lib/python3.9/posixpath.py|||12|||import os|||
... (1000+ more library lines) ...
1000|||your_script.py|||1|||x = 5|||x=5
1001|||/usr/lib/python3.9/builtins.py|||100|||...|||
```

### After Filtering (GOOD)
```
0|||your_script.py|||1|||x = 5|||x=5
1|||your_script.py|||2|||y = 10|||x=5;y=10
2|||your_script.py|||3|||z = x + y|||x=5;y=10;z=15
```

## What Variables Are Captured?

For EVERY line traced, ALL local variables in that scope are captured:

### Function Scope
```python
def calculate(a, b):
    result = a + b
    return result
```

Trace shows:
```
[Inside calculate]
Variables: a=5;b=10
Variables: a=5;b=10;result=15
```

### Loop Scope
```python
for i in range(3):
    square = i * i
```

Trace shows:
```
[Iteration 1] Variables: i=0;square=0
[Iteration 2] Variables: i=1;square=1
[Iteration 3] Variables: i=2;square=4
```

### Global Scope
```python
x = 5
y = 10
z = x + y
```

Trace shows:
```
Line 1: Variables: x=5
Line 2: Variables: x=5;y=10
Line 3: Variables: x=5;y=10;z=15
```

## Verifying It Works

Run the test:
```bash
# Rebuild with new filtering
gcc -o cdebugger.so debugger.c ...  # Or use make
python3 runner.py test_variables.py

# Check the trace
cat trace.log
```

You should see:
- ✅ Only lines from test_variables.py
- ✅ ALL local variables at each line
- ❌ NO library/system code

## Customizing Filters

To trace additional paths, edit `debugger.c` line ~114:

```c
// Add custom filters
if (strstr(filename, "mylib") != NULL) {
    return 0;  // Skip mylib
}

// Or only trace specific files
if (strstr(filename, "test.py") == NULL) {
    return 0;  // Only trace test.py
}
```

## Troubleshooting

### "Too much output in trace.log"
- Some system files still getting through
- Add more filters in debugger.c
- Check the filename patterns

### "Variables not showing"
- Make sure you're in a function scope
- Global scope variables work too
- Check that locals aren't empty

### "Missing some code"
- You may have filtered too aggressively
- Comment out filters one by one
- Check the filename in trace.log

## Quick Test

```bash
# 1. Rebuild
make clean && make all

# 2. Run test
python3 runner.py test_variables.py

# 3. Check trace - should only show test_variables.py lines
grep -v "test_variables.py" trace.log | head
# Should show nothing or just header
```

## Summary

**Current Filters:**
- `/usr/lib/*` - System libraries
- `/usr/local/lib/*` - Local libraries  
- `site-packages/*` - Installed packages
- `python3.*` - Python internals
- `<frozen>` - Frozen modules
- `importlib` - Import system
- `cdebugger` - Debugger itself
- `runner.py` - Runner script

**Result:**
- Clean traces with only YOUR code
- ALL variables captured at every line
- Easy to navigate and understand
- Fast performance (less I/O)

Rebuild with `make clean && make all` to apply the new filters!
