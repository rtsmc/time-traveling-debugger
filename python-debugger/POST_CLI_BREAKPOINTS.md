# Post-Execution Breakpoint Navigation - New Feature Guide

## 🎯 Overview

The trace viewer now supports **breakpoint navigation** in post-execution analysis! You can set breakpoints while exploring a trace and jump between them forwards or backwards.

## ✨ New Commands

### Set Breakpoints
```
b <file> <line>    - Set a breakpoint at specific file and line
b                  - List all breakpoints (same as 'list')
list               - List all breakpoints
```

### Navigate Between Breakpoints
```
c                  - Continue to next breakpoint (forward)
rc                 - Reverse continue to previous breakpoint (backward)
```

### Legacy Commands
```
jump <line>        - Jump to first occurrence of source line
```

## 📖 Usage Examples

### Example 1: Basic Breakpoint Navigation

After running your program and entering the post-execution debugger:

```bash
[1/150] > b test.py 25
✓ Breakpoint set at test.py:25

[1/150] > b test_helper.py 10
✓ Breakpoint set at test_helper.py:10

[1/150] > list
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Breakpoints:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  1. test.py:25
  2. test_helper.py:10
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Total: 2 breakpoint(s)

[1/150] > c
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #25] test_helper.py:10
...

[42/150] > c
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #78] test.py:25
...

[78/150] > rc
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⟲ BREAKPOINT HIT (REVERSE)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #42] test_helper.py:10
...
```

### Example 2: Exploring Loops

Set a breakpoint inside a loop and jump through iterations:

```bash
# Set breakpoint in loop
[1/150] > b test.py 15
✓ Breakpoint set at test.py:15

# Jump to first hit
[1/150] > c
⚫ BREAKPOINT HIT
[Execution #12] test.py:15
Variables: i=0; total=0

# Continue to next iteration
[12/150] > c
⚫ BREAKPOINT HIT
[Execution #25] test.py:15
Variables: i=1; total=1

# Continue to next iteration
[25/150] > c
⚫ BREAKPOINT HIT
[Execution #38] test.py:15
Variables: i=2; total=3

# Go back to previous iteration
[38/150] > rc
⟲ BREAKPOINT HIT (REVERSE)
[Execution #25] test.py:15
Variables: i=1; total=1
```

### Example 3: Cross-File Debugging

Navigate between files in your trace:

```bash
[1/150] > b main.py 20
✓ Breakpoint set at main.py:20

[1/150] > b helper.py 10
✓ Breakpoint set at helper.py:10

[1/150] > b utils.py 5
✓ Breakpoint set at utils.py:5

[1/150] > c
⚫ BREAKPOINT HIT
[Execution #15] utils.py:5

[15/150] > c
⚫ BREAKPOINT HIT
[Execution #23] helper.py:10

[23/150] > c
⚫ BREAKPOINT HIT
[Execution #45] main.py:20
```

## 🎨 Visual Indicators

### Forward Continue
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⚫ BREAKPOINT HIT
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Reverse Continue
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⟲ BREAKPOINT HIT (REVERSE)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## 💡 Use Cases

### 1. Find Bug Pattern in Loop
```bash
# Set breakpoint in loop body
> b process_data.py 42

# Jump through iterations until you see the bug
> c    # iteration 1
> c    # iteration 2
> c    # iteration 3 - bug appears!
> rc   # go back
> n    # step forward line by line
```

### 2. Trace Function Call Chain
```bash
# Set breakpoints at key functions
> b main.py 10
> b process.py 25
> b validate.py 50
> b save.py 100

# Follow the execution path
> c    # main.py:10
> c    # process.py:25
> c    # validate.py:50
> c    # save.py:100

# Review backwards
> rc   # back to validate.py:50
> rc   # back to process.py:25
```

### 3. Compare Variable States
```bash
# Set breakpoint where variable changes
> b calculate.py 15

# Jump to first occurrence
> c
Variables: result=0; count=1

# Continue to next
> c
Variables: result=10; count=2

# Go back to compare
> rc
Variables: result=0; count=1
```

## 🔧 Technical Details

### Filename Matching
Breakpoints use smart filename matching:
- **Exact match**: `test.py` matches `test.py`
- **Basename match**: `test.py` matches `/home/user/project/test.py`
- **Substring match**: `helper.py` matches `/path/to/helper.py`

This means you can use:
- Relative paths: `b test.py 25`
- Absolute paths: `b /home/user/project/test.py 25`
- Just filenames: `b helper.py 10`

### Breakpoint Limits
- Maximum **100 breakpoints** per session
- Breakpoints persist throughout your analysis session
- Setting duplicate breakpoints shows a warning

### Navigation Behavior
- **c (continue)**: Searches forward from current position
- **rc (reverse continue)**: Searches backward from current position
- If no breakpoint found, displays a message
- Shows clear indicator whether moving forward or backward

## 🎯 Command Comparison

### Old Way (Limited)
```
> break 42        # Jump to line 42 (first occurrence only)
> n               # Step forward one by one
> back            # Step backward one by one
```

### New Way (Powerful)
```
> b test.py 42    # Set breakpoint at line 42
> b test.py 50    # Set another breakpoint
> c               # Jump to next breakpoint
> c               # Jump to following breakpoint
> rc              # Jump back to previous breakpoint
```

## 📝 Tips & Tricks

### Tip 1: Set Breakpoints First
```bash
# Start of session - set all your breakpoints
> b main.py 10
> b main.py 50
> b helper.py 25
> list
> c    # Start jumping!
```

### Tip 2: Use with View
```bash
> c              # Jump to breakpoint
> view           # See full source context
> c              # Next breakpoint
> view           # See that context
```

### Tip 3: Combine with Jump
```bash
> jump 42        # Jump to line 42
> b test.py 42   # Set breakpoint there
> c              # Jump to next occurrence
> c              # Keep going
```

### Tip 4: Quick Breakpoint List
```bash
> b              # Just 'b' lists breakpoints
```

## 🚫 What Breakpoints DON'T Do

These are **navigation breakpoints** for post-execution analysis:
- ❌ They don't stop execution (trace already captured)
- ❌ They don't modify the program
- ❌ They're just for jumping around the trace

Think of them as **bookmarks** in your execution trace!

## 🎉 Benefits

1. **Fast Navigation**: Jump directly to important locations
2. **Pattern Analysis**: Easily compare loop iterations
3. **Cross-File Flow**: Follow execution across modules
4. **Bidirectional**: Move forward or backward through breakpoints
5. **Smart Matching**: Works with relative or absolute paths

## 📖 Full Command Reference

```
Navigation:
  n              - Next execution step
  back           - Previous execution step
  :<number>      - Jump to execution number

Breakpoints:
  b <file> <line> - Set breakpoint
  b or list      - List breakpoints
  c              - Continue to next breakpoint
  rc             - Reverse continue to previous breakpoint

Analysis:
  view           - View full source file
  summary        - Show trace summary
  find <var>     - Search for variable
  jump <line>    - Jump to source line

Other:
  help           - Show help
  quit or q      - Exit
```

## 🚀 Try It Now!

```bash
# 1. Run your program with the debugger
python3 idebug.py test_import_main.py
> break test_import_main.py 13
> run

# 2. In post-execution viewer, set breakpoints
[1/100] > b test_import_main.py 13
[1/100] > b test_helper.py 9
[1/100] > list

# 3. Navigate!
[1/100] > c     # Jump to first breakpoint
[1/100] > c     # Jump to next
[1/100] > rc    # Go back
```

**Happy debugging! 🎯**
