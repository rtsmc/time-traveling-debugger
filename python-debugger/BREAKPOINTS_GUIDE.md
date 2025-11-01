# Breakpoints in Imported Files - User Guide

## 🎯 Overview

The debugger now **fully supports breakpoints in imported modules**! You can set breakpoints in any Python file that your main script imports.

## ✨ What's Fixed

Previously, breakpoints in imported files wouldn't trigger because:
- Python stores filenames as absolute paths internally
- Users set breakpoints with relative paths
- String comparison failed to match them

**Now fixed with intelligent path matching that handles:**
- ✅ Relative paths (e.g., `helper.py`)
- ✅ Absolute paths (e.g., `/home/user/project/helper.py`)
- ✅ Basename matching as fallback
- ✅ Automatic path normalization

## 📖 How to Use

### Example Project Structure
```
project/
├── main.py          # Main script
└── helper.py        # Imported module
```

### Setting Breakpoints

**Option 1: Relative paths (recommended)**
```bash
python3 idebug.py main.py

> break main.py 15        # Breakpoint in main file
> break helper.py 10      # Breakpoint in imported file
> list
> run
```

**Option 2: Absolute paths**
```bash
> break /home/user/project/helper.py 10
> run
```

**Both work!** The debugger automatically normalizes paths.

## 🧪 Test It Yourself

We've included test files to demonstrate this feature:

### 1. Run the test files:
```bash
python3 idebug.py test_import_main.py
```

### 2. Set breakpoints in both files:
```
> break test_import_main.py 13    # Main file
> break test_helper.py 9          # Imported module  
> break test_helper.py 21         # Another imported function
> list
> run
```

### 3. Watch it work!
The debugger will stop at breakpoints in both the main file AND the imported helper module.

## 💡 Tips

### Viewing Imported Files
```
> show test_helper.py        # View imported file with line numbers
```

### Multiple Breakpoints in Loops
If you set a breakpoint in a loop inside an imported function:
```
> break helper.py 9          # Inside a for loop
> run
# Will stop on EACH iteration
> c                          # Continue to next iteration
> c                          # Next iteration
```

### Working with Nested Imports
If `main.py` imports `helper.py`, and `helper.py` imports `utils.py`:
```
> break main.py 10
> break helper.py 20
> break utils.py 30
> run
```
**All breakpoints work!**

## 🔍 Technical Details

### Path Matching Algorithm
The debugger uses a three-tier matching strategy:

1. **Exact Match**: Direct string comparison
   - `helper.py` vs `helper.py` ✓

2. **Absolute Path Contains**: Check if paths are subpaths
   - `/home/user/project/helper.py` contains `helper.py` ✓

3. **Basename Match**: Compare just the filename
   - `helper.py` == basename of `/home/user/project/helper.py` ✓

### Path Normalization
When you set a breakpoint:
```
> break helper.py 10
```

The debugger converts it to an absolute path:
```
✓ Breakpoint set at /home/user/project/helper.py:10
```

This ensures it matches Python's internal file representation.

## ⚠️ Troubleshooting

### Breakpoint Not Triggering?

**Check the file path:**
```
> show helper.py              # Verify the file exists
```

**Verify Python can find it:**
```python
# In your main script
import helper  # If this works, breakpoints will work
```

**Use absolute path:**
```
> break /full/path/to/helper.py 10
```

### File Not Found Warning

If you see:
```
⚠ Warning: File 'helper.py' not found
Set breakpoint anyway? (y/n):
```

This usually means:
- The file is in a different directory
- You need to use a relative path from your working directory
- Try: `python3 idebug.py main.py` from the project root

### Still Not Working?

Make sure you're running the debugger from the same directory as your main script, or use absolute paths for everything.

## 🚀 Quick Reference

```bash
# Start debugger
python3 idebug.py main.py

# Set breakpoints in main and imported files
> break main.py 15
> break helper.py 10
> break utils.py 20

# List all breakpoints
> list

# Run with breakpoints
> run

# At breakpoint:
# c - Continue
# n - Step forward
# b - Step back
# h - View history
# q - Quit
```

## ✅ Verification

To verify the fix is working:

1. Create two files (`test_import_main.py` and `test_helper.py`) - already included!
2. Run: `python3 idebug.py test_import_main.py`
3. Set: `break test_helper.py 9`
4. Run: `run`
5. See: Breakpoint triggers in imported file! 🎉

## 📝 Summary

**Before:** Breakpoints only worked in the main file  
**After:** Breakpoints work in ALL imported modules!

No code modification needed - just set breakpoints and run!
