# Quick Start Guide

## 🏃 Get Running in 2 Minutes

### Step 1: Build (One Time Only)
```bash
python3 setup.py build_ext --inplace
gcc -o traceviewer traceviewer.c -Wall -O2
```

### Step 2: Run the Debugger
```bash
python3 idebug.py test_clean_simple.py
```

### Step 3: Set Breakpoints
```
> break test_clean_simple.py 8
> break test_clean_simple.py 20
> list
> run
```

### Step 4: Debug Interactively
When the breakpoint hits:
```
> c    # Continue to next breakpoint
> n    # Step forward one line
> b    # Step back one line
> h    # Show execution history
```

## 🎯 That's It!

You're now debugging with:
- ✅ Time travel (step backwards)
- ✅ Interactive breakpoints
- ✅ Loop persistence
- ✅ No code modifications needed

## 📝 Example Files Included

All test files are ready to use without modification:
- **test_clean_simple.py** - Simple functions
- **test_clean_loops.py** - For/while loops
- **test.py** - Basic test
- **test_crash.py** - Exception handling
- **test_variables.py** - Variable tracking

## 💡 Pro Tips

1. **View source before running:**
   ```
   > show test_clean_simple.py
   ```

2. **Set multiple breakpoints:**
   ```
   > break file.py 10
   > break file.py 20
   > break file.py 30
   ```

3. **Clear breakpoints:**
   ```
   > clear 1      # Clear breakpoint #1
   > clear all    # Clear all breakpoints
   ```

## 🚀 Debug Your Own Code

```bash
python3 idebug.py your_script.py

> break your_script.py 42
> break your_script.py 100
> run
```

**No imports needed! No code changes! Just debug!**
